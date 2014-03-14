/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbluetoothservicediscoveryagent.h"
#include "qbluetoothservicediscoveryagent_p.h"
#include "qlowenergycharacteristicinfo_p.h"
#include "qlowenergyserviceinfo_p.h"

#include "bluez/manager_p.h"
#include "bluez/adapter_p.h"
#include "bluez/device_p.h"
#include "bluez/characteristic_p.h"

#include <QtCore/QLoggingCategory>
#include <QtDBus/QDBusPendingCallWatcher>
#include <QtCore/QUuid>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

QBluetoothServiceDiscoveryAgentPrivate::QBluetoothServiceDiscoveryAgentPrivate(const QBluetoothAddress &deviceAdapter)
:   error(QBluetoothServiceDiscoveryAgent::NoError), m_deviceAdapterAddress(deviceAdapter), state(Inactive), deviceDiscoveryAgent(0),
    mode(QBluetoothServiceDiscoveryAgent::MinimalDiscovery), singleDevice(false),
    manager(0), adapter(0), device(0)
{
    qRegisterMetaType<ServiceMap>("ServiceMap");
    qDBusRegisterMetaType<ServiceMap>();

    manager = new OrgBluezManagerInterface(QLatin1String("org.bluez"), QLatin1String("/"),
                                           QDBusConnection::systemBus());
}

QBluetoothServiceDiscoveryAgentPrivate::~QBluetoothServiceDiscoveryAgentPrivate()
{
    delete device;
    delete manager;
    delete adapter;
}

void QBluetoothServiceDiscoveryAgentPrivate::start(const QBluetoothAddress &address)
{
    Q_Q(QBluetoothServiceDiscoveryAgent);

    qCDebug(QT_BT_BLUEZ) << "Full discovery on: " << address.toString();

    QDBusPendingReply<QDBusObjectPath> reply;
    if (m_deviceAdapterAddress.isNull())
        reply = manager->DefaultAdapter();
    else
        reply = manager->FindAdapter(m_deviceAdapterAddress.toString());

    reply.waitForFinished();
    if (reply.isError()) {
        error = QBluetoothServiceDiscoveryAgent::InputOutputError;
        errorString = QBluetoothServiceDiscoveryAgent::tr("Unable to find appointed local adapter");
        emit q->error(error);
        _q_serviceDiscoveryFinished();
        return;
    }

    adapter = new OrgBluezAdapterInterface(QLatin1String("org.bluez"), reply.value().path(),
                                           QDBusConnection::systemBus());

    if (m_deviceAdapterAddress.isNull()) {
        QDBusPendingReply<QVariantMap> reply = adapter->GetProperties();
        reply.waitForFinished();
        if (!reply.isError()) {
            const QBluetoothAddress path_address(reply.value().value(QStringLiteral("Address")).toString());
            m_deviceAdapterAddress = path_address;
        }
    }

    QDBusPendingReply<QDBusObjectPath> deviceObjectPath = adapter->CreateDevice(address.toString());

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(deviceObjectPath, q);
    watcher->setProperty("_q_BTaddress", QVariant::fromValue(address));
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     q, SLOT(_q_createdDevice(QDBusPendingCallWatcher*)));

}

void QBluetoothServiceDiscoveryAgentPrivate::stop()
{
    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << "Stop called";
    if (device) {
        //we are waiting for _q_discoveredServices() slot to be called
        // adapter is already 0
        QDBusPendingReply<> reply = device->CancelDiscovery();
        reply.waitForFinished();

        device->deleteLater();
        device = 0;
        Q_ASSERT(!adapter);
    } else if (adapter) {
        //we are waiting for _q_createdDevice() slot to be called
        adapter->deleteLater();
        adapter = 0;
        Q_ASSERT(!device);
    }

    discoveredDevices.clear();
    setDiscoveryState(Inactive);
    Q_Q(QBluetoothServiceDiscoveryAgent);
    emit q->canceled();
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_createdDevice(QDBusPendingCallWatcher *watcher)
{
    if (!adapter)
        return;

    Q_Q(QBluetoothServiceDiscoveryAgent);

    const QBluetoothAddress &address = watcher->property("_q_BTaddress").value<QBluetoothAddress>();

    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << "created" << address.toString();

    QDBusPendingReply<QDBusObjectPath> deviceObjectPath = *watcher;
    if (deviceObjectPath.isError()) {
        if (deviceObjectPath.error().name() != QLatin1String("org.bluez.Error.AlreadyExists")) {
            delete adapter;
            adapter = 0;
            _q_serviceDiscoveryFinished();
            qCDebug(QT_BT_BLUEZ) << "Create device failed Error: " << error << deviceObjectPath.error().name();
            return;
        }

        deviceObjectPath = adapter->FindDevice(address.toString());
        deviceObjectPath.waitForFinished();
        if (deviceObjectPath.isError()) {
            delete adapter;
            adapter = 0;
            if (singleDevice) {
                error = QBluetoothServiceDiscoveryAgent::InputOutputError;
                errorString = QBluetoothServiceDiscoveryAgent::tr("Unable to access device");
                emit q->error(error);
            }
            _q_serviceDiscoveryFinished();
            qCDebug(QT_BT_BLUEZ) << "Can't find device after creation Error: " << error << deviceObjectPath.error().name();
            return;
        }
    }

    device = new OrgBluezDeviceInterface(QLatin1String("org.bluez"),
                                         deviceObjectPath.value().path(),
                                         QDBusConnection::systemBus());
    delete adapter;
    adapter = 0;

    QVariantMap deviceProperties;
    QString classType;
    QDBusPendingReply<QVariantMap> deviceReply = device->GetProperties();
    deviceReply.waitForFinished();
    if (!deviceReply.isError()) {
        deviceProperties = deviceReply.value();
        classType = deviceProperties.value(QStringLiteral("Class")).toString();
    }

    /*
     * Low Energy services in bluez are represented as the list of the paths that can be
     * accessed with org.bluez.Characteristic
     */
    //QDBusArgument services = v.value(QLatin1String("Services")).value<QDBusArgument>();


    /*
     * Bluez v4.1 does not have extra bit which gives information if device is Bluetooth
     * Low Energy device and the way to discover it is with Class property of the Bluetooth device.
     * Low Energy devices do not have property Class.
     * In case we have LE device finish service discovery; otherwise search for regular services.
     */
    if (classType.isEmpty()) { //is BLE device or device properties above not retrievable
        qCDebug(QT_BT_BLUEZ) << "Discovered BLE-only device. Normal service discovery skipped.";
        delete device;
        device = 0;

        const QStringList deviceUuids = deviceProperties.value(QStringLiteral("UUIDs")).toStringList();
        for (int i = 0; i < deviceUuids.size(); i++) {
            QString b = deviceUuids.at(i);
            b = b.remove(QLatin1Char('{')).remove(QLatin1Char('}'));
            const QBluetoothUuid uuid(b);

            qCDebug(QT_BT_BLUEZ) << "Discovered BLE service" << uuid << uuidFilter.size();
            QLowEnergyServiceInfo lowEnergyService(uuid);
            lowEnergyService.d_ptr->adapterAddress = m_deviceAdapterAddress;
            lowEnergyService.setDevice(discoveredDevices.at(0));
            if (uuidFilter.isEmpty())
                emit q->serviceDiscovered(lowEnergyService);
            else {
                for (int j = 0; j < uuidFilter.size(); j++) {
                    if (uuidFilter.at(j) == uuid)
                        emit q->serviceDiscovered(lowEnergyService);

                }
            }

        }

        if (singleDevice && deviceReply.isError()) {
            error = QBluetoothServiceDiscoveryAgent::InputOutputError;
            errorString = QBluetoothServiceDiscoveryAgent::tr("Unable to access device");
            emit q->error(error);
        }
        _q_serviceDiscoveryFinished();
    } else {
        QString pattern;
        foreach (const QBluetoothUuid &uuid, uuidFilter)
            pattern += uuid.toString().remove(QLatin1Char('{')).remove(QLatin1Char('}')) + QLatin1Char(' ');

        pattern = pattern.trimmed();
        qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << "Discover restrictions:" << pattern;

        QDBusPendingReply<ServiceMap> discoverReply = device->DiscoverServices(pattern);
        watcher = new QDBusPendingCallWatcher(discoverReply, q);
        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         q, SLOT(_q_discoveredServices(QDBusPendingCallWatcher*)));
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_discoveredServices(QDBusPendingCallWatcher *watcher)
{
    if (!device)
        return;

    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO;

    QDBusPendingReply<ServiceMap> reply = *watcher;
    if (reply.isError()) {
        qCDebug(QT_BT_BLUEZ) << "discoveredServices error: " << error << reply.error().message();
        watcher->deleteLater();
        if (singleDevice) {
            Q_Q(QBluetoothServiceDiscoveryAgent);
            error = QBluetoothServiceDiscoveryAgent::UnknownError;
            errorString = reply.error().message();
            emit q->error(error);
        }
        delete device;
        device = 0;
        _q_serviceDiscoveryFinished();
        return;
    }

    ServiceMap map = reply.value();

    qCDebug(QT_BT_BLUEZ) << "Parsing xml" << discoveredDevices.at(0).address().toString() << discoveredDevices.count() << map.count();

    Q_Q(QBluetoothServiceDiscoveryAgent);

    foreach (const QString &record, reply.value()) {
        QXmlStreamReader xml(record);

        QBluetoothServiceInfo serviceInfo;

        bool btle = false; // Detecting potential BTLE services
        while (!xml.atEnd()) {
            xml.readNext();

            if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == QStringLiteral("attribute")) {
                quint16 attributeId = xml.attributes().value(QStringLiteral("id")).toString().toUShort(0, 0);

                if (xml.readNextStartElement()) {
                    QVariant value = readAttributeValue(xml);
                    if (attributeId == 1) {// Attribute with id 1 contains UUID of the service
                        const QBluetoothServiceInfo::Sequence seq = value.value<QBluetoothServiceInfo::Sequence>();
                        for (int i = 0; i < seq.count(); i++) {
                            const QBluetoothUuid uuid = seq.at(i).value<QBluetoothUuid>();
                            if ((uuid.data1 & 0x1800) == 0x1800) {// We are taking into consideration that LE services starts at 0x1800
                                QLowEnergyServiceInfo leService(uuid);
                                leService.setDevice(discoveredDevices.at(0));
                                btle = true;
                                emit q->serviceDiscovered(leService);
                                break;
                            }
                        }
                    }
                    serviceInfo.setAttribute(attributeId, value);
                }
            }
        }

        if (!btle) {
            serviceInfo.setDevice(discoveredDevices.at(0));
            if (!serviceInfo.isValid())
                continue;

            discoveredServices.append(serviceInfo);
            qCDebug(QT_BT_BLUEZ) << "Discovered services" << discoveredDevices.at(0).address().toString();
            emit q->serviceDiscovered(serviceInfo);
        }

        // could stop discovery, check for state
        if(discoveryState() == Inactive){
            qCDebug(QT_BT_BLUEZ) << "Exit discovery after stop";
            break;
        }
    }

    watcher->deleteLater();
    delete device;
    device = 0;

    _q_serviceDiscoveryFinished();
}

/*
 * Following three methods are implemented in this way to avoid blocking the main thread.
 * We first go through list of services path and then through services characteristics paths.
 *
 * Bluez v4.x does not have support for LE devices through org.bluez interfaces. Because of that
 * these functions will not be used now. I propose to leave them commented because in Bluez v5.x
 * we have support for LE devices and we can use these functions.
 */
/*
void QBluetoothServiceDiscoveryAgentPrivate::_g_discoveredGattService()
{

    if (!gattServices.empty()) {
        qDebug() << gattServices.at(0) << gattServices.size();
        gattService = QLowEnergyServiceInfo(gattServices.at(0));
        gattService.getProperties();
        QObject::connect(gattService.d_ptr.data(), SIGNAL(finished()), this, SLOT(_g_discoveredGattService()));
        characteristic = new OrgBluezCharacteristicInterface(QLatin1String("org.bluez"), gattServices.at(0), QDBusConnection::systemBus());
        QDBusPendingReply<QList<QDBusObjectPath> > characterictisReply = characteristic->DiscoverCharacteristics();
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(characterictisReply, this);
        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(_q_discoverGattCharacteristics(QDBusPendingCallWatcher*)));

        gattServices.removeFirst();
        q_ptr->lowEnergyServiceDiscovered(gattService);
        emit gattService.d_ptr->finished();
    }
    else
        q_ptr->finished();

}

void QBluetoothServiceDiscoveryAgentPrivate::_q_discoverGattCharacteristics(QDBusPendingCallWatcher *watcher)
{

    QDBusPendingReply<QList<QDBusObjectPath> > characterictisReply = *watcher;
    if (characterictisReply.isError()){
        qDebug()<< "Discovering service characteristic error" << characterictisReply.error().message();
        Q_Q(QBluetoothServiceDiscoveryAgent);
        error = QBluetoothServiceDiscoveryAgent::UnknownError;
        errorString = characterictisReply.error().message();
        emit q->error(error);
        _q_serviceDiscoveryFinished();
        return;
    }

    foreach (const QDBusObjectPath &characteristicPath, characterictisReply.value())
        gattCharacteristics.append(characteristicPath.path());
    characteristic = new OrgBluezCharacteristicInterface(QLatin1String("org.bluez"), gattCharacteristics.at(0), QDBusConnection::systemBus());
    QDBusPendingReply<QVariantMap> characteristicProperty = characteristic->GetProperties();
    watcher = new QDBusPendingCallWatcher(characteristicProperty, this);
    _q_discoveredGattCharacteristic(watcher);

}

void QBluetoothServiceDiscoveryAgentPrivate::_q_discoveredGattCharacteristic(QDBusPendingCallWatcher *watcher)
{

    QDBusPendingReply<QVariantMap> characteristicProperty = *watcher;
    //qDebug()<<characteristicProperty.value();
    if (characteristicProperty.isError()){
        qDebug() << "Characteristic properties error" << characteristicProperty.error().message();
        Q_Q(QBluetoothServiceDiscoveryAgent);
        error = QBluetoothServiceDiscoveryAgent::UnknownError;
        errorString = characteristicProperty.error().message();
        emit q->error(error);
        _q_serviceDiscoveryFinished();
        return;
    }
    QStringList serviceName;

    if (characteristicProperty.isError())
        qDebug()<<characteristicProperty.error().message();

    QVariantMap properties = characteristicProperty.value();
    QString name = properties.value(QLatin1String("Name")).toString();
    QString description = properties.value(QLatin1String("Description")).toString();
    serviceName = description.split(QStringLiteral(" "));
    QString charUuid = properties.value(QLatin1String("UUID")).toString();
    QBluetoothUuid characteristicUuid(charUuid);

    QVariant value = properties.value(QLatin1String("Value"));
    QByteArray byteValue = QByteArray();
    if (value.type() == QVariant::ByteArray)
        byteValue = value.toByteArray();

    //qDebug() << name << description << characteristicUuid.toString()<< byteValue.size();
    gattCharacteristic = QLowEnergyCharacteristicInfo(name, description, characteristicUuid, byteValue);
    gattCharacteristic.setPath(gattCharacteristics.at(0));
    qDebug() << gattCharacteristics.at(0);
    gattService.addCharacteristic(gattCharacteristic);


    //Testing part for setting the property
    QString b = "f000aa02-0451-4000-b000-000000000000";
    QBluetoothUuid u(b);
    if (gattCharacteristic.uuid() == u){
        for (int j = 0; j< byteValue.size(); j++){
            qDebug() << (int) byteValue.at(j);
            byteValue[j]=1;
            qDebug() << (int) byteValue.at(j);
        }
        bool s = gattCharacteristic.setPropertyValue(QStringLiteral("Value"), byteValue);
        qDebug() <<s;
    }

    QString serName = serviceName.at(0) + " Service";

    gattCharacteristics.removeFirst();
    if (gattCharacteristics.isEmpty()){
        q_ptr->lowEnergyServiceDiscovered(gattService);
        emit gattService.d_ptr->finished();
    }
    else{
        OrgBluezCharacteristicInterface *characteristicProperties = new OrgBluezCharacteristicInterface(QLatin1String("org.bluez"), gattCharacteristics.at(0), QDBusConnection::systemBus());
        QDBusPendingReply<QVariantMap> characteristicProperty = characteristicProperties->GetProperties();
        watcher = new QDBusPendingCallWatcher(characteristicProperty, this);
                QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                                 this, SLOT(_q_discoveredGattCharacteristic(QDBusPendingCallWatcher*)));
    }

}
*/

QVariant QBluetoothServiceDiscoveryAgentPrivate::readAttributeValue(QXmlStreamReader &xml)
{
    if (xml.name() == QLatin1String("boolean")) {
        const QString value = xml.attributes().value(QLatin1String("value")).toString();
        xml.skipCurrentElement();
        return value == QLatin1String("true");
    } else if (xml.name() == QLatin1String("uint8")) {
        quint8 value = xml.attributes().value(QLatin1String("value")).toString().toUShort(0, 0);
        xml.skipCurrentElement();
        return value;
    } else if (xml.name() == QLatin1String("uint16")) {
        quint16 value = xml.attributes().value(QLatin1String("value")).toString().toUShort(0, 0);
        xml.skipCurrentElement();
        return value;
    } else if (xml.name() == QLatin1String("uint32")) {
        quint32 value = xml.attributes().value(QLatin1String("value")).toString().toUInt(0, 0);
        xml.skipCurrentElement();
        return value;
    } else if (xml.name() == QLatin1String("uint64")) {
        quint64 value = xml.attributes().value(QLatin1String("value")).toString().toULongLong(0, 0);
        xml.skipCurrentElement();
        return value;
    } else if (xml.name() == QLatin1String("uuid")) {
        QBluetoothUuid uuid;
        const QString value = xml.attributes().value(QLatin1String("value")).toString();
        if (value.startsWith(QLatin1String("0x"))) {
            if (value.length() == 6) {
                quint16 v = value.toUShort(0, 0);
                uuid = QBluetoothUuid(v);
            } else if (value.length() == 10) {
                quint32 v = value.toUInt(0, 0);
                uuid = QBluetoothUuid(v);
            }
        } else {
            uuid = QBluetoothUuid(value);
        }
        xml.skipCurrentElement();
        return QVariant::fromValue(uuid);
    } else if (xml.name() == QLatin1String("text")) {
        QString value = xml.attributes().value(QLatin1String("value")).toString();
        if (xml.attributes().value(QLatin1String("encoding")) == QLatin1String("hex"))
            value = QString::fromUtf8(QByteArray::fromHex(value.toLatin1()));
        xml.skipCurrentElement();
        return value;
    } else if (xml.name() == QLatin1String("sequence")) {
        QBluetoothServiceInfo::Sequence sequence;

        while (xml.readNextStartElement()) {
            QVariant value = readAttributeValue(xml);
            sequence.append(value);
        }

        return QVariant::fromValue<QBluetoothServiceInfo::Sequence>(sequence);
    } else {
        qCWarning(QT_BT_BLUEZ) << "unknown attribute type"
                               << xml.name().toString()
                               << xml.attributes().value(QLatin1String("value")).toString();
        xml.skipCurrentElement();
        return QVariant();
    }
}

QT_END_NAMESPACE

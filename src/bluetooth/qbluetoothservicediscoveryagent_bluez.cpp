/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#include "bluez/manager_p.h"
#include "bluez/adapter_p.h"
#include "bluez/device_p.h"

#include <QtCore/QLoggingCategory>
#include <QtDBus/QDBusPendingCallWatcher>

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

    foreach (const QString &record, reply.value()) {
        QXmlStreamReader xml(record);

        QBluetoothServiceInfo serviceInfo;
        serviceInfo.setDevice(discoveredDevices.at(0));

        while (!xml.atEnd()) {
            xml.readNext();

            if (xml.tokenType() == QXmlStreamReader::StartElement &&
                xml.name() == QLatin1String("attribute")) {
                quint16 attributeId =
                    xml.attributes().value(QLatin1String("id")).toString().toUShort(0, 0);

                if (xml.readNextStartElement()) {
                    QVariant value = readAttributeValue(xml);

                    serviceInfo.setAttribute(attributeId, value);
                }
            }
        }

        if (!serviceInfo.isValid())
            continue;

        Q_Q(QBluetoothServiceDiscoveryAgent);

        //don't include the service if we already discovered it before
        bool alreadyDiscovered = false;
        for (int j = 0; j < discoveredServices.count(); j++) {
            const QBluetoothServiceInfo &info = discoveredServices.at(j);
            if (info.device() == serviceInfo.device()
                    && info.serviceClassUuids() == serviceInfo.serviceClassUuids()
                    && info.serviceUuid() == serviceInfo.serviceUuid()) {
                alreadyDiscovered = true;
                break;
            }
        }

        if (!alreadyDiscovered) {
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
    } else if (xml.name() == QLatin1String("text") || xml.name() == QStringLiteral("url")) {
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

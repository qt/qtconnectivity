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

#include <QtCore/QLoggingCategory>
#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothdevicediscoveryagent_p.h"
#include "qbluetoothaddress.h"
#include "qbluetoothuuid.h"

#include "bluez/manager_p.h"
#include "bluez/adapter_p.h"
#include "bluez/device_p.h"
#include "bluez/bluez5_helper_p.h"
#include "bluez/objectmanager_p.h"
#include "bluez/adapter1_bluez5_p.h"
#include "bluez/device1_bluez5_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(
    const QBluetoothAddress &deviceAdapter, QBluetoothDeviceDiscoveryAgent *parent) :
    lastError(QBluetoothDeviceDiscoveryAgent::NoError),
    m_adapterAddress(deviceAdapter),
    pendingCancel(false),
    pendingStart(false),
    manager(0),
    adapter(0),
    managerBluez5(0),
    adapterBluez5(0),
    discoveryTimer(0),
    q_ptr(parent)
{
    if (isBluez5()) {
        Q_Q(QBluetoothDeviceDiscoveryAgent);
        managerBluez5 = new OrgFreedesktopDBusObjectManagerInterface(
                                           QStringLiteral("org.bluez"),
                                           QStringLiteral("/"),
                                           QDBusConnection::systemBus(), parent);
        QObject::connect(managerBluez5,
                         SIGNAL(InterfacesAdded(QDBusObjectPath,InterfaceList)),
                         q, SLOT(_q_InterfacesAdded(QDBusObjectPath,InterfaceList)));

    } else {
        manager = new OrgBluezManagerInterface(QLatin1String("org.bluez"), QLatin1String("/"),
                                           QDBusConnection::systemBus(), parent);
    }
    inquiryType = QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry;
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
    delete adapter;
    delete adapterBluez5;
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    if (pendingStart)
        return true;
    if (pendingCancel)
        return false;

    return (adapter || adapterBluez5);
}

void QBluetoothDeviceDiscoveryAgentPrivate::start()
{
    if (pendingCancel == true) {
        pendingStart = true;
        return;
    }

    discoveredDevices.clear();

    if (managerBluez5) {
        startBluez5();
        return;
    }

    QDBusPendingReply<QDBusObjectPath> reply;

    if (m_adapterAddress.isNull())
        reply = manager->DefaultAdapter();
    else
        reply = manager->FindAdapter(m_adapterAddress.toString());
    reply.waitForFinished();

    if (reply.isError()) {
        errorString = reply.error().message();
        qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << "ERROR: " << errorString;
        lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
        Q_Q(QBluetoothDeviceDiscoveryAgent);
        emit q->error(lastError);
        return;
    }

    adapter = new OrgBluezAdapterInterface(QLatin1String("org.bluez"), reply.value().path(),
                                           QDBusConnection::systemBus());

    Q_Q(QBluetoothDeviceDiscoveryAgent);
    QObject::connect(adapter, SIGNAL(DeviceFound(QString, QVariantMap)),
                     q, SLOT(_q_deviceFound(QString, QVariantMap)));
    QObject::connect(adapter, SIGNAL(PropertyChanged(QString, QDBusVariant)),
                     q, SLOT(_q_propertyChanged(QString, QDBusVariant)));

    QDBusPendingReply<QVariantMap> propertiesReply = adapter->GetProperties();
    propertiesReply.waitForFinished();
    if (propertiesReply.isError()) {
        errorString = propertiesReply.error().message();
        delete adapter;
        adapter = 0;
        qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << "ERROR: " << errorString;
        lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
        Q_Q(QBluetoothDeviceDiscoveryAgent);
        delete adapter;
        adapter = 0;
        emit q->error(lastError);
        return;
    }

    if (!propertiesReply.value().value(QStringLiteral("Powered")).toBool()) {
        qCDebug(QT_BT_BLUEZ) << "Aborting device discovery due to offline Bluetooth Adapter";
        lastError = QBluetoothDeviceDiscoveryAgent::PoweredOffError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Device is powered off");
        delete adapter;
        adapter = 0;
        emit q->error(lastError);
        return;
    }

    QDBusPendingReply<> discoveryReply = adapter->StartDiscovery();
    if (discoveryReply.isError()) {
        delete adapter;
        adapter = 0;
        errorString = discoveryReply.error().message();
        lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
        Q_Q(QBluetoothDeviceDiscoveryAgent);
        emit q->error(lastError);
        qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << "ERROR: " << errorString;
        return;
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::startBluez5()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    QDBusPendingReply<ManagedObjectList> reply = managerBluez5->GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError()) {
        errorString = reply.error().message();
        lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
        emit q->error(lastError);
        return;
    }


    OrgBluezAdapter1Interface *tempAdapter = 0;
    QMap<QString, QVariantMap> devicesForAdapter; // dbus path for devices for matching adapter

    foreach (const QDBusObjectPath &path, reply.value().keys()) {
        const InterfaceList ifaceList = reply.value().value(path);
        foreach (const QString &iface, ifaceList.keys()) {
            if (iface == QStringLiteral("org.bluez.Adapter1")) {
                if (tempAdapter)
                    continue;

                if (m_adapterAddress.isNull()) {
                    // use the first found adapter as default
                    tempAdapter = new OrgBluezAdapter1Interface(QStringLiteral("org.bluez"),
                                                                path.path(),
                                                                QDBusConnection::systemBus());
                } else {
                    const QString addressString = ifaceList.value(iface).
                            value(QStringLiteral("Address")).toString();
                    if (m_adapterAddress == QBluetoothAddress(addressString)) {
                        tempAdapter = new OrgBluezAdapter1Interface(
                                                QStringLiteral("org.bluez"),
                                                path.path(),
                                                QDBusConnection::systemBus());
                    }
                }
            } else if (iface == QStringLiteral("org.bluez.Device1")) {
                devicesForAdapter.insert(path.path(), ifaceList.value(iface));
            }
        }
    }

    if (!tempAdapter) {
        qCDebug(QT_BT_BLUEZ) << "Cannot find Bluez 5 adapter for device search";
        lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Cannot find valid Bluetooth adapter.");
        q->error(lastError);
        return;
    }

    if (!tempAdapter->powered()) {
        qCDebug(QT_BT_BLUEZ) << "Aborting device discovery due to offline Bluetooth Adapter";
        lastError = QBluetoothDeviceDiscoveryAgent::PoweredOffError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Device is powered off");
        delete tempAdapter;
        emit q->error(lastError);
        return;
    }

    adapterBluez5 = tempAdapter;
    QtBluezDiscoveryManager::instance()->registerDiscoveryInterest(adapterBluez5->path());
    QObject::connect(QtBluezDiscoveryManager::instance(), SIGNAL(discoveryInterrupted(QString)),
            q, SLOT(_q_discoveryInterrupted(QString)));

    // collect initial set of information
    foreach (const QString &path, devicesForAdapter.keys()) {
        if (path.indexOf(adapterBluez5->path()) != 0)
            continue; //devices path doesnt start with same path as adapter

        deviceFoundBluez5(path);
    }

    // wait 20s and sum up what was found
    if (!discoveryTimer) {
        discoveryTimer = new QTimer(q);
        discoveryTimer->setSingleShot(true);
        discoveryTimer->setInterval(20000); // 20s
        QObject::connect(discoveryTimer, SIGNAL(timeout()),
                         q, SLOT(_q_discoveryFinished()));
    }

    discoveryTimer->start();
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    if (!adapter && !adapterBluez5)
        return;

    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO;
    pendingCancel = true;
    pendingStart = false;
    if (adapter) {
        QDBusPendingReply<> reply = adapter->StopDiscovery();
        reply.waitForFinished();
    } else {
        _q_discoveryFinished();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::_q_deviceFound(const QString &address,
                                                           const QVariantMap &dict)
{
    const QBluetoothAddress btAddress(address);
    const QString btName = dict.value(QLatin1String("Name")).toString();
    quint32 btClass = dict.value(QLatin1String("Class")).toUInt();

    qCDebug(QT_BT_BLUEZ) << "Discovered: " << address << btName
                         << "Num UUIDs" << dict.value(QLatin1String("UUIDs")).toStringList().count()
                         << "total device" << discoveredDevices.count() << "cached"
                         << dict.value(QLatin1String("Cached")).toBool()
                         << "RSSI" << dict.value(QLatin1String("RSSI")).toInt();

    QBluetoothDeviceInfo device(btAddress, btName, btClass);
    if (dict.value(QLatin1String("RSSI")).isValid())
        device.setRssi(dict.value(QLatin1String("RSSI")).toInt());
    QList<QBluetoothUuid> uuids;
    foreach (const QString &u, dict.value(QLatin1String("UUIDs")).toStringList())
        uuids.append(QBluetoothUuid(u));
    device.setServiceUuids(uuids, QBluetoothDeviceInfo::DataIncomplete);
    device.setCached(dict.value(QLatin1String("Cached")).toBool());
    for (int i = 0; i < discoveredDevices.size(); i++) {
        if (discoveredDevices[i].address() == device.address()) {
            if (discoveredDevices[i] == device) {
                qCDebug(QT_BT_BLUEZ) << "Duplicate: " << address;
                return;
            }
            discoveredDevices.replace(i, device);
            Q_Q(QBluetoothDeviceDiscoveryAgent);
            qCDebug(QT_BT_BLUEZ) << "Updated: " << address;

            emit q->deviceDiscovered(device);
            return; // this works if the list doesn't contain duplicates. Don't let it.
        }
    }
    qCDebug(QT_BT_BLUEZ) << "Emit: " << address;
    discoveredDevices.append(device);
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    emit q->deviceDiscovered(device);
}

void QBluetoothDeviceDiscoveryAgentPrivate::deviceFoundBluez5(const QString& devicePath)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    if (!q->isActive())
        return;

    OrgBluezDevice1Interface device(QStringLiteral("org.bluez"), devicePath,
                                    QDBusConnection::systemBus());

    if (device.adapter().path() != adapterBluez5->path())
        return;

    const QBluetoothAddress btAddress(device.address());
    if (btAddress.isNull()) // no point reporting an empty address
        return;

    const QString btName = device.name();
    quint32 btClass = device.classProperty();

    qCDebug(QT_BT_BLUEZ) << "Discovered: " << btAddress.toString() << btName
                         << "Num UUIDs" << device.uUIDs().count()
                         << "total device" << discoveredDevices.count() << "cached"
                         << "RSSI" << device.rSSI();

    // read information
    QBluetoothDeviceInfo deviceInfo(btAddress, btName, btClass);
    deviceInfo.setRssi(device.rSSI());
    QList<QBluetoothUuid> uuids;
    foreach (const QString &u, device.uUIDs())
        uuids.append(QBluetoothUuid(u));
    deviceInfo.setServiceUuids(uuids, QBluetoothDeviceInfo::DataIncomplete);

    for (int i = 0; i < discoveredDevices.size(); i++) {
        if (discoveredDevices[i].address() == deviceInfo.address()) {
            if (discoveredDevices[i] == deviceInfo) {
                qCDebug(QT_BT_BLUEZ) << "Duplicate: " << btAddress.toString();
                return;
            }
            discoveredDevices.replace(i, deviceInfo);

            emit q->deviceDiscovered(deviceInfo);
            return; // this works if the list doesn't contain duplicates. Don't let it.
        }
    }

    discoveredDevices.append(deviceInfo);
    emit q->deviceDiscovered(deviceInfo);
}

void QBluetoothDeviceDiscoveryAgentPrivate::_q_propertyChanged(const QString &name,
                                                               const QDBusVariant &value)
{
    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << name << value.variant();

    if (name == QLatin1String("Discovering") && !value.variant().toBool()) {
        Q_Q(QBluetoothDeviceDiscoveryAgent);
        adapter->deleteLater();
        adapter = 0;
        if (pendingCancel && !pendingStart) {
            emit q->canceled();
            pendingCancel = false;
        } else if (pendingStart) {
            pendingStart = false;
            pendingCancel = false;
            start();
        } else {
            emit q->finished();
        }
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::_q_InterfacesAdded(const QDBusObjectPath &object_path,
                                                               InterfaceList interfaces_and_properties)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    if (!q->isActive())
        return;

    if (interfaces_and_properties.contains(QStringLiteral("org.bluez.Device1"))) {
        // device interfaces belonging to different adapter
        // will be filtered out by deviceFoundBluez5();
        deviceFoundBluez5(object_path.path());
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::_q_discoveryFinished()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    if (discoveryTimer)
        discoveryTimer->stop();

    QtBluezDiscoveryManager::instance()->disconnect(q);
    QtBluezDiscoveryManager::instance()->unregisterDiscoveryInterest(adapterBluez5->path());

    delete adapterBluez5;
    adapterBluez5 = 0;

    if (pendingCancel && !pendingStart) {
        emit q->canceled();
        pendingCancel = false;
    } else if (pendingStart) {
        pendingStart = false;
        pendingCancel = false;
        start();
    } else {
        emit q->finished();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::_q_discoveryInterrupted(const QString &path)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    if (!q->isActive())
        return;

    if (path == adapterBluez5->path()) {
        qCWarning(QT_BT_BLUEZ) << "Device discovery aborted due to unexpected adapter changes";

        if (discoveryTimer)
            discoveryTimer->stop();

        QtBluezDiscoveryManager::instance()->disconnect(q);
        // no need to call unregisterDiscoveryInterest since QtBluezDiscoveryManager
        // does this automatically when emitting discoveryInterrupted(QString) signal

        delete adapterBluez5;
        adapterBluez5 = 0;

        errorString = QBluetoothDeviceDiscoveryAgent::tr("Bluetooth adapter error");
        lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
        emit q->error(lastError);
    }
}

QT_END_NAMESPACE

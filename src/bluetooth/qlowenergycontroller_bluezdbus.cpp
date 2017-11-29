/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qlowenergycontroller_bluezdbus_p.h"
#include "bluez/adapter1_bluez5_p.h"
#include "bluez/bluez5_helper_p.h"
#include "bluez/device1_bluez5_p.h"
#include "bluez/objectmanager_p.h"
#include "bluez/properties_p.h"


QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

QLowEnergyControllerPrivateBluezDBus::QLowEnergyControllerPrivateBluezDBus()
    : QLowEnergyControllerPrivate()
{
}

QLowEnergyControllerPrivateBluezDBus::~QLowEnergyControllerPrivateBluezDBus()
{
}

void QLowEnergyControllerPrivateBluezDBus::init()
{
}

void QLowEnergyControllerPrivateBluezDBus::devicePropertiesChanged(
        const QString &interface, const QVariantMap &changedProperties,
        const QStringList &/*removedProperties*/)
{
    if (interface == QStringLiteral("org.bluez.Device1")) {
        qCDebug(QT_BT_BLUEZ) << "######" << interface << changedProperties;
        if (changedProperties.contains(QStringLiteral("ServicesResolved"))) {
            //we could check for Connected property as well, but we choose to wait
            //for ServicesResolved being true

            if (pendingConnect) {
                bool isResolved = changedProperties.value(QStringLiteral("ServicesResolved")).toBool();
                if (isResolved) {
                    setState(QLowEnergyController::ConnectedState);
                    pendingConnect = false;
                    disconnectSignalRequired = true;
                    Q_Q(QLowEnergyController);
                    emit q->connected();
                }
            }
        }

        if (changedProperties.contains(QStringLiteral("Connected"))) {
            bool isConnected = changedProperties.value(QStringLiteral("Connected")).toBool();
            if (!isConnected) {
                switch (state) {
                case QLowEnergyController::ConnectingState:
                case QLowEnergyController::ConnectedState:
                case QLowEnergyController::DiscoveringState:
                case QLowEnergyController::DiscoveredState:
                case QLowEnergyController::ClosingState:
                {
                    bool emitDisconnect = disconnectSignalRequired;
                    bool emitError = pendingConnect;

                    resetController();

                    if (emitError)
                        setError(QLowEnergyController::ConnectionError);
                    setState(QLowEnergyController::UnconnectedState);

                    if (emitDisconnect) {
                        Q_Q(QLowEnergyController);
                        emit q->disconnected();
                    }
                }
                    break;
                case QLowEnergyController::AdvertisingState:
                case QLowEnergyController::UnconnectedState:
                    //ignore
                    break;
                }
            }
        }
    }
}

void QLowEnergyControllerPrivateBluezDBus::interfacesRemoved(
        const QDBusObjectPath &objectPath, const QStringList &/*interfaces*/)
{
    if (objectPath.path() == device->path()) {
        resetController();
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        qCWarning(QT_BT_BLUEZ) << "DBus Device1 was removed";
        setState(QLowEnergyController::UnconnectedState);
    } else if (objectPath.path() == adapter->path()) {
        resetController();
        setError(QLowEnergyController::InvalidBluetoothAdapterError);
        qCWarning(QT_BT_BLUEZ) << "DBus Adapter was removed";
        setState(QLowEnergyController::UnconnectedState);
    }
}

void QLowEnergyControllerPrivateBluezDBus::resetController()
{
    if (managerBluez) {
        delete managerBluez;
        managerBluez = nullptr;
    }

    if (adapter) {
        delete adapter;
        adapter = nullptr;
    }

    if (device) {
        delete device;
        device = nullptr;
    }

    if (deviceMonitor) {
        delete deviceMonitor;
        deviceMonitor = nullptr;
    }

    pendingConnect = pendingDisconnect = disconnectSignalRequired = false;
}

void QLowEnergyControllerPrivateBluezDBus::connectToDeviceHelper()
{
    resetController();

    bool ok = false;
    const QString hostAdapterPath = findAdapterForAddress(localAdapter, &ok);
    if (!ok || hostAdapterPath.isEmpty()) {
        qCWarning(QT_BT_BLUEZ) << "Cannot find suitable bluetooth adapter";
        setError(QLowEnergyController::InvalidBluetoothAdapterError);
        return;
    }

    QScopedPointer<OrgFreedesktopDBusObjectManagerInterface> manager(
                            new OrgFreedesktopDBusObjectManagerInterface(
                                QStringLiteral("org.bluez"), QStringLiteral("/"),
                                QDBusConnection::systemBus()));

    QDBusPendingReply<ManagedObjectList> reply = manager->GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError()) {
        qCWarning(QT_BT_BLUEZ) << "Cannot enumerate Bluetooth devices for GATT connect";
        setError(QLowEnergyController::ConnectionError);
        return;
    }

    QString devicePath;
    ManagedObjectList managedObjectList = reply.value();
    for (ManagedObjectList::const_iterator it = managedObjectList.constBegin(); it != managedObjectList.constEnd(); ++it) {
        const InterfaceList &ifaceList = it.value();

        for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd(); ++jt) {
            const QString &iface = jt.key();
            const QVariantMap &ifaceValues = jt.value();

            if (iface == QStringLiteral("org.bluez.Device1")) {
                if (remoteDevice.toString() == ifaceValues.value(QStringLiteral("Address")).toString()) {
                    devicePath = it.key().path();
                    break;
                }
            }
        }

        if (!devicePath.isEmpty())
            break;
    }

    if (devicePath.isEmpty()) {
        qCDebug(QT_BT_BLUEZ) << "Cannot find targeted remote device. "
                                "Re-running device discovery might help";
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        return;
    }

    managerBluez = manager.take();
    connect(managerBluez, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
            this, &QLowEnergyControllerPrivateBluezDBus::interfacesRemoved);
    adapter = new OrgBluezAdapter1Interface(
                                QStringLiteral("org.bluez"), hostAdapterPath,
                                QDBusConnection::systemBus(), this);
    device = new OrgBluezDevice1Interface(
                                QStringLiteral("org.bluez"), devicePath,
                                QDBusConnection::systemBus(), this);
    deviceMonitor = new OrgFreedesktopDBusPropertiesInterface(
                                QStringLiteral("org.bluez"), devicePath,
                                QDBusConnection::systemBus(), this);
    connect(deviceMonitor, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged,
            this, &QLowEnergyControllerPrivateBluezDBus::devicePropertiesChanged);
}

void QLowEnergyControllerPrivateBluezDBus::connectToDevice()
{
    qCDebug(QT_BT_BLUEZ) << "QLowEnergyControllerPrivateBluezDBus::connectToDevice()";

    connectToDeviceHelper();

    if (!adapter || !device)
        return;

    if (!adapter->powered()) {
        qCWarning(QT_BT_BLUEZ) << "Error: Local adapter is powered off";
        setError(QLowEnergyController::ConnectionError);
        return;
    }

    setState(QLowEnergyController::ConnectingState);

    //Bluez interface is shared among all platform processes
    //and hence we might be connected already
    if (device->connected() && device->servicesResolved()) {
        //connectToDevice is noop
        disconnectSignalRequired = true;
        setState(QLowEnergyController::ConnectedState);
        return;
    }

    pendingConnect = true;

    QDBusPendingReply<> reply = device->Connect();
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [=](QDBusPendingCallWatcher* call) {
        QDBusPendingReply<> reply = *call;
        if (reply.isError()) {
            qCDebug(QT_BT_BLUEZ) << "BTLE_DBUS::connect() failed"
                                 << reply.reply().errorName()
                                 << reply.reply().errorMessage();
            bool emitDisconnect = disconnectSignalRequired;
            resetController();
            setError(QLowEnergyController::UnknownError);
            setState(QLowEnergyController::UnconnectedState);
            if (emitDisconnect) {
                Q_Q(QLowEnergyController);
                emit q->disconnected();
            }
        } // else -> connected when Connected property is set to true (see devicePropertiesChanged())
        call->deleteLater();
    });
}

void QLowEnergyControllerPrivateBluezDBus::disconnectFromDevice()
{
    setState(QLowEnergyController::ClosingState);

    pendingDisconnect = true;

    QDBusPendingReply<> reply = device->Disconnect();
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [=](QDBusPendingCallWatcher* call) {
        QDBusPendingReply<> reply = *call;
        if (reply.isError()) {
            qCDebug(QT_BT_BLUEZ) << "BTLE_DBUS::disconnect() failed"
                                 << reply.reply().errorName()
                                 << reply.reply().errorMessage();
            bool emitDisconnect = disconnectSignalRequired;
            resetController();
            setState(QLowEnergyController::UnconnectedState);
            if (emitDisconnect) {
                Q_Q(QLowEnergyController);
                emit q->disconnected();
            }
        }
        call->deleteLater();
    });
}

void QLowEnergyControllerPrivateBluezDBus::discoverServices()
{

}

void QLowEnergyControllerPrivateBluezDBus::discoverServiceDetails(const QBluetoothUuid &/*service*/)
{

}

void QLowEnergyControllerPrivateBluezDBus::readCharacteristic(
                    const QSharedPointer<QLowEnergyServicePrivate> /*service*/,
                    const QLowEnergyHandle /*charHandle*/)
{

}

void QLowEnergyControllerPrivateBluezDBus::readDescriptor(
                    const QSharedPointer<QLowEnergyServicePrivate> /*service*/,
                    const QLowEnergyHandle /*charHandle*/,
                    const QLowEnergyHandle /*descriptorHandle*/)
{

}

void QLowEnergyControllerPrivateBluezDBus::writeCharacteristic(
                    const QSharedPointer<QLowEnergyServicePrivate> /*service*/,
                    const QLowEnergyHandle /*charHandle*/,
                    const QByteArray &/*newValue*/,
                    QLowEnergyService::WriteMode /*writeMode*/)
            {

}

void QLowEnergyControllerPrivateBluezDBus::writeDescriptor(
                    const QSharedPointer<QLowEnergyServicePrivate> /*service*/,
                    const QLowEnergyHandle /*charHandle*/,
                    const QLowEnergyHandle /*descriptorHandle*/,
                    const QByteArray &/*newValue*/)
{

}

void QLowEnergyControllerPrivateBluezDBus::startAdvertising(
                    const QLowEnergyAdvertisingParameters &/* params */,
                    const QLowEnergyAdvertisingData &/* advertisingData */,
                    const QLowEnergyAdvertisingData &/* scanResponseData */)
{
}

void QLowEnergyControllerPrivateBluezDBus::stopAdvertising()
{
}

void QLowEnergyControllerPrivateBluezDBus::requestConnectionUpdate(
                    const QLowEnergyConnectionParameters & /* params */)
{
}

void QLowEnergyControllerPrivateBluezDBus::addToGenericAttributeList(
                    const QLowEnergyServiceData &/* service */,
                    QLowEnergyHandle /* startHandle */)
{
}

QLowEnergyService *QLowEnergyControllerPrivateBluezDBus::addServiceHelper(
                    const QLowEnergyServiceData &/*service*/)
{
    return nullptr;
}

QT_END_NAMESPACE

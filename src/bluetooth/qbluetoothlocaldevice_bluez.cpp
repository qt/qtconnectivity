/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include <QtCore/QLoggingCategory>
#include <QtCore/QRandomGenerator>
#include <QtDBus/QDBusContext>

#include "qbluetoothlocaldevice.h"
#include "qbluetoothaddress.h"
#include "qbluetoothlocaldevice_p.h"

#include "bluez/bluez5_helper_p.h"
#include "bluez/objectmanager_p.h"
#include "bluez/properties_p.h"
#include "bluez/adapter1_bluez5_p.h"
#include "bluez/device1_bluez5_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

inline uint qHash(const QBluetoothAddress &address)
{
    return ::qHash(address.toUInt64());
}

QBluetoothLocalDevice::QBluetoothLocalDevice(QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothLocalDevicePrivate(this))
{
}

QBluetoothLocalDevice::QBluetoothLocalDevice(const QBluetoothAddress &address, QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothLocalDevicePrivate(this, address))
{
}

QString QBluetoothLocalDevice::name() const
{
    if (d_ptr->adapterBluez5)
        return d_ptr->adapterBluez5->alias();

    return QString();
}

QBluetoothAddress QBluetoothLocalDevice::address() const
{
    if (d_ptr->adapterBluez5)
        return QBluetoothAddress(d_ptr->adapterBluez5->address());

    return QBluetoothAddress();
}

void QBluetoothLocalDevice::powerOn()
{
    if (d_ptr->adapterBluez5)
        d_ptr->adapterBluez5->setPowered(true);
}

void QBluetoothLocalDevice::setHostMode(QBluetoothLocalDevice::HostMode mode)
{
    if (!isValid())
        return;

    Q_D(QBluetoothLocalDevice);

    switch (mode) {
    case HostDiscoverableLimitedInquiry:
    case HostDiscoverable:
        if (hostMode() == HostPoweredOff) {
            // We first have to wait for BT to be powered on,
            // then we can set the host mode correctly
            d->pendingHostModeChange = static_cast<int>(HostDiscoverable);
            d->adapterBluez5->setPowered(true);
        } else {
            d->adapterBluez5->setDiscoverable(true);
        }
        break;
    case HostConnectable:
        if (hostMode() == HostPoweredOff) {
            d->pendingHostModeChange = static_cast<int>(HostConnectable);
            d->adapterBluez5->setPowered(true);
        } else {
            d->adapterBluez5->setDiscoverable(false);
        }
        break;
    case HostPoweredOff:
        d->adapterBluez5->setPowered(false);
        break;
    }
}

QBluetoothLocalDevice::HostMode QBluetoothLocalDevice::hostMode() const
{
    if (d_ptr->adapterBluez5) {
        if (!d_ptr->adapterBluez5->powered())
            return HostPoweredOff;
        else if (d_ptr->adapterBluez5->discoverable())
            return HostDiscoverable;
        else if (d_ptr->adapterBluez5->powered())
            return HostConnectable;
    }

    return HostPoweredOff;
}

QList<QBluetoothAddress> QBluetoothLocalDevice::connectedDevices() const
{
    return d_ptr->connectedDevices();
}

QList<QBluetoothHostInfo> QBluetoothLocalDevice::allDevices()
{
    QList<QBluetoothHostInfo> localDevices;

    isBluez5(); // temporary fix for dbus registration
    OrgFreedesktopDBusObjectManagerInterface manager(
            QStringLiteral("org.bluez"), QStringLiteral("/"), QDBusConnection::systemBus());
    QDBusPendingReply<ManagedObjectList> reply = manager.GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError())
        return localDevices;

    ManagedObjectList managedObjectList = reply.value();
    for (ManagedObjectList::const_iterator it = managedObjectList.constBegin();
         it != managedObjectList.constEnd(); ++it) {
        const InterfaceList &ifaceList = it.value();

        for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd();
             ++jt) {
            const QString &iface = jt.key();
            const QVariantMap &ifaceValues = jt.value();

            if (iface == QStringLiteral("org.bluez.Adapter1")) {
                QBluetoothHostInfo hostInfo;
                const QString temp = ifaceValues.value(QStringLiteral("Address")).toString();

                hostInfo.setAddress(QBluetoothAddress(temp));
                if (hostInfo.address().isNull())
                    continue;
                hostInfo.setName(ifaceValues.value(QStringLiteral("Name")).toString());
                localDevices.append(hostInfo);
            }
        }
    }
    return localDevices;
}

void QBluetoothLocalDevice::requestPairing(const QBluetoothAddress &address, Pairing pairing)
{
    if (!isValid() || address.isNull()) {
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothLocalDevice::Error,
                                        QBluetoothLocalDevice::PairingError));
        return;
    }

    const Pairing current_pairing = pairingStatus(address);
    if (current_pairing == pairing) {
        if (d_ptr->adapterBluez5) {
            // A possibly running discovery or pending pairing request should be canceled
            if (d_ptr->pairingDiscoveryTimer && d_ptr->pairingDiscoveryTimer->isActive()) {
                d_ptr->pairingDiscoveryTimer->stop();
            }

            if (d_ptr->pairingTarget) {
                qCDebug(QT_BT_BLUEZ) << "Cancelling pending pairing request to" << d_ptr->pairingTarget->address();
                QDBusPendingReply<> cancelReply = d_ptr->pairingTarget->CancelPairing();
                cancelReply.waitForFinished();
                delete d_ptr->pairingTarget;
                d_ptr->pairingTarget = nullptr;
            }

        }
        QMetaObject::invokeMethod(this, "pairingFinished", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothAddress, address),
                                  Q_ARG(QBluetoothLocalDevice::Pairing, pairing));
        return;
    }

    d_ptr->requestPairingBluez5(address, pairing);
}

void QBluetoothLocalDevicePrivate::requestPairingBluez5(const QBluetoothAddress &targetAddress,
                                                        QBluetoothLocalDevice::Pairing targetPairing)
{
    if (!isValid())
        return;

    //are we already discovering something? -> abort those attempts
    if (pairingDiscoveryTimer && pairingDiscoveryTimer->isActive()) {
        pairingDiscoveryTimer->stop();
        QtBluezDiscoveryManager::instance()->unregisterDiscoveryInterest(
                    adapterBluez5->path());
    }

    if (pairingTarget) {
        delete pairingTarget;
        pairingTarget = nullptr;
    }

    // pairing implies that the device was found
    // if we cannot find it we may have to turn on Discovery mode for a limited amount of time

    // check device doesn't already exist
    QDBusPendingReply<ManagedObjectList> reply = managerBluez5->GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError()) {
        emit q_ptr->error(QBluetoothLocalDevice::PairingError);
        return;
    }

    ManagedObjectList managedObjectList = reply.value();
    for (ManagedObjectList::const_iterator it = managedObjectList.constBegin(); it != managedObjectList.constEnd(); ++it) {
        const QDBusObjectPath &path = it.key();
        const InterfaceList &ifaceList = it.value();

        for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd(); ++jt) {
            const QString &iface = jt.key();

            if (iface == QStringLiteral("org.bluez.Device1")) {

                OrgBluezDevice1Interface device(QStringLiteral("org.bluez"),
                                                path.path(),
                                                QDBusConnection::systemBus());
                if (targetAddress == QBluetoothAddress(device.address())) {
                    qCDebug(QT_BT_BLUEZ) << "Initiating direct pair to" << targetAddress.toString();
                    //device exist -> directly work with it
                    processPairingBluez5(path.path(), targetPairing);
                    return;
                }
            }
        }
    }

    //no device matching -> turn on discovery
    QtBluezDiscoveryManager::instance()->registerDiscoveryInterest(adapterBluez5->path());

    address = targetAddress;
    pairing = targetPairing;
    if (!pairingDiscoveryTimer) {
        pairingDiscoveryTimer = new QTimer(this);
        pairingDiscoveryTimer->setSingleShot(true);
        pairingDiscoveryTimer->setInterval(20000); //20s
        connect(pairingDiscoveryTimer, &QTimer::timeout,
                this, &QBluetoothLocalDevicePrivate::pairingDiscoveryTimedOut);
    }

    qCDebug(QT_BT_BLUEZ) << "Initiating discovery for pairing on" << targetAddress.toString();
    pairingDiscoveryTimer->start();
}

/*!
 * \internal
 *
 * Found a matching device. Now we must ensure its pairing/trusted state is as desired.
 * If it has to be paired then we need another roundtrip through the event loop
 * while we wait for the user to accept the pairing dialogs.
 */
void QBluetoothLocalDevicePrivate::processPairingBluez5(const QString &objectPath,
                                                        QBluetoothLocalDevice::Pairing target)
{
    if (pairingTarget)
        delete pairingTarget;

    //stop possibly running discovery
    if (pairingDiscoveryTimer && pairingDiscoveryTimer->isActive()) {
        pairingDiscoveryTimer->stop();

        QtBluezDiscoveryManager::instance()->unregisterDiscoveryInterest(
                    adapterBluez5->path());
    }

    pairingTarget = new OrgBluezDevice1Interface(QStringLiteral("org.bluez"), objectPath,
                                                 QDBusConnection::systemBus(), this);
    const QBluetoothAddress targetAddress(pairingTarget->address());

    Q_Q(QBluetoothLocalDevice);

    switch (target) {
    case QBluetoothLocalDevice::Unpaired: {
        delete pairingTarget;
        pairingTarget = nullptr;

        QDBusPendingReply<> removeReply = adapterBluez5->RemoveDevice(QDBusObjectPath(objectPath));
        auto watcher = new QDBusPendingCallWatcher(removeReply, this);
        connect(watcher, &QDBusPendingCallWatcher::finished,
                this, [q, targetAddress](QDBusPendingCallWatcher* watcher){
            QDBusPendingReply<> reply = *watcher;
            if (reply.isError())
                emit q->error(QBluetoothLocalDevice::PairingError);
            else
                emit q->pairingFinished(targetAddress, QBluetoothLocalDevice::Unpaired);

            watcher->deleteLater();
        });
        break;
    }
    case QBluetoothLocalDevice::Paired:
    case QBluetoothLocalDevice::AuthorizedPaired:
        pairing = target;

        if (!pairingTarget->paired()) {
            qCDebug(QT_BT_BLUEZ) << "Sending pairing request to" << pairingTarget->address();
            //initiate the pairing
            QDBusPendingReply<> pairReply = pairingTarget->Pair();
            QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pairReply, this);
            connect(watcher, &QDBusPendingCallWatcher::finished,
                    this, &QBluetoothLocalDevicePrivate::pairingCompleted);
            return;
        }

        //already paired but Trust level must be adjusted
        if (target == QBluetoothLocalDevice::AuthorizedPaired && !pairingTarget->trusted())
            pairingTarget->setTrusted(true);
        else if (target == QBluetoothLocalDevice::Paired && pairingTarget->trusted())
            pairingTarget->setTrusted(false);

        delete pairingTarget;
        pairingTarget = nullptr;

        emit q->pairingFinished(targetAddress, target);

        break;
    default:
        break;
    }
}

void QBluetoothLocalDevicePrivate::pairingDiscoveryTimedOut()
{
    qCWarning(QT_BT_BLUEZ) << "Discovery for pairing purposes failed. Cannot find parable device.";

    QtBluezDiscoveryManager::instance()->unregisterDiscoveryInterest(
                adapterBluez5->path());

    emit q_ptr->error(QBluetoothLocalDevice::PairingError);
}

QBluetoothLocalDevice::Pairing QBluetoothLocalDevice::pairingStatus(
    const QBluetoothAddress &address) const
{
    if (address.isNull())
        return Unpaired;

    if (isValid())
    {
        QDBusPendingReply<ManagedObjectList> reply = d_ptr->managerBluez5->GetManagedObjects();
        reply.waitForFinished();
        if (reply.isError())
            return Unpaired;

        ManagedObjectList managedObjectList = reply.value();
        for (ManagedObjectList::const_iterator it = managedObjectList.constBegin(); it != managedObjectList.constEnd(); ++it) {
            const QDBusObjectPath &path = it.key();
            const InterfaceList &ifaceList = it.value();

            for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd(); ++jt) {
                const QString &iface = jt.key();

                if (iface == QStringLiteral("org.bluez.Device1")) {

                    OrgBluezDevice1Interface device(QStringLiteral("org.bluez"),
                                                    path.path(),
                                                    QDBusConnection::systemBus());

                    if (address == QBluetoothAddress(device.address())) {
                        if (device.trusted() && device.paired())
                            return AuthorizedPaired;
                        else if (device.paired())
                            return Paired;
                        else
                            return Unpaired;
                    }
                }
            }
        }
    }

    return Unpaired;
}

QBluetoothLocalDevicePrivate::QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *q,
                                                           QBluetoothAddress address) :
        localAddress(address),
        pendingHostModeChange(-1),
        q_ptr(q)
{
    registerQBluetoothLocalDeviceMetaType();

    if (isBluez5()) // TODO registers DBUS meta types -> make registration independent of this fct
                    // call
        initializeAdapterBluez5();

    connectDeviceChanges();
}

bool objectPathIsForThisDevice(const QString &adapterPath, const QString &objectPath)
{
    return (!adapterPath.isEmpty() && objectPath.startsWith(adapterPath));
}

void QBluetoothLocalDevicePrivate::connectDeviceChanges()
{
    if (isValid()) {
        //setup property change notifications for all existing devices
        QDBusPendingReply<ManagedObjectList> reply = managerBluez5->GetManagedObjects();
        reply.waitForFinished();
        if (reply.isError())
            return;

        OrgFreedesktopDBusPropertiesInterface *monitor = nullptr;

        ManagedObjectList managedObjectList = reply.value();
        for (ManagedObjectList::const_iterator it = managedObjectList.constBegin(); it != managedObjectList.constEnd(); ++it) {
            const QDBusObjectPath &path = it.key();
            const InterfaceList &ifaceList = it.value();

            // don't track connected devices from other adapters but the current
            if (!objectPathIsForThisDevice(deviceAdapterPath, path.path()))
                continue;

            for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd(); ++jt) {
                const QString &iface = jt.key();
                const QVariantMap &ifaceValues = jt.value();

                if (iface == QStringLiteral("org.bluez.Device1")) {
                    monitor = new OrgFreedesktopDBusPropertiesInterface(QStringLiteral("org.bluez"),
                                                                        path.path(),
                                                                        QDBusConnection::systemBus(), this);
                    connect(monitor, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged,
                            this, &QBluetoothLocalDevicePrivate::PropertiesChanged);
                    deviceChangeMonitors.insert(path.path(), monitor);

                    if (ifaceValues.value(QStringLiteral("Connected"), false).toBool()) {
                        QBluetoothAddress address(ifaceValues.value(QStringLiteral("Address")).toString());
                        connectedDevicesSet.insert(address);
                    }
                }
            }
        }
    }
}

QBluetoothLocalDevicePrivate::~QBluetoothLocalDevicePrivate()
{
    delete msgConnection;
    delete adapterBluez5;
    delete adapterProperties;
    delete managerBluez5;
    delete pairingTarget;

    qDeleteAll(deviceChangeMonitors);
}

void QBluetoothLocalDevicePrivate::initializeAdapterBluez5()
{
    if (adapterBluez5)
        return;

    //get all local adapters
    if (!managerBluez5)
        managerBluez5 = new OrgFreedesktopDBusObjectManagerInterface(
                                                     QStringLiteral("org.bluez"),
                                                     QStringLiteral("/"),
                                                     QDBusConnection::systemBus(), this);

    connect(managerBluez5, &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
            this, &QBluetoothLocalDevicePrivate::InterfacesAdded);
    connect(managerBluez5, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
            this, &QBluetoothLocalDevicePrivate::InterfacesRemoved);

    bool ok = true;
    const QString adapterPath = findAdapterForAddress(localAddress, &ok);
    if (!ok || adapterPath.isEmpty())
        return;

    deviceAdapterPath = adapterPath;
    adapterBluez5 = new OrgBluezAdapter1Interface(QStringLiteral("org.bluez"),
                                                   adapterPath,
                                                   QDBusConnection::systemBus(), this);

    if (adapterBluez5) {
        //hook up propertiesChanged for current adapter
        adapterProperties = new OrgFreedesktopDBusPropertiesInterface(
                    QStringLiteral("org.bluez"), adapterBluez5->path(),
                    QDBusConnection::systemBus(), this);
        connect(adapterProperties, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged,
                this, &QBluetoothLocalDevicePrivate::PropertiesChanged);
    }

    currentMode = static_cast<QBluetoothLocalDevice::HostMode>(-1);
}

void QBluetoothLocalDevicePrivate::PropertiesChanged(const QString &interface,
                                                     const QVariantMap &changed_properties,
                                                     const QStringList &/*invalidated_properties*/,
                                                     const QDBusMessage &)
{
    //qDebug() << "Change" << interface << changed_properties;
    if (interface == QStringLiteral("org.bluez.Adapter1")) {
        //update host mode
        if (changed_properties.contains(QStringLiteral("Discoverable"))
                || changed_properties.contains(QStringLiteral("Powered"))) {

            QBluetoothLocalDevice::HostMode mode;

            if (!adapterBluez5->powered()) {
                mode = QBluetoothLocalDevice::HostPoweredOff;
            } else {
                if (adapterBluez5->discoverable())
                    mode = QBluetoothLocalDevice::HostDiscoverable;
                else
                    mode = QBluetoothLocalDevice::HostConnectable;

                if (pendingHostModeChange != -1) {

                    if (static_cast<int>(mode) != pendingHostModeChange) {
                        adapterBluez5->setDiscoverable(
                            pendingHostModeChange
                            == static_cast<int>(QBluetoothLocalDevice::HostDiscoverable));
                        pendingHostModeChange = -1;
                        return;
                    }
                    pendingHostModeChange = -1;
                }
            }

            if (mode != currentMode)
                emit q_ptr->hostModeStateChanged(mode);

            currentMode = mode;
        }
    } else if (interface == QStringLiteral("org.bluez.Device1")
               && changed_properties.contains(QStringLiteral("Connected"))) {
        // update list of connected devices
        OrgFreedesktopDBusPropertiesInterface *senderIface =
                qobject_cast<OrgFreedesktopDBusPropertiesInterface*>(sender());
        if (!senderIface)
            return;

        const QString currentPath = senderIface->path();
        bool isConnected = changed_properties.value(QStringLiteral("Connected"), false).toBool();
        OrgBluezDevice1Interface device(QStringLiteral("org.bluez"), currentPath,
                                        QDBusConnection::systemBus());
        const QBluetoothAddress changedAddress(device.address());
        bool isInSet = connectedDevicesSet.contains(changedAddress);
        if (isConnected && !isInSet) {
            connectedDevicesSet.insert(changedAddress);
            emit q_ptr->deviceConnected(changedAddress);
        } else if (!isConnected && isInSet) {
            connectedDevicesSet.remove(changedAddress);
            emit q_ptr->deviceDisconnected(changedAddress);
        }
    }
}

void QBluetoothLocalDevicePrivate::InterfacesAdded(const QDBusObjectPath &object_path, InterfaceList interfaces_and_properties)
{
    if (interfaces_and_properties.contains(QStringLiteral("org.bluez.Device1"))
        && !deviceChangeMonitors.contains(object_path.path())) {
        // a new device was added which we need to add to list of known devices

        if (objectPathIsForThisDevice(deviceAdapterPath, object_path.path())) {
            OrgFreedesktopDBusPropertiesInterface *monitor = new OrgFreedesktopDBusPropertiesInterface(
                                               QStringLiteral("org.bluez"),
                                               object_path.path(),
                                               QDBusConnection::systemBus());
            connect(monitor, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged,
                    this, &QBluetoothLocalDevicePrivate::PropertiesChanged);
            deviceChangeMonitors.insert(object_path.path(), monitor);

            const QVariantMap ifaceValues = interfaces_and_properties.value(QStringLiteral("org.bluez.Device1"));
            if (ifaceValues.value(QStringLiteral("Connected"), false).toBool()) {
                QBluetoothAddress address(ifaceValues.value(QStringLiteral("Address")).toString());
                connectedDevicesSet.insert(address);
                emit q_ptr->deviceConnected(address);
            }
        }
    }

    if (pairingDiscoveryTimer && pairingDiscoveryTimer->isActive()
        && interfaces_and_properties.contains(QStringLiteral("org.bluez.Device1"))) {
        //device discovery for pairing found new remote device
        OrgBluezDevice1Interface device(QStringLiteral("org.bluez"),
                                        object_path.path(), QDBusConnection::systemBus());
        if (!address.isNull() && address == QBluetoothAddress(device.address()))
            processPairingBluez5(object_path.path(), pairing);
    }
}

void QBluetoothLocalDevicePrivate::InterfacesRemoved(const QDBusObjectPath &object_path,
                                                     const QStringList &interfaces)
{
    if (deviceChangeMonitors.contains(object_path.path())
            && interfaces.contains(QLatin1String("org.bluez.Device1"))) {

        if (objectPathIsForThisDevice(deviceAdapterPath, object_path.path())) {
            //a device was removed
            delete deviceChangeMonitors.take(object_path.path());

            //the path contains the address (e.g.: /org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX)
            //-> use it to update current list of connected devices
            QString addressString = object_path.path().right(17);
            addressString.replace(QStringLiteral("_"), QStringLiteral(":"));
            const QBluetoothAddress address(addressString);
            bool found = connectedDevicesSet.remove(address);
            if (found)
                emit q_ptr->deviceDisconnected(address);
        }
    }

    if (adapterBluez5
            && object_path.path() == adapterBluez5->path()
            && interfaces.contains(QLatin1String("org.bluez.Adapter1"))) {
        qCDebug(QT_BT_BLUEZ) << "Adapter" << adapterBluez5->path() << "was removed";
        // current adapter was removed -> invalidate the instance
        delete adapterBluez5;
        adapterBluez5 = nullptr;
        managerBluez5->deleteLater();
        managerBluez5 = nullptr;
        delete adapterProperties;
        adapterProperties = nullptr;

        delete pairingTarget;
        pairingTarget = nullptr;

        // turn  off connectivity monitoring
        qDeleteAll(deviceChangeMonitors);
        deviceChangeMonitors.clear();
        connectedDevicesSet.clear();
    }
}

bool QBluetoothLocalDevicePrivate::isValid() const
{
    return adapterBluez5 && managerBluez5;
}

void QBluetoothLocalDevicePrivate::RequestConfirmation(const QDBusObjectPath &in0, uint in1)
{
    Q_UNUSED(in0);
    Q_Q(QBluetoothLocalDevice);
    setDelayedReply(true);
    msgConfirmation = message();
    msgConnection = new QDBusConnection(connection());
    emit q->pairingDisplayConfirmation(address, QString::fromLatin1("%1").arg(in1));
}


QList<QBluetoothAddress> QBluetoothLocalDevicePrivate::connectedDevices() const
{
    return connectedDevicesSet.values();
}

void QBluetoothLocalDevice::pairingConfirmation(bool confirmation)
{
    if (!d_ptr
        || !d_ptr->msgConfirmation.isReplyRequired()
        || !d_ptr->msgConnection)
        return;

    if (confirmation) {
        QDBusMessage msg = d_ptr->msgConfirmation.createReply(QVariant(true));
        d_ptr->msgConnection->send(msg);
    } else {
        QDBusMessage error
            = d_ptr->msgConfirmation.createErrorReply(QDBusError::AccessDenied,
                                                      QStringLiteral("Pairing rejected"));
        d_ptr->msgConnection->send(error);
    }
    delete d_ptr->msgConnection;
    d_ptr->msgConnection = nullptr;
}

QString QBluetoothLocalDevicePrivate::RequestPinCode(const QDBusObjectPath &in0)
{
    Q_UNUSED(in0);
    Q_Q(QBluetoothLocalDevice);
    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << in0.path();
    // seeded in constructor, 6 digit pin
    QString pin = QString::fromLatin1("%1").arg(QRandomGenerator::global()->bounded(1000000));
    pin = QString::fromLatin1("%1").arg(pin, 6, QLatin1Char('0'));

    emit q->pairingDisplayPinCode(address, pin);
    return pin;
}

void QBluetoothLocalDevicePrivate::pairingCompleted(QDBusPendingCallWatcher *watcher)
{
    Q_Q(QBluetoothLocalDevice);
    QDBusPendingReply<> reply = *watcher;

    if (reply.isError()) {
        qCWarning(QT_BT_BLUEZ) << "Failed to create pairing" << reply.error().name();
        if (reply.error().name() != QStringLiteral("org.bluez.Error.AuthenticationCanceled"))
            emit q->error(QBluetoothLocalDevice::PairingError);
        watcher->deleteLater();
        return;
    }

    if (adapterBluez5) {
        if (!pairingTarget) {
            qCWarning(QT_BT_BLUEZ) << "Pairing target expected but found null pointer.";
            emit q->error(QBluetoothLocalDevice::PairingError);
            watcher->deleteLater();
            return;
        }

        if (!pairingTarget->paired()) {
            qCWarning(QT_BT_BLUEZ) << "Device was not paired as requested";
            emit q->error(QBluetoothLocalDevice::PairingError);
            watcher->deleteLater();
            return;
        }

        const QBluetoothAddress targetAddress(pairingTarget->address());

        if (pairing == QBluetoothLocalDevice::AuthorizedPaired && !pairingTarget->trusted())
            pairingTarget->setTrusted(true);
        else if (pairing == QBluetoothLocalDevice::Paired && pairingTarget->trusted())
            pairingTarget->setTrusted(false);

        delete pairingTarget;
        pairingTarget = nullptr;

        emit q->pairingFinished(targetAddress, pairing);
    }

    watcher->deleteLater();
}

void QBluetoothLocalDevicePrivate::Authorize(const QDBusObjectPath &in0, const QString &in1)
{
    Q_UNUSED(in0);
    Q_UNUSED(in1);
    // TODO implement this
    qCDebug(QT_BT_BLUEZ) << "Got authorize for" << in0.path() << in1;
}

void QBluetoothLocalDevicePrivate::Cancel()
{
    // TODO implement this
    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO;
}

void QBluetoothLocalDevicePrivate::Release()
{
    // TODO implement this
    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO;
}

void QBluetoothLocalDevicePrivate::ConfirmModeChange(const QString &in0)
{
    Q_UNUSED(in0);
    // TODO implement this
    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << in0;
}

void QBluetoothLocalDevicePrivate::DisplayPasskey(const QDBusObjectPath &in0, uint in1, uchar in2)
{
    Q_UNUSED(in0);
    Q_UNUSED(in1);
    Q_UNUSED(in2);
    // TODO implement this
    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << in0.path() << in1 << in2;
}

uint QBluetoothLocalDevicePrivate::RequestPasskey(const QDBusObjectPath &in0)
{
    Q_UNUSED(in0);
    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO;
    return (QRandomGenerator::global()->bounded(1000000));
}

#include "moc_qbluetoothlocaldevice_p.cpp"

QT_END_NAMESPACE

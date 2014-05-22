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
#include <QtDBus/QDBusContext>

#include "qbluetoothlocaldevice.h"
#include "qbluetoothaddress.h"
#include "qbluetoothlocaldevice_p.h"

#include "bluez/manager_p.h"
#include "bluez/adapter_p.h"
#include "bluez/agent_p.h"
#include "bluez/device_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

static const QLatin1String agentPath("/qt/agent");

inline uint qHash(const QBluetoothAddress &address)
{
    return ::qHash(address.toUInt64());
}

QBluetoothLocalDevice::QBluetoothLocalDevice(QObject *parent) :
    QObject(parent), d_ptr(new QBluetoothLocalDevicePrivate(this))
{
}

QBluetoothLocalDevice::QBluetoothLocalDevice(const QBluetoothAddress &address, QObject *parent) :
    QObject(parent), d_ptr(new QBluetoothLocalDevicePrivate(this, address))
{
}

QString QBluetoothLocalDevice::name() const
{
    if (!d_ptr || !d_ptr->adapter)
        return QString();

    QDBusPendingReply<QVariantMap> reply = d_ptr->adapter->GetProperties();
    reply.waitForFinished();
    if (reply.isError())
        return QString();

    return reply.value().value(QLatin1String("Name")).toString();
}

QBluetoothAddress QBluetoothLocalDevice::address() const
{
    if (!d_ptr || !d_ptr->adapter)
        return QBluetoothAddress();

    QDBusPendingReply<QVariantMap> reply = d_ptr->adapter->GetProperties();
    reply.waitForFinished();
    if (reply.isError())
        return QBluetoothAddress();

    return QBluetoothAddress(reply.value().value(QLatin1String("Address")).toString());
}

void QBluetoothLocalDevice::powerOn()
{
    if (!d_ptr || !d_ptr->adapter)
        return;

    d_ptr->adapter->SetProperty(QLatin1String("Powered"), QDBusVariant(QVariant::fromValue(true)));
}

void QBluetoothLocalDevice::setHostMode(QBluetoothLocalDevice::HostMode mode)
{
    if (!d_ptr || !d_ptr->adapter)
        return;

    switch (mode) {
    case HostDiscoverableLimitedInquiry:
    case HostDiscoverable:
        if (hostMode() == HostPoweredOff) {
            //We first have to wait for BT to be powered on,
            //then we can set the host mode correctly
            d_ptr->pendingHostModeChange = (int) HostDiscoverable;
            d_ptr->adapter->SetProperty(QStringLiteral("Powered"),
                                        QDBusVariant(QVariant::fromValue(true)));
        } else {
            d_ptr->adapter->SetProperty(QStringLiteral("Discoverable"),
                                QDBusVariant(QVariant::fromValue(true)));
        }
        break;
    case HostConnectable:
        if (hostMode() == HostPoweredOff) {
            d_ptr->pendingHostModeChange = (int) HostConnectable;
            d_ptr->adapter->SetProperty(QStringLiteral("Powered"),
                                        QDBusVariant(QVariant::fromValue(true)));
        } else {
            d_ptr->adapter->SetProperty(QStringLiteral("Discoverable"),
                                QDBusVariant(QVariant::fromValue(false)));
        }
        break;
    case HostPoweredOff:
        d_ptr->adapter->SetProperty(QLatin1String("Powered"),
                                QDBusVariant(QVariant::fromValue(false)));
        break;
    }
}

QBluetoothLocalDevice::HostMode QBluetoothLocalDevice::hostMode() const
{
    if (!d_ptr || !d_ptr->adapter)
        return HostPoweredOff;

    QDBusPendingReply<QVariantMap> reply = d_ptr->adapter->GetProperties();
    reply.waitForFinished();
    if (reply.isError())
        return HostPoweredOff;

    if (!reply.value().value(QLatin1String("Powered")).toBool())
        return HostPoweredOff;
    else if (reply.value().value(QLatin1String("Discoverable")).toBool())
        return HostDiscoverable;
    else if (reply.value().value(QLatin1String("Powered")).toBool())
        return HostConnectable;

    return HostPoweredOff;
}

QList<QBluetoothAddress> QBluetoothLocalDevice::connectedDevices() const
{
    return d_ptr->connectedDevices();
}

QList<QBluetoothHostInfo> QBluetoothLocalDevice::allDevices()
{
    QList<QBluetoothHostInfo> localDevices;

    OrgBluezManagerInterface manager(QLatin1String("org.bluez"), QLatin1String("/"),
                                     QDBusConnection::systemBus());

    QDBusPendingReply<QList<QDBusObjectPath> > reply = manager.ListAdapters();
    reply.waitForFinished();
    if (reply.isError())
        return localDevices;


    foreach (const QDBusObjectPath &path, reply.value()) {
        QBluetoothHostInfo hostinfo;
        OrgBluezAdapterInterface adapter(QLatin1String("org.bluez"), path.path(),
                                         QDBusConnection::systemBus());

        QDBusPendingReply<QVariantMap> reply = adapter.GetProperties();
        reply.waitForFinished();
        if (reply.isError())
            continue;

        hostinfo.setAddress(QBluetoothAddress(reply.value().value(QLatin1String("Address")).toString()));
        hostinfo.setName(reply.value().value(QLatin1String("Name")).toString());

        localDevices.append(hostinfo);
    }

    return localDevices;
}

static inline OrgBluezDeviceInterface *getDevice(const QBluetoothAddress &address, QBluetoothLocalDevicePrivate *d_ptr)
{
    if (!d_ptr || !d_ptr->adapter)
        return 0;
    QDBusPendingReply<QDBusObjectPath> reply = d_ptr->adapter->FindDevice(address.toString());
    reply.waitForFinished();
    if(reply.isError()){
        qCWarning(QT_BT_BLUEZ) << Q_FUNC_INFO << "reply failed" << reply.error();
        return 0;
    }

    QDBusObjectPath path = reply.value();

    return new OrgBluezDeviceInterface(QLatin1String("org.bluez"), path.path(),
                                   QDBusConnection::systemBus());
}

void QBluetoothLocalDevice::requestPairing(const QBluetoothAddress &address, Pairing pairing)
{
    if (!isValid() || address.isNull()) {
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothLocalDevice::Error, QBluetoothLocalDevice::PairingError));
        return;
    }

    const Pairing current_pairing = pairingStatus(address);
    if (current_pairing == pairing) {
        QMetaObject::invokeMethod(this, "pairingFinished", Qt::QueuedConnection, Q_ARG(QBluetoothAddress, address),
                                  Q_ARG(QBluetoothLocalDevice::Pairing, pairing));
        return;
    }

    if(pairing == Paired || pairing == AuthorizedPaired) {

        d_ptr->address = address;
        d_ptr->pairing = pairing;

        if(!d_ptr->agent){
            d_ptr->agent = new OrgBluezAgentAdaptor(d_ptr);
            bool res = QDBusConnection::systemBus().registerObject(d_ptr->agent_path, d_ptr);
            if(!res){
                QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                          Q_ARG(QBluetoothLocalDevice::Error, QBluetoothLocalDevice::PairingError));
                qCWarning(QT_BT_BLUEZ) << "Failed to register agent";
                return;
            }
        }

        if(current_pairing == Paired && pairing == AuthorizedPaired){
            OrgBluezDeviceInterface *device = getDevice(address, d_ptr);
            if (!device) {
                QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                          Q_ARG(QBluetoothLocalDevice::Error, QBluetoothLocalDevice::PairingError));
                return;
            }
            QDBusPendingReply<> deviceReply = device->SetProperty(QLatin1String("Trusted"), QDBusVariant(true));
            deviceReply.waitForFinished();
            if(deviceReply.isError()){
                qCWarning(QT_BT_BLUEZ) << Q_FUNC_INFO << "reply failed" << deviceReply.error();
                QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                          Q_ARG(QBluetoothLocalDevice::Error, QBluetoothLocalDevice::PairingError));
                return;
            }
            delete device;
            QMetaObject::invokeMethod(this, "pairingFinished", Qt::QueuedConnection, Q_ARG(QBluetoothAddress, address),
                                      Q_ARG(QBluetoothLocalDevice::Pairing, QBluetoothLocalDevice::AuthorizedPaired));
        }
        else if(current_pairing == AuthorizedPaired && pairing == Paired){
            OrgBluezDeviceInterface *device = getDevice(address, d_ptr);
            if (!device) {
                QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                          Q_ARG(QBluetoothLocalDevice::Error, QBluetoothLocalDevice::PairingError));
                return;
            }
            QDBusPendingReply<> deviceReply = device->SetProperty(QLatin1String("Trusted"), QDBusVariant(false));
            deviceReply.waitForFinished();
            if(deviceReply.isError()){
                qCWarning(QT_BT_BLUEZ) << Q_FUNC_INFO << "reply failed" << deviceReply.error();
                QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                          Q_ARG(QBluetoothLocalDevice::Error, QBluetoothLocalDevice::PairingError));
                return;
            }
            delete device;
            QMetaObject::invokeMethod(this, "pairingFinished", Qt::QueuedConnection, Q_ARG(QBluetoothAddress, address),
                                      Q_ARG(QBluetoothLocalDevice::Pairing, QBluetoothLocalDevice::Paired));
        }
        else {
            QDBusPendingReply<QDBusObjectPath> reply =
                    d_ptr->adapter->CreatePairedDevice(address.toString(),
                                                       QDBusObjectPath(d_ptr->agent_path),
                                                       QLatin1String("NoInputNoOutput"));

            QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
            connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), d_ptr, SLOT(pairingCompleted(QDBusPendingCallWatcher*)));

            if(reply.isError())
                qCWarning(QT_BT_BLUEZ) << Q_FUNC_INFO << reply.error() << d_ptr->agent_path;
        }
    }
    else if(pairing == Unpaired) {
        QDBusPendingReply<QDBusObjectPath> reply = this->d_ptr->adapter->FindDevice(address.toString());
        reply.waitForFinished();
        if(reply.isError()) {
            qCWarning(QT_BT_BLUEZ) << Q_FUNC_INFO << "failed to find device" << reply.error();
            QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                      Q_ARG(QBluetoothLocalDevice::Error, QBluetoothLocalDevice::PairingError));
            return;
        }
        QDBusPendingReply<> removeReply = this->d_ptr->adapter->RemoveDevice(reply.value());
        removeReply.waitForFinished();
        if(removeReply.isError()){
            qCWarning(QT_BT_BLUEZ) << Q_FUNC_INFO << "failed to remove device" << removeReply.error();
            QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                      Q_ARG(QBluetoothLocalDevice::Error, QBluetoothLocalDevice::PairingError));
        } else {
            QMetaObject::invokeMethod(this, "pairingFinished", Qt::QueuedConnection, Q_ARG(QBluetoothAddress, address),
                                      Q_ARG(QBluetoothLocalDevice::Pairing, QBluetoothLocalDevice::Unpaired));
        }
    }
    return;
}

QBluetoothLocalDevice::Pairing QBluetoothLocalDevice::pairingStatus(const QBluetoothAddress &address) const
{
    if (address.isNull())
        return Unpaired;

    OrgBluezDeviceInterface *device = getDevice(address, d_ptr);

    if(!device)
        return Unpaired;

    QDBusPendingReply<QVariantMap> deviceReply = device->GetProperties();
    deviceReply.waitForFinished();
    if (deviceReply.isError())
        return Unpaired;

    QVariantMap map = deviceReply.value();

    if (map.value(QLatin1String("Trusted")).toBool() && map.value(QLatin1String("Paired")).toBool())
        return AuthorizedPaired;
    else if (map.value(QLatin1String("Paired")).toBool())
            return Paired;
    else
            return Unpaired;
}

QBluetoothLocalDevicePrivate::QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *q, QBluetoothAddress address)
    : adapter(0), agent(0), manager(0),
      localAddress(address), pendingHostModeChange(-1), msgConnection(0), q_ptr(q)
{
    initializeAdapter();
    connectDeviceChanges();
}

void QBluetoothLocalDevicePrivate::connectDeviceChanges()
{
    if (adapter) { //invalid QBluetoothLocalDevice due to wrong local adapter address
        createCache();
        connect(adapter, SIGNAL(PropertyChanged(QString,QDBusVariant)), SLOT(PropertyChanged(QString,QDBusVariant)));
        connect(adapter, SIGNAL(DeviceCreated(QDBusObjectPath)), SLOT(_q_deviceCreated(QDBusObjectPath)));
        connect(adapter, SIGNAL(DeviceRemoved(QDBusObjectPath)), SLOT(_q_deviceRemoved(QDBusObjectPath)));
    }
}

QBluetoothLocalDevicePrivate::~QBluetoothLocalDevicePrivate()
{
    delete msgConnection;
    delete adapter;
    delete agent;
    delete manager;
    qDeleteAll(devices);
}

void QBluetoothLocalDevicePrivate::initializeAdapter()
{
    if (adapter)
        return;

    QScopedPointer<OrgBluezManagerInterface> man(new OrgBluezManagerInterface(
                            QStringLiteral("org.bluez"), QStringLiteral("/"),
                            QDBusConnection::systemBus()));

    if (localAddress == QBluetoothAddress()) {
        QDBusPendingReply<QDBusObjectPath> reply = man->DefaultAdapter();
        reply.waitForFinished();
        if (reply.isError())
            return;

        adapter = new OrgBluezAdapterInterface(QLatin1String("org.bluez"), reply.value().path(), QDBusConnection::systemBus());
    } else {
        QDBusPendingReply<QList<QDBusObjectPath> > reply = man->ListAdapters();
        reply.waitForFinished();
        if (reply.isError())
            return;

        foreach (const QDBusObjectPath &path, reply.value()) {
            OrgBluezAdapterInterface *tmpAdapter = new OrgBluezAdapterInterface(QLatin1String("org.bluez"), path.path(), QDBusConnection::systemBus());

            QDBusPendingReply<QVariantMap> reply = tmpAdapter->GetProperties();
            reply.waitForFinished();
            if (reply.isError()) {
                delete tmpAdapter;
                continue;
            }

            QBluetoothAddress path_address(reply.value().value(QLatin1String("Address")).toString());

            if (path_address == localAddress) {
                adapter = tmpAdapter;
                break;
            } else {
                delete tmpAdapter;
            }
        }
    }

    // monitor case when local adapter is removed
    manager = man.take();
    connect(manager, SIGNAL(AdapterRemoved(QDBusObjectPath)),
            this, SLOT(adapterRemoved(QDBusObjectPath)));

    currentMode = static_cast<QBluetoothLocalDevice::HostMode>(-1);
    if (adapter) {
        connect(adapter, SIGNAL(PropertyChanged(QString,QDBusVariant)), SLOT(PropertyChanged(QString,QDBusVariant)));

        qsrand(QTime::currentTime().msec());
        agent_path = agentPath;
        agent_path.append(QString::fromLatin1("/%1").arg(qrand()));
    }
}

bool QBluetoothLocalDevicePrivate::isValid() const
{
    return adapter;
}

// Bluez 4
void QBluetoothLocalDevicePrivate::adapterRemoved(const QDBusObjectPath &devicePath)
{
    if (!adapter )
        return;

    if (adapter->path() != devicePath.path())
        return;

    qCDebug(QT_BT_BLUEZ) << "Adapter" << devicePath.path()
                         << "was removed. Invalidating object.";
    // the current adapter was removed
    delete adapter;
    adapter = 0;
    manager->deleteLater();
    manager = 0;

    // stop all pairing related activities
    if (agent) {
        QDBusConnection::systemBus().unregisterObject(agent_path);
        delete agent;
        agent = 0;
    }

    delete msgConnection;
    msgConnection = 0;

    // stop all connectivity monitoring
    qDeleteAll(devices);
    devices.clear();
    connectedDevicesSet.clear();
}

void QBluetoothLocalDevicePrivate::RequestConfirmation(const QDBusObjectPath &in0, uint in1)
{
    Q_UNUSED(in0);
    Q_Q(QBluetoothLocalDevice);
    setDelayedReply(true);
    msgConfirmation = message();
    msgConnection = new QDBusConnection(connection());
    emit q->pairingDisplayConfirmation(address, QString::fromLatin1("%1").arg(in1));
    return;
}

void QBluetoothLocalDevicePrivate::_q_deviceCreated(const QDBusObjectPath &device)
{
    OrgBluezDeviceInterface *deviceInterface =
            new OrgBluezDeviceInterface(QLatin1String("org.bluez"), device.path(), QDBusConnection::systemBus(), this);
    connect(deviceInterface, SIGNAL(PropertyChanged(QString,QDBusVariant)), SLOT(_q_devicePropertyChanged(QString,QDBusVariant)));
    devices << deviceInterface;
    QDBusPendingReply<QVariantMap> properties = deviceInterface->asyncCall(QLatin1String("GetProperties"));

    properties.waitForFinished();
    if (!properties.isValid()) {
        qCritical() << "Unable to get device properties from: " << device.path();
        return;
    }
    const QBluetoothAddress address = QBluetoothAddress(properties.value().value(QLatin1String("Address")).toString());
    const bool connected = properties.value().value(QLatin1String("Connected")).toBool();

    if (connected) {
        connectedDevicesSet.insert(address);
        emit q_ptr->deviceConnected(address);
    } else {
        connectedDevicesSet.remove(address);
        emit q_ptr->deviceDisconnected(address);
    }
}

void QBluetoothLocalDevicePrivate::_q_deviceRemoved(const QDBusObjectPath &device)
{
    foreach (OrgBluezDeviceInterface *deviceInterface, devices) {
        if (deviceInterface->path() == device.path()) {
            devices.remove(deviceInterface);
            delete deviceInterface; //deviceDisconnected is already emitted by _q_devicePropertyChanged
            break;
        }
    }
}

void QBluetoothLocalDevicePrivate::_q_devicePropertyChanged(const QString &property, const QDBusVariant &value)
{
    OrgBluezDeviceInterface *deviceInterface = qobject_cast<OrgBluezDeviceInterface*>(sender());
    if (deviceInterface && property == QLatin1String("Connected")) {
        QDBusPendingReply<QVariantMap> propertiesReply = deviceInterface->GetProperties();
        propertiesReply.waitForFinished();
        if (propertiesReply.isError()) {
            qCWarning(QT_BT_BLUEZ) << propertiesReply.error().message();
            return;
        }
        const QVariantMap properties = propertiesReply.value();
        const QBluetoothAddress address = QBluetoothAddress(properties.value(QLatin1String("Address")).toString());
        const bool connected = value.variant().toBool();

        if (connected) {
            connectedDevicesSet.insert(address);
            emit q_ptr->deviceConnected(address);
        } else {
            connectedDevicesSet.remove(address);
            emit q_ptr->deviceDisconnected(address);
        }
    }
}

void QBluetoothLocalDevicePrivate::createCache()
{
    if (!adapter)
        return;

    QDBusPendingReply<QList<QDBusObjectPath> > reply = adapter->ListDevices();
    reply.waitForFinished();
    if (reply.isError()) {
        qCWarning(QT_BT_BLUEZ) << reply.error().message();
        return;
    }
    foreach (const QDBusObjectPath &device, reply.value()) {
        OrgBluezDeviceInterface *deviceInterface =
                new OrgBluezDeviceInterface(QLatin1String("org.bluez"), device.path(), QDBusConnection::systemBus(), this);
        connect(deviceInterface, SIGNAL(PropertyChanged(QString,QDBusVariant)), SLOT(_q_devicePropertyChanged(QString,QDBusVariant)));
        devices << deviceInterface;

        QDBusPendingReply<QVariantMap> properties = deviceInterface->asyncCall(QLatin1String("GetProperties"));
        properties.waitForFinished();
        if (!properties.isValid()) {
            qCWarning(QT_BT_BLUEZ) << "Unable to get properties for device " << device.path();
            return;
        }

        if (properties.value().value(QLatin1String("Connected")).toBool())
            connectedDevicesSet.insert(QBluetoothAddress(properties.value().value(QLatin1String("Address")).toString()));
    }
}

QList<QBluetoothAddress> QBluetoothLocalDevicePrivate::connectedDevices() const
{
    return connectedDevicesSet.toList();
}

void QBluetoothLocalDevice::pairingConfirmation(bool confirmation)
{
    if(!d_ptr ||
            !d_ptr->msgConfirmation.isReplyRequired() ||
            !d_ptr->msgConnection)
        return;

    if(confirmation){
        QDBusMessage msg = d_ptr->msgConfirmation.createReply(QVariant(true));
        d_ptr->msgConnection->send(msg);
    }
    else {
        QDBusMessage error =
            d_ptr->msgConfirmation.createErrorReply(QDBusError::AccessDenied,
                                                    QLatin1String("Pairing rejected"));
        d_ptr->msgConnection->send(error);
    }
    delete d_ptr->msgConnection;
    d_ptr->msgConnection = 0;
}

QString QBluetoothLocalDevicePrivate::RequestPinCode(const QDBusObjectPath &in0)
{
    Q_UNUSED(in0)
    Q_Q(QBluetoothLocalDevice);
    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << in0.path();
    // seeded in constructor, 6 digit pin
    QString pin = QString::fromLatin1("%1").arg(qrand()&1000000);
    pin = QString::fromLatin1("%1").arg(pin, 6, QLatin1Char('0'));

    emit q->pairingDisplayPinCode(address, pin);
    return pin;
}

void QBluetoothLocalDevicePrivate::pairingCompleted(QDBusPendingCallWatcher *watcher)
{
    Q_Q(QBluetoothLocalDevice);
    QDBusPendingReply<> reply = *watcher;

    if(reply.isError()) {
        qCWarning(QT_BT_BLUEZ) << Q_FUNC_INFO << "failed to create pairing" << reply.error();
        emit q->error(QBluetoothLocalDevice::PairingError);
        delete watcher;
        return;
    }

    QDBusPendingReply<QDBusObjectPath> findReply = adapter->FindDevice(address.toString());
    findReply.waitForFinished();
    if(findReply.isError()) {
        qCWarning(QT_BT_BLUEZ) << Q_FUNC_INFO << "failed to find device" << findReply.error();
        emit q->error(QBluetoothLocalDevice::PairingError);
        delete watcher;
        return;
    }

    OrgBluezDeviceInterface device(QLatin1String("org.bluez"), findReply.value().path(),
                                   QDBusConnection::systemBus());

    if(pairing == QBluetoothLocalDevice::AuthorizedPaired) {
        device.SetProperty(QLatin1String("Trusted"), QDBusVariant(QVariant(true)));
        emit q->pairingFinished(address, QBluetoothLocalDevice::AuthorizedPaired);
    }
    else {
        device.SetProperty(QLatin1String("Trusted"), QDBusVariant(QVariant(false)));
        emit q->pairingFinished(address, QBluetoothLocalDevice::Paired);
    }
    delete watcher;

}

void QBluetoothLocalDevicePrivate::Authorize(const QDBusObjectPath &in0, const QString &in1)
{
    Q_UNUSED(in0)
    Q_UNUSED(in1)
    //TODO implement this
    qCDebug(QT_BT_BLUEZ) << "Got authorize for" << in0.path() << in1;
}

void QBluetoothLocalDevicePrivate::Cancel()
{
    //TODO implement this
    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO;
}

void QBluetoothLocalDevicePrivate::Release()
{
    //TODO implement this
    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO;
}


void QBluetoothLocalDevicePrivate::ConfirmModeChange(const QString &in0)
{
    Q_UNUSED(in0)
    //TODO implement this
    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << in0;
}

void QBluetoothLocalDevicePrivate::DisplayPasskey(const QDBusObjectPath &in0, uint in1, uchar in2)
{
    Q_UNUSED(in0)
    Q_UNUSED(in1)
    Q_UNUSED(in2)
    //TODO implement this
    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << in0.path() << in1 << in2;
}

uint QBluetoothLocalDevicePrivate::RequestPasskey(const QDBusObjectPath &in0)
{
    Q_UNUSED(in0);
    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO;
    return qrand()&0x1000000;
}


void QBluetoothLocalDevicePrivate::PropertyChanged(QString property, QDBusVariant value)
{
    Q_UNUSED(value);

    if (property != QLatin1String("Powered") &&
        property != QLatin1String("Discoverable"))
        return;

    Q_Q(QBluetoothLocalDevice);
    QBluetoothLocalDevice::HostMode mode;

    QDBusPendingReply<QVariantMap> reply = adapter->GetProperties();
    reply.waitForFinished();
    if (reply.isError()){
        qCWarning(QT_BT_BLUEZ) << "Failed to get bluetooth properties for mode change";
        return;
    }

    QVariantMap map = reply.value();

    if(!map.value(QLatin1String("Powered")).toBool()){
        mode = QBluetoothLocalDevice::HostPoweredOff;
    }
    else {
        if (map.value(QLatin1String("Discoverable")).toBool())
            mode = QBluetoothLocalDevice::HostDiscoverable;
        else
            mode = QBluetoothLocalDevice::HostConnectable;

        if (pendingHostModeChange != -1) {
            if ((int)mode != pendingHostModeChange) {
                if (property == QStringLiteral("Powered"))
                    return;
                if (pendingHostModeChange == (int)QBluetoothLocalDevice::HostDiscoverable) {
                    adapter->SetProperty(QStringLiteral("Discoverable"),
                                        QDBusVariant(QVariant::fromValue(true)));
                } else {
                    adapter->SetProperty(QStringLiteral("Discoverable"),
                                        QDBusVariant(QVariant::fromValue(false)));
                }
                pendingHostModeChange = -1;
                return;
            }
        }
    }

    if(mode != currentMode)
        emit q->hostModeStateChanged(mode);

    currentMode = mode;
}

#include "moc_qbluetoothlocaldevice_p.cpp"

QT_END_NAMESPACE

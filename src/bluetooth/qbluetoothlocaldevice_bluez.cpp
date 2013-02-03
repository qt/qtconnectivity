/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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


#include <QDBusContext>

#include "qbluetoothlocaldevice.h"
#include "qbluetoothaddress.h"
#include "qbluetoothlocaldevice_p.h"

#include "bluez/manager_p.h"
#include "bluez/adapter_p.h"
#include "bluez/agent_p.h"
#include "bluez/device_p.h"

QT_BEGIN_NAMESPACE_BLUETOOTH

static const QLatin1String agentPath("/qt/agent");

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
        d_ptr->adapter->SetProperty(QLatin1String("Powered"), QDBusVariant(QVariant::fromValue(true)));
        d_ptr->adapter->SetProperty(QLatin1String("Discoverable"),
                                QDBusVariant(QVariant::fromValue(true)));
        break;
    case HostConnectable:
        d_ptr->adapter->SetProperty(QLatin1String("Powered"), QDBusVariant(QVariant::fromValue(true)));
        d_ptr->adapter->SetProperty(QLatin1String("Discoverable"),
                                QDBusVariant(QVariant::fromValue(false)));
        break;
    case HostPoweredOff:
        d_ptr->adapter->SetProperty(QLatin1String("Powered"),
                                QDBusVariant(QVariant::fromValue(false)));
//        d->adapter->SetProperty(QLatin1String("Discoverable"),
//                                QDBusVariant(QVariant::fromValue(false)));
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
        qDebug() << Q_FUNC_INFO << "reply failed" << reply.error();
        return 0;
    }

    QDBusObjectPath path = reply.value();

    return new OrgBluezDeviceInterface(QLatin1String("org.bluez"), path.path(),
                                   QDBusConnection::systemBus());
}

void QBluetoothLocalDevice::requestPairing(const QBluetoothAddress &address, Pairing pairing)
{

    if(pairing == Paired || pairing == AuthorizedPaired) {

        d_ptr->address = address;
        d_ptr->pairing = pairing;

        if(!d_ptr->agent){
            d_ptr->agent = new OrgBluezAgentAdaptor(d_ptr);
            bool res = QDBusConnection::systemBus().registerObject(d_ptr->agent_path, d_ptr);
            if(!res){
                qDebug() << "Failed to register agent";
                return;
            }
        }

        Pairing current_pairing = pairingStatus(address);
        if(current_pairing == Paired && pairing == AuthorizedPaired){
            OrgBluezDeviceInterface *device = getDevice(address, d_ptr);
            if(!device)
                return;
            QDBusPendingReply<> deviceReply = device->SetProperty(QLatin1String("Trusted"), QDBusVariant(true));
            deviceReply.waitForFinished();
            if(deviceReply.isError()){
                qDebug() << Q_FUNC_INFO << "reply failed" << deviceReply.error();
                return;
            }
            delete device;
            QMetaObject::invokeMethod(this, "pairingFinished", Qt::QueuedConnection, Q_ARG(QBluetoothAddress, address),
                                      Q_ARG(QBluetoothLocalDevice::Pairing, QBluetoothLocalDevice::AuthorizedPaired));
        }
        else if(current_pairing == AuthorizedPaired && pairing == Paired){
            OrgBluezDeviceInterface *device = getDevice(address, d_ptr);
            if(!device)
                return;
            QDBusPendingReply<> deviceReply = device->SetProperty(QLatin1String("Trusted"), QDBusVariant(false));
            deviceReply.waitForFinished();
            if(deviceReply.isError()){
                qDebug() << Q_FUNC_INFO << "reply failed" << deviceReply.error();
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
                qDebug() << Q_FUNC_INFO << reply.error() << d_ptr->agent_path;
        }
    }
    else if(pairing == Unpaired) {
        QDBusPendingReply<QDBusObjectPath> reply = this->d_ptr->adapter->FindDevice(address.toString());
        reply.waitForFinished();
        if(reply.isError()) {
            qDebug() << Q_FUNC_INFO << "failed to find device" << reply.error();
            return;
        }
        QDBusPendingReply<> removeReply = this->d_ptr->adapter->RemoveDevice(reply.value());
        removeReply.waitForFinished();
        if(removeReply.isError()){
            qDebug() << Q_FUNC_INFO << "failed to remove device" << removeReply.error();
        }
        return;
    }
    return;
}

QBluetoothLocalDevice::Pairing QBluetoothLocalDevice::pairingStatus(const QBluetoothAddress &address) const
{
    OrgBluezDeviceInterface *device = getDevice(address, d_ptr);

    if(!device)
        return Unpaired;

    QDBusPendingReply<QVariantMap> deviceReply = device->GetProperties();
    deviceReply.waitForFinished();
    if (deviceReply.isError())
        return Unpaired;

    QVariantMap map = deviceReply.value();

//    qDebug() << "Paired: " << map.value("Paired");


    if (map.value(QLatin1String("Trusted")).toBool() && map.value(QLatin1String("Paired")).toBool())
        return AuthorizedPaired;
    else if (map.value(QLatin1String("Paired")).toBool())
            return Paired;
    else
            return Unpaired;

}

QBluetoothLocalDevicePrivate::QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *q, QBluetoothAddress address)
    : adapter(0), agent(0), localAddress(address), msgConnection(0), q_ptr(q)
{
    initializeAdapter();
}

QBluetoothLocalDevicePrivate::~QBluetoothLocalDevicePrivate()
{
    delete msgConnection;
    delete adapter;
    delete agent;
}

void QBluetoothLocalDevicePrivate::initializeAdapter()
{
    if (adapter)
        return;

    qDebug() << "initialize adapter interface";

    OrgBluezManagerInterface manager(QLatin1String("org.bluez"), QLatin1String("/"), QDBusConnection::systemBus());

    if (localAddress == QBluetoothAddress()) {
        QDBusPendingReply<QDBusObjectPath> reply = manager.DefaultAdapter();
        reply.waitForFinished();
        if (reply.isError())
            return;

        adapter = new OrgBluezAdapterInterface(QLatin1String("org.bluez"), reply.value().path(), QDBusConnection::systemBus());
    } else {
        QDBusPendingReply<QList<QDBusObjectPath> > reply = manager.ListAdapters();
        reply.waitForFinished();
        if (reply.isError())
            return;

        foreach (const QDBusObjectPath &path, reply.value()) {
            OrgBluezAdapterInterface *tmpAdapter = new OrgBluezAdapterInterface(QLatin1String("org.bluez"), path.path(), QDBusConnection::systemBus());

            QDBusPendingReply<QVariantMap> reply = tmpAdapter->GetProperties();
            reply.waitForFinished();
            if (reply.isError())
                continue;

            QBluetoothAddress path_address(reply.value().value(QLatin1String("Address")).toString());

            if (path_address == localAddress) {
                adapter = tmpAdapter;
                break;
            } else {
                delete tmpAdapter;
            }
        }
    }

    currentMode = static_cast<QBluetoothLocalDevice::HostMode>(-1);
    if (adapter) {
        connect(adapter, SIGNAL(PropertyChanged(QString,QDBusVariant)), SLOT(PropertyChanged(QString,QDBusVariant)));

        qsrand(QTime::currentTime().msec());
        agent_path = agentPath;
        agent_path.append(QString::fromLatin1("/%1").arg(qrand()));
    }
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
    Q_Q(QBluetoothLocalDevice);
    qDebug() << Q_FUNC_INFO << in0.path();
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
        qDebug() << Q_FUNC_INFO << "failed to create pairing" << reply.error();
        emit q->pairingFinished(address, QBluetoothLocalDevice::Unpaired);
        delete watcher;
        return;
    }

    QDBusPendingReply<QDBusObjectPath> findReply = adapter->FindDevice(address.toString());
    findReply.waitForFinished();
    if(findReply.isError()) {
        qDebug() << Q_FUNC_INFO << "failed to find device" << findReply.error();
        emit q->pairingFinished(address, QBluetoothLocalDevice::Unpaired);
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
    qDebug() << "Got authorize for" << in0.path() << in1;
}

void QBluetoothLocalDevicePrivate::Cancel()
{
    qDebug() << Q_FUNC_INFO;
}

void QBluetoothLocalDevicePrivate::Release()
{
    qDebug() << Q_FUNC_INFO;
}


void QBluetoothLocalDevicePrivate::ConfirmModeChange(const QString &in0)
{
    qDebug() << Q_FUNC_INFO << in0;
}

void QBluetoothLocalDevicePrivate::DisplayPasskey(const QDBusObjectPath &in0, uint in1, uchar in2)
{
    qDebug() << Q_FUNC_INFO << in0.path() << in1 << in2;
}

uint QBluetoothLocalDevicePrivate::RequestPasskey(const QDBusObjectPath &in0)
{
    Q_UNUSED(in0);
    qDebug() << Q_FUNC_INFO;
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
        qWarning() << "Failed to get bluetooth properties for mode change";
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
    }

    if(mode != currentMode)
        emit q->hostModeStateChanged(mode);

    currentMode = mode;
}

//#include "qbluetoothlocaldevice.moc"
#include "moc_qbluetoothlocaldevice_p.cpp"

QT_END_NAMESPACE_BLUETOOTH

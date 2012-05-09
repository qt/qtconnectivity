/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
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

QTBLUETOOTH_BEGIN_NAMESPACE

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
#ifdef NOKIA_BT_SERVICES
    nokiaBtManServiceInstance()->setPowered(true);
#else
    if (!d_ptr || !d_ptr->adapter)
        return;

    d_ptr->adapter->SetProperty(QLatin1String("Powered"), QDBusVariant(QVariant::fromValue(true)));
#endif
}

void QBluetoothLocalDevice::setHostMode(QBluetoothLocalDevice::HostMode mode)
{
#ifdef NOKIA_BT_SERVICES
    nokiaBtManServiceInstance()->setHostMode(mode);
#else
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
#endif
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
#ifndef NOKIA_BT_SERVICES
    initializeAdapter();
#else
    connect(nokiaBtManServiceInstance(), SIGNAL(poweredChanged(bool)), SLOT(powerStateChanged(bool)));
    nokiaBtManServiceInstance()->acquire();
#endif
}

QBluetoothLocalDevicePrivate::~QBluetoothLocalDevicePrivate()
{
    delete msgConnection;
    delete adapter;
    delete agent;

#ifdef NOKIA_BT_SERVICES
    nokiaBtManServiceInstance()->release();
#endif
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

#ifdef NOKIA_BT_SERVICES
void QBluetoothLocalDevicePrivate::powerStateChanged(bool powered)
{
    Q_Q(QBluetoothLocalDevice);
    QBluetoothLocalDevice::HostMode mode;

    if (powered) {
        initializeAdapter();

        QDBusPendingReply<QVariantMap> reply = adapter->GetProperties();
        reply.waitForFinished();
        if (reply.isError()){
            qWarning() << "Failed to get bluetooth properties";
            return;
        }

        if (reply.value().value(QLatin1String("Discoverable")).toBool()) {
            mode = QBluetoothLocalDevice::HostDiscoverable;
        } else {
            mode = QBluetoothLocalDevice::HostConnectable;
        }
    } else {
        mode = QBluetoothLocalDevice::HostPoweredOff;
        delete msgConnection;
        msgConnection = 0;
        delete adapter;
        adapter = 0;
        delete agent;
        agent = 0;
    }

    if (mode != currentMode) {
        emit q->hostModeStateChanged(mode);
        currentMode = mode;
    }
}
#endif

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


#ifdef NOKIA_BT_SERVICES

NokiaBtManServiceConnection::NokiaBtManServiceConnection():
    m_btmanService(NULL),
    m_refCount(0)
{
}

void NokiaBtManServiceConnection::acquire()
{
    QMutexLocker m(&m_refCountMutex);
    ++m_refCount;
    if (m_btmanService == NULL) {
        connectToBtManService();
    }
}

void NokiaBtManServiceConnection::release()
{
    QMutexLocker m(&m_refCountMutex);
    --m_refCount;
    if (m_refCount == 0) {
        QTimer::singleShot(5000, this, SLOT(disconnectFromBtManService()));
    }
}

void NokiaBtManServiceConnection::connectToBtManService()
{
    if (m_btmanService == NULL) {
        QServiceManager manager;
        QServiceFilter filter(QLatin1String("com.nokia.mt.btmanservice"));

        // find services complying with filter
        QList<QServiceInterfaceDescriptor> foundServices;
        foundServices = manager.findInterfaces(filter);

        if (foundServices.count()) {
            m_btmanService = manager.loadInterface(foundServices.at(0));
        }
        if (m_btmanService) {
            qDebug() << "connected to service:" << m_btmanService;
            connect(m_btmanService, SIGNAL(errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)), SLOT(sfwIPCError(QService::UnrecoverableIPCError)));
            connect(m_btmanService, SIGNAL(powerStateChanged(int)), SLOT(powerStateChanged(int)));
            if (powered()) {
                emit poweredChanged(true);
            }
        } else {
            qDebug() << "failed to connect to btman service";
        }
    } else {
        qDebug() << "already connected to service:" << m_btmanService;
    }
}

void NokiaBtManServiceConnection::disconnectFromBtManService()
{
    // Check if none acquired the service in the meantime
    QMutexLocker m(&m_refCountMutex);
    if (m_refCount == 0 && m_btmanService) {
        qDebug() << "disconnecting from btman service";
        m_btmanService->deleteLater();
        m_btmanService = NULL;
    }
}

bool NokiaBtManServiceConnection::powered() const
{
    int powerState;
    QMetaObject::invokeMethod(m_btmanService, "powerState", Q_RETURN_ARG(int, powerState));
    return powerState == 2; // enabled
}

void NokiaBtManServiceConnection::setPowered(bool powered)
{
    int powerState; // Disabled  = 0, Enabling  = 1, Enabled   = 2, Disabling = 3
    QMetaObject::invokeMethod(m_btmanService, "powerState", Q_RETURN_ARG(int, powerState));

    if ((powered && powerState != 0) || (!powered && powerState != 2)) {
        qWarning() << "cannot change powered (wrong state)";
        return;
    }

    QMetaObject::invokeMethod(m_btmanService, "setPowered", Q_ARG(bool, powered));
}

void NokiaBtManServiceConnection::setHostMode(QBluetoothLocalDevice::HostMode mode)
{
    m_forceDiscoverable = false;
    m_forceConnectable = false;

    switch (mode) {
    case QBluetoothLocalDevice::HostDiscoverableLimitedInquiry:
    case QBluetoothLocalDevice::HostDiscoverable:
        if (powered()) {
            QMetaObject::invokeMethod(m_btmanService, "setDiscoverable", Q_ARG(bool, true));
        } else {
            m_forceDiscoverable = true;
            setPowered(true);
        }
        break;
    case QBluetoothLocalDevice::HostConnectable:
        if (powered()) {
            QMetaObject::invokeMethod(m_btmanService, "setDiscoverable", Q_ARG(bool, false));
        } else {
            m_forceConnectable = true;
            setPowered(true);
        }
        break;
    case QBluetoothLocalDevice::HostPoweredOff:
        if (powered()) {
            setPowered(false);
        }
        break;
    }
}

void NokiaBtManServiceConnection::powerStateChanged(int powerState)
{
    if (powerState == 0) {
        emit poweredChanged(false);
    } else if (powerState == 2) {
        if (m_forceDiscoverable) {
            m_forceDiscoverable = false;
            QMetaObject::invokeMethod(m_btmanService, "setDiscoverable", Q_ARG(bool, true));
        } else if (m_forceConnectable) {
            m_forceConnectable = false;
            QMetaObject::invokeMethod(m_btmanService, "setDiscoverable", Q_ARG(bool, false));
        }
        emit poweredChanged(true);
    }
}

void NokiaBtManServiceConnection::sfwIPCError(QService::UnrecoverableIPCError error)
{
    qDebug() << "Connection to btman service broken:" << error << ". Trying to reconnect...";
    if (m_btmanService) {
        m_btmanService->deleteLater();
        m_btmanService = NULL;
    }
    QMetaObject::invokeMethod(this, "connectToBtManService", Qt::QueuedConnection);
}

#endif

//#include "qbluetoothlocaldevice.moc"
#include "moc_qbluetoothlocaldevice_p.cpp"

QTBLUETOOTH_END_NAMESPACE

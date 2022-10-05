// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "btlocaldevice.h"
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QtBluetooth/QBluetoothServiceInfo>
#include <QtBluetooth/QLowEnergyCharacteristicData>
#include <QtBluetooth/QLowEnergyDescriptorData>
#include <QtBluetooth/QLowEnergyAdvertisingParameters>

#define BTCHAT_DEVICE_ADDR "00:15:83:38:17:C3"

//same uuid as examples/bluetooth/btchat
//the reverse UUID is only used on Android to counter
//https://issuetracker.google.com/issues/37076498 (tracked via QTBUG-61392)
#define TEST_SERVICE_UUID "e8e10f95-1a70-4b27-9ccf-02010264e9c8"
#define TEST_REVERSE_SERVICE_UUID "c8e96402-0102-cf9c-274b-701a950fe1e8"

#define SOCKET_PROTOCOL QBluetoothServiceInfo::RfcommProtocol
//#define SOCKET_PROTOCOL QBluetoothServiceInfo::L2capProtocol

using namespace Qt::Literals::StringLiterals;
static const auto leServiceUuid = "1ff5e37c-ac16-11eb-ae5c-93d3a763feed"_L1;
static const auto leCharacteristicUuid = "2ff4f68e-ac16-11eb-9956-cfe55a8ccafe"_L1;
// Used for finding a matching LE peripheral device. Typically the default BtTestUi is ok
// when running against macOS/iOS/Linux peripheral, but with Android this needs to be adjusted
// to device's name. We can't use bluetooth address for matching as the public address of the
// peripheral may change
static const auto leRemotePeriphreralDeviceName = "BtTestUi"_L1;
static const qsizetype leCharacteristicSize = 4; // Set to 1...512 bytes
static auto leCharacteristicValue = QByteArray{leCharacteristicSize, 0};
static quint8 leValueUpdate = 1;

// String tables to shorten the enum strings to fit the screen estate.
// The values in the tables must be in same order as the corresponding enums
static constexpr const char* controllerStateString[] = {
    "Unconnected",
    "Connecting",
    "Connected",
    "Discovering",
    "Discovered",
    "Closing",
    "Advertising",
};

static constexpr const char* controllerErrorString[] = {
    "None",
    "UnknownError",
    "UnknownRemDev",
    "NetworkError",
    "InvAdapter",
    "ConnectionErr",
    "AdvertisingErr",
    "RemHostClosed",
    "AuthError",
    "MissingPerm",
    "RssiError"
};

static constexpr const char* serviceStateString[] = {
    "InvalidService",
    "RemoteService",
    "RemDiscovering",
    "RemDiscovered",
    "LocalService",
};

static constexpr const char* serviceErrorString[] = {
    "None",
    "Operation",
    "CharWrite",
    "DescWrite",
    "Unknown",
    "CharRead",
    "DescRead",
};

BtLocalDevice::BtLocalDevice(QObject *parent)
    : QObject(parent), securityFlags(QBluetooth::Security::NoSecurity)
{
    localDevice = new QBluetoothLocalDevice(this);
    connect(localDevice, &QBluetoothLocalDevice::errorOccurred, this,
            &BtLocalDevice::errorOccurred);
    connect(localDevice, &QBluetoothLocalDevice::hostModeStateChanged,
            this, &BtLocalDevice::hostModeStateChanged);
    connect(localDevice, &QBluetoothLocalDevice::pairingFinished,
            this, &BtLocalDevice::pairingFinished);
    connect(localDevice, &QBluetoothLocalDevice::deviceConnected,
            this, &BtLocalDevice::connected);
    connect(localDevice, &QBluetoothLocalDevice::deviceDisconnected,
            this, &BtLocalDevice::disconnected);

    if (localDevice->isValid()) {
        deviceAgent = new QBluetoothDeviceDiscoveryAgent(this);
        connect(deviceAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                this, &BtLocalDevice::deviceDiscovered);
        connect(deviceAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
                this, &BtLocalDevice::deviceUpdated);
        connect(deviceAgent, &QBluetoothDeviceDiscoveryAgent::finished,
                this, &BtLocalDevice::discoveryFinished);
        connect(deviceAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this,
                &BtLocalDevice::discoveryError);
        connect(deviceAgent, &QBluetoothDeviceDiscoveryAgent::canceled,
                this, &BtLocalDevice::discoveryCanceled);

        serviceAgent = new QBluetoothServiceDiscoveryAgent(this);
        connect(serviceAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered,
                this, &BtLocalDevice::serviceDiscovered);
        connect(serviceAgent, &QBluetoothServiceDiscoveryAgent::finished,
                this, &BtLocalDevice::serviceDiscoveryFinished);
        connect(serviceAgent, &QBluetoothServiceDiscoveryAgent::canceled,
                this, &BtLocalDevice::serviceDiscoveryCanceled);
        connect(serviceAgent, &QBluetoothServiceDiscoveryAgent::errorOccurred, this,
                &BtLocalDevice::serviceDiscoveryError);

        socket = new QBluetoothSocket(SOCKET_PROTOCOL, this);
        connect(socket, &QBluetoothSocket::stateChanged,
                this, &BtLocalDevice::socketStateChanged);
        connect(socket, &QBluetoothSocket::errorOccurred, this, &BtLocalDevice::socketError);
        connect(socket, &QBluetoothSocket::connected, this, &BtLocalDevice::socketConnected);
        connect(socket, &QBluetoothSocket::disconnected, this, &BtLocalDevice::socketDisconnected);
        connect(socket, &QIODevice::readyRead, this, &BtLocalDevice::readData);
        connect(socket, &QBluetoothSocket::bytesWritten, this, [](qint64 bytesWritten){
            qDebug() << "Bytes Written to Client socket:" << bytesWritten;
        });
        setSecFlags(static_cast<int>(socket->preferredSecurityFlags()));

        server = new QBluetoothServer(SOCKET_PROTOCOL, this);
        connect(server, &QBluetoothServer::newConnection, this, &BtLocalDevice::serverNewConnection);
        connect(server, &QBluetoothServer::errorOccurred, this, &BtLocalDevice::serverError);
    }
}

BtLocalDevice::~BtLocalDevice()
{
    while (!serverSockets.isEmpty())
    {
        QBluetoothSocket* s = serverSockets.takeFirst();
        s->abort();
        s->deleteLater();
    }
}

QBluetooth::SecurityFlags BtLocalDevice::secFlags() const
{
    return securityFlags;
}

void BtLocalDevice::setSecFlags(int newFlags)
{
    QBluetooth::SecurityFlags fl(newFlags);

    if (securityFlags != fl) {
        securityFlags = fl;
        emit secFlagsChanged();
    }
}

QString BtLocalDevice::hostMode() const
{
    if (localDevice) {
        switch (localDevice->hostMode()) {
        case QBluetoothLocalDevice::HostDiscoverable:
            return QStringLiteral("HostMode: Discoverable");
        case QBluetoothLocalDevice::HostConnectable:
            return QStringLiteral("HostMode: Connectable");
        case QBluetoothLocalDevice::HostDiscoverableLimitedInquiry:
            return QStringLiteral("HostMode: DiscoverableLimit");
        case QBluetoothLocalDevice::HostPoweredOff:
            return QStringLiteral("HostMode: Powered Off");
        }
    }

    return QStringLiteral("HostMode: <None>");
}

void BtLocalDevice::setHostMode(int newMode)
{
    if (localDevice)
        localDevice->setHostMode(static_cast<QBluetoothLocalDevice::HostMode>(newMode));
}

void BtLocalDevice::requestPairingUpdate(bool isPairing)
{
    QBluetoothAddress baddr(BTCHAT_DEVICE_ADDR);
    if (!localDevice || baddr.isNull())
        return;

    if (isPairing) {
        //toggle between authorized and non-authorized pairing to achieve better
        //level of testing
        static short pairing = 0;
        if ((pairing%2) == 1)
            localDevice->requestPairing(baddr, QBluetoothLocalDevice::Paired);
        else
            localDevice->requestPairing(baddr, QBluetoothLocalDevice::AuthorizedPaired);
        pairing++;
    } else {
        localDevice->requestPairing(baddr, QBluetoothLocalDevice::Unpaired);
    }

    for (qsizetype i = 0; i < foundTestServers.size(); ++i) {
        if (isPairing)
            localDevice->requestPairing(foundTestServers.at(i).device().address(),
                                    QBluetoothLocalDevice::Paired);
        else
            localDevice->requestPairing(foundTestServers.at(i).device().address(),
                                    QBluetoothLocalDevice::Unpaired);
    }
}

void BtLocalDevice::pairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing)
{
    qDebug() << "(Un)Pairing finished" << address.toString() << pairing;
}

void BtLocalDevice::connected(const QBluetoothAddress &addr)
{
    qDebug() << "Newly connected device" << addr.toString();
}

void BtLocalDevice::disconnected(const QBluetoothAddress &addr)
{
    qDebug() << "Newly disconnected device" << addr.toString();
}

void BtLocalDevice::cycleSecurityFlags()
{
    if (securityFlags.testFlag(QBluetooth::Security::Secure))
        setSecFlags(QBluetooth::SecurityFlags(QBluetooth::Security::NoSecurity));
    else if (securityFlags.testFlag(QBluetooth::Security::Encryption))
        setSecFlags(secFlags() | QBluetooth::Security::Secure);
    else if (securityFlags.testFlag(QBluetooth::Security::Authentication))
        setSecFlags(secFlags() | QBluetooth::Security::Encryption);
    else if (securityFlags.testFlag(QBluetooth::Security::Authorization))
        setSecFlags(secFlags() | QBluetooth::Security::Authentication);
    else
        setSecFlags(secFlags() | QBluetooth::Security::Authorization);
}

void BtLocalDevice::deviceDiscovered(const QBluetoothDeviceInfo &info)
{
    QString services;
    if (info.serviceClasses() & QBluetoothDeviceInfo::PositioningService)
        services += "Position|";
    if (info.serviceClasses() & QBluetoothDeviceInfo::NetworkingService)
        services += "Network|";
    if (info.serviceClasses() & QBluetoothDeviceInfo::RenderingService)
        services += "Rendering|";
    if (info.serviceClasses() & QBluetoothDeviceInfo::CapturingService)
        services += "Capturing|";
    if (info.serviceClasses() & QBluetoothDeviceInfo::ObjectTransferService)
        services += "ObjectTra|";
    if (info.serviceClasses() & QBluetoothDeviceInfo::AudioService)
        services += "Audio|";
    if (info.serviceClasses() & QBluetoothDeviceInfo::TelephonyService)
        services += "Telephony|";
    if (info.serviceClasses() & QBluetoothDeviceInfo::InformationService)
        services += "Information|";

    services.truncate(services.size()-1); //cut last '/'

    qDebug() << "Found new device: " << info.name() << info.isValid() << info.address().toString()
                                     << info.rssi() << info.majorDeviceClass()
                                     << info.minorDeviceClass() << services;
    // With LE we match the device by its name as the public bluetooth address can change
    if (info.name() == leRemotePeriphreralDeviceName) {
        qDebug() << "#### Matching LE peripheral device found";
        leRemotePeripheralDevice = info;
        latestRSSI = QByteArray::number(info.rssi());
        emit leChanged();
    }
}

void BtLocalDevice::deviceUpdated(const QBluetoothDeviceInfo &info,
                   QBluetoothDeviceInfo::Fields updateFields)
{
    if (info.name() == leRemotePeriphreralDeviceName
            && updateFields & QBluetoothDeviceInfo::Field::RSSI) {
        qDebug() << "#### LE peripheral RSSI updated during scan";
        latestRSSI = QByteArray::number(info.rssi());
        emit leChanged();
    }
}

void BtLocalDevice::discoveryFinished()
{
    qDebug() << "###### Device Discovery Finished";
}

void BtLocalDevice::discoveryCanceled()
{
    qDebug() << "###### Device Discovery Canceled";
}

void BtLocalDevice::discoveryError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    auto *client = qobject_cast<QBluetoothDeviceDiscoveryAgent *>(sender());
    if (!client)
        return;
    qDebug() << "###### Device Discovery Error:" << error << (client ? client->errorString() : QString());
}

void BtLocalDevice::startDiscovery()
{
    if (deviceAgent) {
        qDebug() << "###### Starting device discovery process";
        deviceAgent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod);
    }
}

void BtLocalDevice::stopDiscovery()
{
    if (deviceAgent) {
        qDebug() << "Stopping device discovery process";
        deviceAgent->stop();
    }
}

void BtLocalDevice::startServiceDiscovery(bool isMinimalDiscovery)
{
    if (serviceAgent) {
        serviceAgent->setRemoteAddress(QBluetoothAddress());

        qDebug() << "###### Starting service discovery process";
        serviceAgent->start(isMinimalDiscovery
                            ? QBluetoothServiceDiscoveryAgent::MinimalDiscovery
                            : QBluetoothServiceDiscoveryAgent::FullDiscovery);
    }
}

void BtLocalDevice::startTargettedServiceDiscovery()
{
    if (serviceAgent) {
        const QBluetoothAddress baddr(BTCHAT_DEVICE_ADDR);
        qDebug() << "###### Starting service discovery on"
                 << baddr.toString();
        if (baddr.isNull())
            return;

        if (!serviceAgent->setRemoteAddress(baddr)) {
            qWarning() << "###### Cannot set remote address. Aborting";
            return;
        }

        serviceAgent->start();
    }
}

void BtLocalDevice::stopServiceDiscovery()
{
    if (serviceAgent) {
        qDebug() << "Stopping service discovery process";
        serviceAgent->stop();
    }
}

void BtLocalDevice::serviceDiscovered(const QBluetoothServiceInfo &info)
{
    QStringList classIds;
    const QList<QBluetoothUuid> uuids = info.serviceClassUuids();
    for (const QBluetoothUuid &uuid : uuids)
        classIds.append(uuid.toString());
    qDebug() << "$$ Found new service" << info.device().address().toString()
             << info.serviceUuid() << info.serviceName() << info.serviceDescription() << classIds;

    bool matchingService =
            (info.serviceUuid() == QBluetoothUuid(QString(TEST_SERVICE_UUID)));
#ifdef Q_OS_ANDROID
    // QTBUG-61392
    matchingService = matchingService
            || (info.serviceUuid() == QBluetoothUuid(QString(TEST_REVERSE_SERVICE_UUID)));
#endif

    if (matchingService
            || info.serviceClassUuids().contains(QBluetoothUuid(QString(TEST_SERVICE_UUID))))
    {
        //This is here to detect the test server for SPP testing later on
        bool alreadyKnown = false;
        for (const QBluetoothServiceInfo& found : qAsConst(foundTestServers)) {
            if (found.device().address() == info.device().address()) {
                alreadyKnown = true;
                break;
            }
        }

        if (!alreadyKnown) {
            foundTestServers.append(info);
            qDebug() << "@@@@@@@@ Adding:" << info.device().address().toString();
        }
    }
}

void BtLocalDevice::serviceDiscoveryFinished()
{
    qDebug() << "###### Service Discovery Finished";
}

void BtLocalDevice::serviceDiscoveryCanceled()
{
    qDebug() << "###### Service Discovery Canceled";
}

void BtLocalDevice::serviceDiscoveryError(QBluetoothServiceDiscoveryAgent::Error error)
{
    auto *client = qobject_cast<QBluetoothServiceDiscoveryAgent *>(sender());
    if (!client)
        return;
    qDebug() << "###### Service Discovery Error:" << error << (client ? client->errorString() : QString());
}

void BtLocalDevice::dumpServiceDiscovery()
{
    if (deviceAgent) {
        qDebug() << "Device Discovery active:" << deviceAgent->isActive();
        qDebug() << "Error:" << deviceAgent->error() << deviceAgent->errorString();
        const QList<QBluetoothDeviceInfo> list = deviceAgent->discoveredDevices();
        qDebug() << "Discovered Devices:" << list.size();

        for (const QBluetoothDeviceInfo &info : list)
            qDebug() << info.name() << info.address().toString() << info.rssi();
    }
    if (serviceAgent) {
        qDebug() << "Service Discovery active:" << serviceAgent->isActive();
        qDebug() << "Error:" << serviceAgent->error() << serviceAgent->errorString();
        const QList<QBluetoothServiceInfo> list = serviceAgent->discoveredServices();
        qDebug() << "Discovered Services:" << list.size();

        for (const QBluetoothServiceInfo &i : list) {
            qDebug() << i.device().address().toString() << i.device().name() << i.serviceName();
        }

        qDebug() << "###### TestServer offered by:";
        for (const QBluetoothServiceInfo& found : qAsConst(foundTestServers)) {
            qDebug() << found.device().name() << found.device().address().toString();
        }
    }
}

void BtLocalDevice::connectToService()
{
    if (socket) {
        if (socket->preferredSecurityFlags() != securityFlags)
            socket->setPreferredSecurityFlags(securityFlags);
        socket->connectToService(QBluetoothAddress(BTCHAT_DEVICE_ADDR),QBluetoothUuid(QString(TEST_SERVICE_UUID)));
    }
}

void BtLocalDevice::connectToServiceViaSearch()
{
    if (socket) {
        qDebug() << "###### Connecting to service socket";
        if (!foundTestServers.isEmpty()) {
            if (socket->preferredSecurityFlags() != securityFlags)
                socket->setPreferredSecurityFlags(securityFlags);

            QBluetoothServiceInfo info = foundTestServers.at(0);
            socket->connectToService(info);
        } else {
            qWarning() << "Perform search for test service before triggering this function";
        }
    }
}

void BtLocalDevice::disconnectToService()
{
    if (socket) {
        qDebug() << "###### Disconnecting socket";
        socket->disconnectFromService();
    }
}

void BtLocalDevice::closeSocket()
{
    if (socket) {
        qDebug() << "###### Closing socket";
        socket->close();
    }

    if (!serverSockets.isEmpty()) {
        qDebug() << "###### Closing server sockets";
        for (QBluetoothSocket *s : serverSockets)
            s->close();
    }
}

void BtLocalDevice::abortSocket()
{
    if (socket) {
        qDebug() << "###### Disconnecting socket";
        socket->abort();
    }

    if (!serverSockets.isEmpty()) {
        qDebug() << "###### Closing server sockets";
        for (QBluetoothSocket *s : serverSockets)
            s->abort();
    }
}

void BtLocalDevice::socketConnected()
{
    qDebug() << "###### Socket connected";
}

void BtLocalDevice::socketDisconnected()
{
    qDebug() << "###### Socket disconnected";
}

void BtLocalDevice::socketError(QBluetoothSocket::SocketError error)
{
    auto *client = qobject_cast<QBluetoothSocket *>(sender());

    qDebug() << "###### Socket error" << error << (client ? client->errorString() : QString());
}

void BtLocalDevice::socketStateChanged(QBluetoothSocket::SocketState state)
{
    qDebug() << "###### Socket state" << state;
    emit socketStateUpdate(static_cast<int>(state));
}

void BtLocalDevice::dumpSocketInformation()
{
    if (socket) {
        qDebug() << "*******************************";
        qDebug() << "Local info (addr, name, port):" << socket->localAddress().toString()
                 << socket->localName() << socket->localPort();
        qDebug() << "Peer Info (adr, name, port):" << socket->peerAddress().toString()
                 << socket->peerName() << socket->peerPort();
        qDebug() << "socket type:" << socket->socketType();
        qDebug() << "socket state:" << socket->state();
        qDebug() << "socket bytesAvailable()" << socket->bytesAvailable();
        QString tmp;
        switch (socket->error()) {
            case QBluetoothSocket::SocketError::NoSocketError: tmp += "NoSocketError"; break;
            case QBluetoothSocket::SocketError::UnknownSocketError: tmp += "UnknownSocketError"; break;
            case QBluetoothSocket::SocketError::HostNotFoundError: tmp += "HostNotFoundError"; break;
            case QBluetoothSocket::SocketError::ServiceNotFoundError: tmp += "ServiceNotFound"; break;
            case QBluetoothSocket::SocketError::NetworkError: tmp += "NetworkError"; break;
            //case QBluetoothSocket::SocketError::OperationError: tmp+= "OperationError"; break;
            case QBluetoothSocket::SocketError::UnsupportedProtocolError: tmp += "UnsupportedProtocolError"; break;
            case QBluetoothSocket::SocketError::MissingPermissionsError: tmp += "MissingPermissionsError"; break;
            default: tmp+= "Undefined"; break;
        }

        qDebug() << "socket error:" << tmp << socket->errorString();
    } else {
        qDebug() << "No valid socket existing";
    }
}

void BtLocalDevice::writeData()
{
    const char * testData = "ABCABC\n";
    if (socket && socket->state() == QBluetoothSocket::SocketState::ConnectedState) {
        socket->write(testData);
    }
    for (QBluetoothSocket* client : serverSockets) {
        client->write(testData);
    }
}

void BtLocalDevice::readData()
{
    if (socket) {
        while (socket->canReadLine()) {
            QByteArray line = socket->readLine().trimmed();
            qDebug() << ">> peer(" << socket->peerName() << socket->peerAddress()
                     << socket->peerPort() << ") local("
                     << socket->localName() << socket->localAddress() << socket->localPort()
                     << ")>>" << QString::fromUtf8(line.constData(), line.size());
        }
    }
}

void BtLocalDevice::serverError(QBluetoothServer::Error error)
{
    qDebug() << "###### Server socket error" << error;
}

void BtLocalDevice::serverListenPort()
{
    if (server && localDevice) {
        if (server->isListening() || serviceInfo.isRegistered()) {
            qDebug() << "###### Already listening" << serviceInfo.isRegistered();
            return;
        }

        if (server->securityFlags() != securityFlags) {
            qDebug() << "###### Setting security policy on server socket" << securityFlags;
            server->setSecurityFlags(securityFlags);
        }

        qDebug() << "###### Start listening via port";
        bool ret = server->listen(localDevice->address());
        qDebug() << "###### Listening(Expecting TRUE):" << ret;

        if (!ret)
            return;

        QBluetoothServiceInfo::Sequence profileSequence;
        QBluetoothServiceInfo::Sequence classId;
        classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));
        classId << QVariant::fromValue(quint16(0x100));
        profileSequence.append(QVariant::fromValue(classId));
        serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList,
                                 profileSequence);

        classId.clear();
        classId << QVariant::fromValue(QBluetoothUuid(QString(TEST_SERVICE_UUID)));
        classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));
        serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);

        // Service name, description and provider
        serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, tr("Bt Chat Server"));
        serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription,
                                 tr("Example bluetooth chat server"));
        serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider, tr("qt-project.org"));

        // Service UUID set
        serviceInfo.setServiceUuid(QBluetoothUuid(QString(TEST_SERVICE_UUID)));


        // Service Discoverability
        QBluetoothServiceInfo::Sequence browseSequence;
        browseSequence << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::PublicBrowseGroup));
        serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList, browseSequence);

        // Protocol descriptor list
        QBluetoothServiceInfo::Sequence protocolDescriptorList;
        QBluetoothServiceInfo::Sequence protocol;
        protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::L2cap));
        if (server->serverType() == QBluetoothServiceInfo::L2capProtocol)
            protocol << QVariant::fromValue(server->serverPort());
        protocolDescriptorList.append(QVariant::fromValue(protocol));

        if (server->serverType() == QBluetoothServiceInfo::RfcommProtocol) {
            protocol.clear();
            protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::Rfcomm))
                     << QVariant::fromValue(quint8(server->serverPort()));
            protocolDescriptorList.append(QVariant::fromValue(protocol));
        }
        serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                                 protocolDescriptorList);

        //Register service
        qDebug() << "###### Registering service on" << localDevice->address().toString() << server->serverPort();
        bool result = serviceInfo.registerService(localDevice->address());
        if (!result) {
            server->close();
            qDebug() << "###### Reverting registration due to SDP failure.";
        }
    }

}

void BtLocalDevice::serverListenUuid()
{
    if (server) {
        if (server->isListening() || serviceInfo.isRegistered()) {
            qDebug() << "###### Already listening" << serviceInfo.isRegistered();
            return;
        }

        if (server->securityFlags() != securityFlags) {
            qDebug() << "###### Setting security policy on server socket" << securityFlags;
            server->setSecurityFlags(securityFlags);
        }

        qDebug() << "###### Start listening via UUID";
        serviceInfo = server->listen(QBluetoothUuid(QString(TEST_SERVICE_UUID)), tr("Bt Chat Server"));
        qDebug() << "###### Listening(Expecting TRUE, TRUE):" << serviceInfo.isRegistered() << serviceInfo.isValid();
    }
}

void BtLocalDevice::serverClose()
{
    if (server) {
        qDebug() << "###### Closing Server socket";
        if (serviceInfo.isRegistered())
            serviceInfo.unregisterService();
        server->close();
    }
}

void BtLocalDevice::serverNewConnection()
{
    qDebug() << "###### New incoming server connection, pending:" << server->hasPendingConnections();
    if (!server->hasPendingConnections()) {
        qDebug() << "FAIL: expected pending server connection";
        return;
    }
    QBluetoothSocket *client = server->nextPendingConnection();
    if (!client) {
        qDebug() << "FAIL: Cannot obtain pending server connection";
        return;
    }

    client->setParent(this);
    connect(client, &QBluetoothSocket::disconnected, this, &BtLocalDevice::clientSocketDisconnected);
    connect(client, &QIODevice::readyRead, this, &BtLocalDevice::clientSocketReadyRead);
    connect(client, &QBluetoothSocket::stateChanged,
            this, &BtLocalDevice::socketStateChanged);
    connect(client, &QBluetoothSocket::errorOccurred, this, &BtLocalDevice::socketError);
    connect(client, &QBluetoothSocket::connected, this, &BtLocalDevice::socketConnected);
    connect(client, &QBluetoothSocket::bytesWritten, this, [](qint64 bytesWritten){
        qDebug() << "Bytes Written to Server socket:" << bytesWritten;
    });
    serverSockets.append(client);
}

void BtLocalDevice::clientSocketDisconnected()
{
    auto *client = qobject_cast<QBluetoothSocket *>(sender());
    if (!client)
        return;

    qDebug() << "######" << "Removing server socket connection";

    serverSockets.removeOne(client);
    client->deleteLater();
}


void BtLocalDevice::clientSocketReadyRead()
{
    auto *socket = qobject_cast<QBluetoothSocket *>(sender());
    if (!socket)
        return;

    while (socket->canReadLine()) {
        const QByteArray line = socket->readLine().trimmed();
        QString lineString = QString::fromUtf8(line.constData(), line.size());
        qDebug() <<  ">>(" << server->serverAddress() << server->serverPort()  <<")>>"
                 << lineString;

        //when using the tst_QBluetoothSocket we echo received text back
        //Any line starting with "Echo:" will be echoed
        if (lineString.startsWith(QStringLiteral("Echo:"))) {
            qDebug() << "Assuming tst_qbluetoothsocket as client. Echoing back.";
            lineString += QLatin1Char('\n');
            socket->write(lineString.toUtf8());
        }
    }
}


void BtLocalDevice::dumpServerInformation()
{
    static QBluetooth::SecurityFlags secFlag = QBluetooth::Security::Authentication;
    if (server) {
        qDebug() << "*******************************";
        qDebug() << "server port:" <<server->serverPort()
                 << "type:" << server->serverType()
                 << "address:" << server->serverAddress().toString();
        qDebug() << "error:" << server->error();
        qDebug() << "listening:" << server->isListening()
                 << "hasPending:" << server->hasPendingConnections()
                 << "maxPending:" << server->maxPendingConnections();
        qDebug() << "security:" << server->securityFlags() << "Togling security flag";
        if (secFlag == QBluetooth::SecurityFlags(QBluetooth::Security::Authentication))
            secFlag = QBluetooth::Security::Encryption;
        else
            secFlag = QBluetooth::Security::Authentication;

        //server->setSecurityFlags(secFlag);

        for (const QBluetoothSocket *client : qAsConst(serverSockets)) {
            qDebug() << "##" << client->localAddress().toString()
                     << client->localName() << client->localPort();
            qDebug() << "##" << client->peerAddress().toString()
                     << client->peerName() << client->peerPort();
            qDebug() << client->socketType() << client->state();
            qDebug() << "Pending bytes: " << client->bytesAvailable();
            QString tmp;
            switch (client->error()) {
            case QBluetoothSocket::SocketError::NoSocketError: tmp += "NoSocketError"; break;
            case QBluetoothSocket::SocketError::UnknownSocketError: tmp += "UnknownSocketError"; break;
            case QBluetoothSocket::SocketError::HostNotFoundError: tmp += "HostNotFoundError"; break;
            case QBluetoothSocket::SocketError::ServiceNotFoundError: tmp += "ServiceNotFound"; break;
            case QBluetoothSocket::SocketError::NetworkError: tmp += "NetworkError"; break;
            case QBluetoothSocket::SocketError::UnsupportedProtocolError: tmp += "UnsupportedProtocolError"; break;
            //case QBluetoothSocket::SocketError::OperationError: tmp+= "OperationError"; break;
            case QBluetoothSocket::SocketError::MissingPermissionsError: tmp += "MissingPermissionsError"; break;
            default: tmp += QString::number(static_cast<int>(client->error())); break;
            }

            qDebug() << "socket error:" << tmp << client->errorString();
        }
    }
}

template <typename T>
void printError(const QLatin1StringView name, T* ptr)
{
    if (!ptr)
        return;
    qDebug() << name << "error:" << ptr->error();
}

void BtLocalDevice::dumpErrors()
{
    qDebug() << "###### Errors";
    printError("Device agent"_L1, deviceAgent);
    printError("Service agent"_L1, serviceAgent);
    printError("LE Central"_L1, leCentralController.get());
    printError("LE Central Service"_L1, leCentralService.get());
    printError("LE Peripheral"_L1, lePeripheralController.get());
    printError("LE Peripheral Service"_L1, lePeripheralService.get());
    printError("Socket"_L1, socket);
    printError("Server"_L1, server);
}

void BtLocalDevice::dumpInformation()
{
    if (!localDevice)
        return;
    qDebug() << "###### default local device";
    dumpLocalDevice(localDevice);
    const QList<QBluetoothHostInfo> list = QBluetoothLocalDevice::allDevices();
    qDebug() << "Found local devices: "  << list.size();
    for (const QBluetoothHostInfo &info : list) {
        qDebug() << "    " << info.address().toString() << " " <<info.name();
    }

    QBluetoothAddress address(QStringLiteral("11:22:33:44:55:66"));
    QBluetoothLocalDevice temp(address);
    qDebug() << "###### 11:22:33:44:55:66 address valid:" << !address.isNull();
    dumpLocalDevice(&temp);

    QBluetoothAddress address2;
    QBluetoothLocalDevice temp2(address2);
    qDebug() << "###### 00:00:00:00:00:00 address valid:" << !address2.isNull();
    dumpLocalDevice(&temp2);

    const QBluetoothAddress BB(BTCHAT_DEVICE_ADDR);
    qDebug() << "###### Bonding state with" <<  QString(BTCHAT_DEVICE_ADDR) << ":" << localDevice->pairingStatus(BB);
    qDebug() << "###### Bonding state with" << address2.toString() << ": " << localDevice->pairingStatus(address2);
    qDebug() << "###### Bonding state with" << address.toString() << ": " << localDevice->pairingStatus(address);

    qDebug() << "###### Connected Devices";
    const QList<QBluetoothAddress> connectedDevices = localDevice->connectedDevices();
    for (const QBluetoothAddress &addr : connectedDevices)
        qDebug() << "    " << addr.toString();

    qDebug() << "###### Discovered Devices";
    if (deviceAgent) {
        const QList<QBluetoothDeviceInfo> devices = deviceAgent->discoveredDevices();
        for (const QBluetoothDeviceInfo &info : devices) {
            deviceDiscovered(info);
        }
    }

    QBluetoothDeviceDiscoveryAgent invalidAgent(QBluetoothAddress("11:22:33:44:55:66"));
    invalidAgent.start();
    qDebug() << "######" << "Testing device discovery agent constructor with invalid address";
    qDebug() << "######" << (invalidAgent.error() == QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError)
                         << "(Expected: true)";
    QBluetoothDeviceDiscoveryAgent validAgent(localDevice->address());
    validAgent.start();
    qDebug() << "######" << (validAgent.error() == QBluetoothDeviceDiscoveryAgent::NoError) << "(Expected: true)";

    QBluetoothServiceDiscoveryAgent invalidSAgent(QBluetoothAddress("11:22:33:44:55:66"));
    invalidSAgent.start();
    qDebug() << "######" << "Testing service discovery agent constructor with invalid address";
    qDebug() << "######" << (invalidSAgent.error() == QBluetoothServiceDiscoveryAgent::InvalidBluetoothAdapterError)
                         << "(Expected: true)";
    QBluetoothServiceDiscoveryAgent validSAgent(localDevice->address());
    validSAgent.start();
    qDebug() << "######" << (validSAgent.error() == QBluetoothServiceDiscoveryAgent::NoError) << "(Expected: true)";

    dumpLeInfo();
}

void BtLocalDevice::powerOn()
{
    if (!localDevice)
        return;
    qDebug() << "Powering on";
    localDevice->powerOn();
}

void BtLocalDevice::reset()
{
    emit errorOccurred(static_cast<QBluetoothLocalDevice::Error>(1000));
    if (serviceAgent) {
        serviceAgent->clear();
    }
    foundTestServers.clear();
}

void BtLocalDevice::dumpLocalDevice(QBluetoothLocalDevice *dev)
{
    qDebug() << "    Valid: " << dev->isValid();
    qDebug() << "    Name" << dev->name();
    qDebug() << "    Address" << dev->address().toString();
    qDebug() << "    HostMode" << dev->hostMode();
}

void BtLocalDevice::peripheralCreate()
{
    qDebug() << "######" << "LE create peripheral";
    if (lePeripheralController) {
        qDebug() << "Peripheral already existed";
        return;
    }

    lePeripheralController.reset(QLowEnergyController::createPeripheral());
    emit leChanged();

    QObject::connect(lePeripheralController.get(), &QLowEnergyController::errorOccurred,
                     [this](QLowEnergyController::Error error) {
        qDebug() << "QLowEnergyController peripheral errorOccurred:" << error;
        emit leChanged();
    });
    QObject::connect(lePeripheralController.get(), &QLowEnergyController::stateChanged,
                     [this](QLowEnergyController::ControllerState state) {
        qDebug() << "QLowEnergyController peripheral stateChanged:" << state;
        emit leChanged();
    });
}

void BtLocalDevice::peripheralStartAdvertising()
{
    qDebug() << "######" << "LE start advertising";
    if (!lePeripheralController) {
        qDebug() << "Create peripheral first";
        return;
    }

    if (!leServiceData.isValid()) {
        // Create service data if this is the first advertisement
        leServiceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        leServiceData.setUuid(QBluetoothUuid(leServiceUuid));

        QLowEnergyCharacteristicData charData;
        charData.setUuid(QBluetoothUuid(leCharacteristicUuid));
        charData.setValue(leCharacteristicValue);
        charData.setValueLength(leCharacteristicSize, leCharacteristicSize);
        charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read
                               | QLowEnergyCharacteristic::PropertyType::Write
                               | QLowEnergyCharacteristic::PropertyType::Notify);

        const QLowEnergyDescriptorData clientConfig(
                QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                QLowEnergyCharacteristic::CCCDDisable);
        charData.addDescriptor(clientConfig);
        leServiceData.addCharacteristic(charData);

        // Create advertisement data
        leAdvertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
        leAdvertisingData.setIncludePowerLevel(true);
        leAdvertisingData.setLocalName(leRemotePeriphreralDeviceName);
        // Results in too big advertisement data, can be useful for testing such scenario
        // leAdvertisingData.setServices(QList{leServiceData.uuid()});
    }

    // Add/create the service
    lePeripheralService.reset(lePeripheralController->addService(leServiceData));
    emit leChanged();
    if (!lePeripheralService) {
        qDebug() << "Peripheral service creation failed";
        return;
    }
    QObject::connect(lePeripheralService.get(), &QLowEnergyService::characteristicWritten,
                     [](const QLowEnergyCharacteristic&, const QByteArray& value){
        qDebug() << "LE peripheral service characteristic value written" << value;
    });
    QObject::connect(lePeripheralService.get(), &QLowEnergyService::characteristicRead,
                     [](const QLowEnergyCharacteristic&, const QByteArray& value){
        qDebug() << "LE peripheral service characteristic value read" << value;
    });
    QObject::connect(lePeripheralService.get(), &QLowEnergyService::characteristicChanged,
                     [](const QLowEnergyCharacteristic&, const QByteArray& value){
        qDebug() << "LE peripheral service characteristic value changed" << value;
    });
    QObject::connect(lePeripheralService.get(), &QLowEnergyService::errorOccurred,
                     [this](QLowEnergyService::ServiceError error){
        qDebug() << "LE peripheral service errorOccurred:" << error;
        emit leChanged();
    });
    QObject::connect(lePeripheralService.get(), &QLowEnergyService::stateChanged,
                     [this](QLowEnergyService::ServiceState state){
        qDebug() << "LE peripheral service state changed:" << state;
        emit leChanged();
    });

    // Start advertising
    lePeripheralController->startAdvertising(QLowEnergyAdvertisingParameters{},
                                             leAdvertisingData, leAdvertisingData);
}

void BtLocalDevice::peripheralStopAdvertising()
{
    qDebug() << "######" << "LE stop advertising";
    if (!lePeripheralController) {
        qDebug() << "Peripheral does not exist";
        return;
    }
    lePeripheralController->stopAdvertising();
}

void BtLocalDevice::centralWrite()
{
    qDebug() << "######" << "LE central write";
    if (!leCentralController || !leCentralService) {
        qDebug() << "Central or central service does not exist";
        return;
    }
    // Update value at the beginning and end so we can check whole data is sent in large writes.
    // Value is offset'd with 100 to easily see which end did the write when testing
    leValueUpdate += 1;
    leCharacteristicValue[0] = leValueUpdate + 100;
    leCharacteristicValue[leCharacteristicSize - 1] = leValueUpdate + 100;
    auto characteristic = leCentralService->characteristic(QBluetoothUuid(leCharacteristicUuid));
    if (characteristic.isValid()) {
        qDebug() << "    Central writes value:" << leCharacteristicValue;
        leCentralService->writeCharacteristic(characteristic, leCharacteristicValue);
    } else {
        qDebug() << "Characteristic was invalid";
    }
}

void BtLocalDevice::centralRead()
{
    qDebug() << "######" << "LE central read";
    if (!leCentralController || !leCentralService) {
        qDebug() << "Central or central service does not exist";
        return;
    }
    auto characteristic = leCentralService->characteristic(QBluetoothUuid(leCharacteristicUuid));
    if (characteristic.isValid()) {
        qDebug() << "    Value before issuing read():" << characteristic.value();
        leCentralService->readCharacteristic(characteristic);
    } else {
        qDebug() << "Characteristic was invalid";
    }
}

void BtLocalDevice::peripheralWrite()
{
    qDebug() << "######" << "LE peripheral write";
    if (!lePeripheralController || !lePeripheralService) {
        qDebug() << "Peripheral or peripheral service does not exist";
        return;
    }
    // Update value at the beginning and end so we can check whole data is sent in large writes
    leCharacteristicValue[0] = ++leValueUpdate;
    leCharacteristicValue[leCharacteristicSize - 1] = leValueUpdate;
    auto characteristic = lePeripheralService->characteristic(QBluetoothUuid(leCharacteristicUuid));
    if (characteristic.isValid()) {
        qDebug() << "    Peripheral writes value:" << leCharacteristicValue;
        lePeripheralService->writeCharacteristic(characteristic, leCharacteristicValue);
    } else {
        qDebug() << "Characteristic was invalid";
    }
}

void BtLocalDevice::peripheralRead()
{
    qDebug() << "######" << "LE peripheral read";
    if (!lePeripheralController || !lePeripheralService) {
        qDebug() << "Peripheral or peripheral service does not exist";
        return;
    }
    auto characteristic = lePeripheralService->characteristic(QBluetoothUuid(leCharacteristicUuid));
    if (characteristic.isValid())
        qDebug() << "    Value:" << characteristic.value();
    else
        qDebug() << "Characteristic was invalid";
}

void BtLocalDevice::startLeDeviceDiscovery()
{
    qDebug() << "######" << "LE device discovery start for:" << leRemotePeriphreralDeviceName;
    if (deviceAgent)
        deviceAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void BtLocalDevice::centralStartServiceDiscovery()
{
    qDebug() << "######" << "LE service discovery start";
    if (!leCentralController) {
        qDebug() << "Create and connect central first";
        return;
    }
    leCentralController->discoverServices();
}

void BtLocalDevice::centralCreate()
{
    qDebug() << "######" << "Central create";
    if (leCentralController) {
        qDebug() << "Central already existed";
        return;
    }

    if (!leRemotePeripheralDevice.isValid()) {
        qDebug() << "Creation failed, needs successful LE device discovery first";
        return;
    }

    if (deviceAgent && deviceAgent->isActive()) {
        qDebug() << "###### Stopping device discovery agent";
        deviceAgent->stop();
    }

    leCentralController.reset(QLowEnergyController::createCentral(leRemotePeripheralDevice));
    emit leChanged();

    if (!leCentralController) {
        qDebug() << "LE Central creation failed";
        return;
    }

    QObject::connect(leCentralController.get(), &QLowEnergyController::errorOccurred,
                     [](QLowEnergyController::Error error) {
        qDebug() << "QLowEnergyController central errorOccurred:" << error;
    });
    QObject::connect(leCentralController.get(), &QLowEnergyController::stateChanged,
                     [this](QLowEnergyController::ControllerState state) {
        qDebug() << "QLowEnergyController central stateChanged:" << state;
        if (state == QLowEnergyController::UnconnectedState)
            latestRSSI = "N/A"_ba;
        emit leChanged();
    });
    QObject::connect(leCentralController.get(), &QLowEnergyController::rssiRead,
                     [this](qint16 rssi) {
        qDebug() << "QLowEnergyController central RSSI updated:" << rssi;
        latestRSSI = QByteArray::number(rssi);
        emit leChanged();
    });
}

void BtLocalDevice::centralDiscoverServiceDetails()
{
     qDebug() << "###### Discover Service details";
     if (!leCentralController) {
         qDebug() << "Central does not exist";
         return;
     }
     leCentralService.reset(
                 leCentralController->createServiceObject(QBluetoothUuid(leServiceUuid)));
     emit leChanged();
     if (!leCentralService) {
         qDebug() << "Service creation failed, cannot discover details";
         return;
     }
     QObject::connect(leCentralService.get(), &QLowEnergyService::stateChanged,
                      [this](QLowEnergyService::ServiceState state){
         qDebug() << "LE central service state changed:" << state;
         emit leChanged();
     });
     QObject::connect(leCentralService.get(), &QLowEnergyService::characteristicWritten,
                      [](const QLowEnergyCharacteristic&, const QByteArray& value){
         qDebug() << "LE central service characteristic value written" << value;
     });
     QObject::connect(leCentralService.get(), &QLowEnergyService::characteristicRead,
                      [](const QLowEnergyCharacteristic&, const QByteArray& value){
         qDebug() << "LE central service characteristic value read" << value;
     });
     QObject::connect(leCentralService.get(), &QLowEnergyService::characteristicChanged,
                      [](const QLowEnergyCharacteristic&, const QByteArray& value){
         qDebug() << "LE central service characteristic value changed" << value;
     });
     QObject::connect(leCentralService.get(), &QLowEnergyService::descriptorWritten,
                      [this](const QLowEnergyDescriptor&, const QByteArray& value){
         qDebug() << "LE central service descriptor value written" << value;
         emit leChanged();
     });
     QObject::connect(leCentralService.get(), &QLowEnergyService::errorOccurred,
                      [](QLowEnergyService::ServiceError error){
         qDebug() << "LE central service error occurred:" << error;
     });
     leCentralService->discoverDetails(QLowEnergyService::FullDiscovery);
}

void BtLocalDevice::centralConnect()
{
    qDebug() << "######" <<  "Central connect";
    if (!leCentralController) {
        qDebug() << "Create central first";
        return;
    }
    leCentralController->connectToDevice();
}

void BtLocalDevice::dumpLeInfo()
{
    const auto controllerDump = [](QLowEnergyController* controller) {
        qDebug() << "    State:" << controller->state();
        qDebug() << "    Role:" << controller->role();
        qDebug() << "    Error:" << controller->error();
        qDebug() << "    ErrorString:" << controller->errorString();
        qDebug() << "    MTU:" << controller->mtu();
        qDebug() << "    Local Address:" << controller->localAddress();
        qDebug() << "    RemoteAddress:" << controller->remoteAddress();
        qDebug() << "    RemoteName:" << controller->remoteName();
        qDebug() << "    Services count:" << controller->services().size();
    };
    qDebug() << "######" << "LE Peripheral controller";
    if (lePeripheralController)
        controllerDump(lePeripheralController.get());

    qDebug() << "######" << "LE Central controller";
    if (leCentralController)
        controllerDump(leCentralController.get());

    qDebug() << "######" << "LE Found peripheral device";
    if (leRemotePeripheralDevice.isValid()) {
        qDebug() << "    Name:" << leRemotePeripheralDevice.name();
        qDebug() << "    UUID:" << leRemotePeripheralDevice.deviceUuid();
        qDebug() << "    Address:" << leRemotePeripheralDevice.address();
    }

    const auto serviceDump = [](QLowEnergyService* service){
        qDebug() << "    Name:" << service->serviceName();
        qDebug() << "    Uuid:" << service->serviceUuid();
        qDebug() << "    Error:" << service->error();
        auto characteristics = service->characteristics();
        for (const auto& characteristic : characteristics) {
            qDebug() << "    Characteristic";
            qDebug() << "        Uuid" << characteristic.uuid();
            qDebug() << "        Value" << characteristic.value();

        }
    };

    qDebug() << "######" << "LE Central-side service";
    if (leCentralService)
        serviceDump(leCentralService.get());

    qDebug() << "######" << "LE Peripheral-side service";
    if (lePeripheralService)
        serviceDump(lePeripheralService.get());
}

void BtLocalDevice::centralSubscribeUnsubscribe()
{
    qDebug() << "######" << "LE Central (Un)Subscribe";
    if (!leCentralService) {
        qDebug() << "Service object does not exist";
        return;
    }
    auto characteristic = leCentralService->characteristic(QBluetoothUuid(leCharacteristicUuid));
    if (!characteristic.isValid()) {
        qDebug() << "Characteristic is not valid";
        return;
    }

    auto descriptor = characteristic.descriptor(
                QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
    if (!descriptor.isValid()) {
        qDebug() << "Descriptor is not valid";
        return;
    }
    if (descriptor.value() == QByteArray::fromHex("0000")) {
        qDebug() << "    Subscribing notifications";
        leCentralService->writeDescriptor(descriptor, QByteArray::fromHex("0100"));
    } else {
        qDebug() << "    Unsubscribing notifications";
        leCentralService->writeDescriptor(descriptor, QByteArray::fromHex("0000"));
    }
    emit leChanged();
}

void BtLocalDevice::centralDelete()
{
    qDebug() << "######" << "Delete central" << leCentralController.get();
    leCentralController.reset(nullptr);
    latestRSSI = "(N/A)"_ba;
    emit leChanged();
}

void BtLocalDevice::peripheralDelete()
{
    qDebug() << "######" << "Delete peripheral" << lePeripheralController.get();
    lePeripheralController.reset(nullptr);
    emit leChanged();
}

bool BtLocalDevice::centralExists() const
{
    return leCentralController.get();
}

bool BtLocalDevice::centralSubscribed() const
{
    if (!leCentralService)
        return false;

    auto characteristic = leCentralService->characteristic(QBluetoothUuid(leCharacteristicUuid));
    if (!characteristic.isValid())
        return false;

    auto descriptor = characteristic.descriptor(
                QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
    if (!descriptor.isValid())
        return false;

    return (descriptor.value() != QByteArray::fromHex("0000"));
}

QByteArray BtLocalDevice::centralState() const
{
    if (!leCentralController)
        return "(N/A)"_ba;

    return controllerStateString[leCentralController->state()];
}

QByteArray BtLocalDevice::centralServiceState() const
{
    if (!leCentralService)
        return "(N/A)"_ba;

    return serviceStateString[leCentralService->state()];
}

QByteArray BtLocalDevice::centralError() const
{
    if (!leCentralController)
        return "(N/A)"_ba;

    return controllerErrorString[leCentralController->error()];
}

QByteArray BtLocalDevice::centralServiceError() const
{
    if (!leCentralService)
        return "(N/A)"_ba;

    return serviceErrorString[leCentralService->error()];
}

void BtLocalDevice::centralReadRSSI() const
{
    qDebug() << "######" << "LE central readRSSI";
    if (!leCentralController)
        return;
    leCentralController->readRssi();
}

QByteArray BtLocalDevice::centralRSSI() const
{
    return latestRSSI;
}

QByteArray BtLocalDevice::peripheralState() const
{
    if (!lePeripheralController)
        return "(N/A)"_ba;

    return controllerStateString[lePeripheralController->state()];
}

QByteArray BtLocalDevice::peripheralServiceState() const
{
    if (!lePeripheralService)
        return "(N/A)"_ba;

    return serviceStateString[lePeripheralService->state()];
}

QByteArray BtLocalDevice::peripheralError() const
{
    if (!lePeripheralController)
        return "(N/A)"_ba;

    return controllerErrorString[lePeripheralController->error()];
}

QByteArray BtLocalDevice::peripheralServiceError() const
{
    if (!lePeripheralService)
        return "(N/A)"_ba;

    return serviceErrorString[lePeripheralService->error()];
}

bool BtLocalDevice::peripheralExists() const
{
    return lePeripheralController.get();
}

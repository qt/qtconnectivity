// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "btlocaldevice.h"
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QtBluetooth/QBluetoothServiceInfo>

#define BTCHAT_DEVICE_ADDR "00:15:83:38:17:C3"

//same uuid as examples/bluetooth/btchat
//the reverse UUID is only used on Android to counter
//https://issuetracker.google.com/issues/37076498 (tracked via QTBUG-61392)
#define TEST_SERVICE_UUID "e8e10f95-1a70-4b27-9ccf-02010264e9c8"
#define TEST_REVERSE_SERVICE_UUID "c8e96402-0102-cf9c-274b-701a950fe1e8"

#define SOCKET_PROTOCOL QBluetoothServiceInfo::RfcommProtocol
//#define SOCKET_PROTOCOL QBluetoothServiceInfo::L2capProtocol

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

    return QStringLiteral("HostMode: <None>");
}

void BtLocalDevice::setHostMode(int newMode)
{
    localDevice->setHostMode(static_cast<QBluetoothLocalDevice::HostMode>(newMode));
}

void BtLocalDevice::requestPairingUpdate(bool isPairing)
{
    QBluetoothAddress baddr(BTCHAT_DEVICE_ADDR);
    if (baddr.isNull())
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
        for (const QBluetoothServiceInfo& found : std::as_const(foundTestServers)) {
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
        for (const QBluetoothServiceInfo& found : std::as_const(foundTestServers)) {
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

        for (const QBluetoothSocket *client : std::as_const(serverSockets)) {
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

void BtLocalDevice::dumpInformation()
{
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
}

void BtLocalDevice::powerOn()
{
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

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

#include <QtTest/QtTest>

#include <QDebug>

#include <QProcessEnvironment>

#include <qbluetoothsocket.h>
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothserviceinfo.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothlocaldevice.h>

#include <btclient.h>

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QBluetoothSocket::SocketState)
Q_DECLARE_METATYPE(QBluetoothServiceInfo::Protocol)

//#define BTADDRESS "00:1A:9F:92:9E:5A"
char BTADDRESS[] = "00:00:00:00:00:00";

// Max time to wait for connection

static const int MaxConnectTime = 60 * 1000;   // 1 minute in ms
static const int MaxReadWriteTime = 60 * 1000; // 1 minute in ms

class tst_QBluetoothSocket : public QObject
{
    Q_OBJECT

public:
    enum ClientConnectionShutdown {
        Error,
        Disconnect,
        Close,
        Abort,
    };

    tst_QBluetoothSocket();
    ~tst_QBluetoothSocket();

private slots:
    void initTestCase();

    void tst_construction_data();
    void tst_construction();

    void tst_clientConnection_data();
    void tst_clientConnection();

    void tst_serviceConnection_data();
    void tst_serviceConnection();

    void tst_clientCommunication_data();
    void tst_clientCommunication();

    void tst_localPeer_data();
    void tst_localPeer();

    void tst_error();

public slots:
    void serviceDiscovered(const QBluetoothServiceInfo &info);
    void finished();
    void error(QBluetoothServiceDiscoveryAgent::Error error);
private:
    bool done_discovery;

};

Q_DECLARE_METATYPE(tst_QBluetoothSocket::ClientConnectionShutdown)

tst_QBluetoothSocket::tst_QBluetoothSocket()
{
    qRegisterMetaType<QBluetoothSocket::SocketState>("QBluetoothSocket::SocketState");
    qRegisterMetaType<QBluetoothSocket::SocketError>("QBluetoothSocket::SocketError");
}

tst_QBluetoothSocket::~tst_QBluetoothSocket()
{
}

void tst_QBluetoothSocket::initTestCase()
{
    // start Bluetooth if not started
    if (!QBluetoothLocalDevice::allDevices().count())
        QSKIP("Skipping test due to missing Bluetooth device");

    QBluetoothLocalDevice *device = new QBluetoothLocalDevice();
    device->powerOn();
    delete device;

    QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
    QLatin1String t("TESTSERVER");
    if (pe.contains(t)){
        qDebug() << pe.value(t);
        strcpy(BTADDRESS, pe.value(t).toLatin1());
    }

    if (QBluetoothAddress(BTADDRESS).isNull()){
        // Go find an echo server for BTADDRESS
        QBluetoothServiceDiscoveryAgent *sda = new QBluetoothServiceDiscoveryAgent(this);
        connect(sda, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)), this, SLOT(serviceDiscovered(QBluetoothServiceInfo)));
        connect(sda, SIGNAL(error(QBluetoothServiceDiscoveryAgent::Error)), this, SLOT(error(QBluetoothServiceDiscoveryAgent::Error)));
        connect(sda, SIGNAL(finished()), this, SLOT(finished()));


        qDebug() << "Starting discovery";
        done_discovery = false;
        memset(BTADDRESS, 0, 18);

        sda->setUuidFilter(QBluetoothUuid(QString(ECHO_SERVICE_UUID)));
        sda->start(QBluetoothServiceDiscoveryAgent::MinimalDiscovery);

        for (int connectTime = MaxConnectTime; !done_discovery && connectTime > 0; connectTime -= 1000)
            QTest::qWait(1000);

        sda->stop();

        if (QBluetoothAddress(BTADDRESS).isNull()){
            QFAIL("Unable to find test service");
        }
        delete sda;
        sda = 0x0;
    }

}

void tst_QBluetoothSocket::error(QBluetoothServiceDiscoveryAgent::Error error)
{
    qDebug() << "Received error" << error;
//    done_discovery = true;
}

void tst_QBluetoothSocket::finished()
{
    qDebug() << "Finished";
    done_discovery = true;
}

void tst_QBluetoothSocket::serviceDiscovered(const QBluetoothServiceInfo &info)
{
    qDebug() << "Found: " << info.device().name() << info.serviceUuid();
    strcpy(BTADDRESS, info.device().address().toString().toLatin1());
    done_discovery = true;
}

void tst_QBluetoothSocket::tst_construction_data()
{
    QTest::addColumn<QBluetoothServiceInfo::Protocol>("socketType");

    QTest::newRow("unknown protocol") << QBluetoothServiceInfo::UnknownProtocol;
    QTest::newRow("rfcomm socket") << QBluetoothServiceInfo::RfcommProtocol;
    QTest::newRow("l2cap socket") << QBluetoothServiceInfo::L2capProtocol;
}

void tst_QBluetoothSocket::tst_construction()
{
    QFETCH(QBluetoothServiceInfo::Protocol, socketType);

    {
        QBluetoothSocket socket;

        QCOMPARE(socket.socketType(), QBluetoothServiceInfo::UnknownProtocol);
    }

    {
        QBluetoothSocket socket(socketType);

        QCOMPARE(socket.socketType(), socketType);
    }
}

void tst_QBluetoothSocket::tst_clientConnection_data()
{
    QTest::addColumn<QBluetoothServiceInfo::Protocol>("socketprotocol");
    QTest::addColumn<ClientConnectionShutdown>("shutdown");
    QTest::addColumn<QBluetoothAddress>("address");
    QTest::addColumn<quint16>("port");
    QTest::addColumn<QByteArray>("data");

    //QBluetoothAddress address("00:1E:3A:81:BA:69");
    QBluetoothAddress address(BTADDRESS);
    quint16 port = 10;

    QTest::newRow("unavailable, error") << QBluetoothServiceInfo::RfcommProtocol
                                        << Error << QBluetoothAddress("112233445566") << quint16(10) << QByteArray();

    QTest::newRow("available, disconnect") << QBluetoothServiceInfo::RfcommProtocol
                                           << Disconnect << address << port << QByteArray();
    QTest::newRow("available, disconnect with data") << QBluetoothServiceInfo::RfcommProtocol
                                                     << Disconnect << address << port << QByteArray("Test message\n");
    QTest::newRow("available, close") << QBluetoothServiceInfo::RfcommProtocol
                                      << Close << address << port << QByteArray();
    QTest::newRow("available, abort") << QBluetoothServiceInfo::RfcommProtocol
                                      << Abort << address << port << QByteArray();
    QTest::newRow("available, abort with data") << QBluetoothServiceInfo::RfcommProtocol
                                                << Abort << address << port << QByteArray("Test message\n");


    port = 0x1011;
    QTest::newRow("unavailable, error") << QBluetoothServiceInfo::L2capProtocol
                                        << Error << QBluetoothAddress("112233445566") << quint16(10) << QByteArray();

    QTest::newRow("available, disconnect") << QBluetoothServiceInfo::L2capProtocol
                                           << Disconnect << address << port << QByteArray();
    QTest::newRow("available, disconnect with data") << QBluetoothServiceInfo::L2capProtocol
                                                     << Disconnect << address << port << QByteArray("Test message\n");
    QTest::newRow("available, close") << QBluetoothServiceInfo::L2capProtocol
                                      << Close << address << port << QByteArray();
    QTest::newRow("available, abort") << QBluetoothServiceInfo::L2capProtocol
                                      << Abort << address << port << QByteArray();
    QTest::newRow("available, abort with data") << QBluetoothServiceInfo::L2capProtocol
                                                << Abort << address << port << QByteArray("Test message\n");

}

void tst_QBluetoothSocket::tst_clientConnection()
{
    QFETCH(QBluetoothServiceInfo::Protocol, socketprotocol);
    QFETCH(ClientConnectionShutdown, shutdown);
    QFETCH(QBluetoothAddress, address);
    QFETCH(quint16, port);
    QFETCH(QByteArray, data);
    int loop = 5;

    tryagain:
    /* Construction */
    QBluetoothSocket *socket = new QBluetoothSocket(socketprotocol);

    QSignalSpy stateSpy(socket, SIGNAL(stateChanged(QBluetoothSocket::SocketState)));

    QCOMPARE(socket->socketType(), socketprotocol);
    QCOMPARE(socket->state(), QBluetoothSocket::UnconnectedState);

    /* Connection */
    QSignalSpy connectedSpy(socket, SIGNAL(connected()));
    QSignalSpy errorSpy(socket, SIGNAL(error(QBluetoothSocket::SocketError)));

    socket->connectToService(address, port);

    QCOMPARE(stateSpy.count(), 1);
    QCOMPARE(stateSpy.takeFirst().at(0).value<QBluetoothSocket::SocketState>(), QBluetoothSocket::ConnectingState);
    QCOMPARE(socket->state(), QBluetoothSocket::ConnectingState);

    stateSpy.clear();

    int connectTime = MaxConnectTime;
    while (connectedSpy.count() == 0 && errorSpy.count() == 0 && connectTime > 0) {
        QTest::qWait(1000);
        connectTime -= 1000;
    }

    if (shutdown == Error) {
        QCOMPARE(connectedSpy.count(), 0);
        QCOMPARE(errorSpy.count(), 1);
        QCOMPARE(stateSpy.count(), 1);
        QCOMPARE(stateSpy.takeFirst().at(0).value<QBluetoothSocket::SocketState>(), QBluetoothSocket::UnconnectedState);
        QCOMPARE(socket->state(), QBluetoothSocket::UnconnectedState);
        // The remote service needs time to close the connection and resume listening
        QTest::qSleep(100);
        return;
    } else {
        if (errorSpy.count() != 0) {
            qDebug() << errorSpy.takeFirst().at(0).toInt();
            if (loop--)
                goto tryagain;
            QSKIP("Connection error");
        }
        QCOMPARE(connectedSpy.count(), 1);
        QCOMPARE(stateSpy.count(), 1);
        QCOMPARE(stateSpy.takeFirst().at(0).value<QBluetoothSocket::SocketState>(), QBluetoothSocket::ConnectedState);
        QCOMPARE(socket->state(), QBluetoothSocket::ConnectedState);
    }
    QVERIFY(shutdown != Error);

    stateSpy.clear();

    /* Read / Write */
    QSignalSpy bytesWrittenSpy(socket, SIGNAL(bytesWritten(qint64)));

    if (!data.isEmpty()) {
        // Write data but don't flush.
        socket->write(data);
    }

    /* Disconnection */
    QSignalSpy disconnectedSpy(socket, SIGNAL(disconnected()));

    if (shutdown == Abort) {
        socket->abort();

// TODO: no buffereing, all data is sent on write
        if (!data.isEmpty()) {
            // Check that pending write did not complete.
//            QEXPECT_FAIL("", "TODO: need to implement write buffering", Continue);
            QCOMPARE(bytesWrittenSpy.count(), 0);
        }

        QCOMPARE(disconnectedSpy.count(), 1);
        QCOMPARE(stateSpy.count(), 1);
        QCOMPARE(stateSpy.takeFirst().at(0).value<QBluetoothSocket::SocketState>(), QBluetoothSocket::UnconnectedState);
        QCOMPARE(socket->state(), QBluetoothSocket::UnconnectedState);
    } else {
        if (shutdown == Disconnect)
            socket->disconnectFromService();
        else if (shutdown == Close)
            socket->close();

        if (socket->state() == QBluetoothSocket::UnconnectedState){
            // Linux for example on close goes through closing and unconnected without stopping
            QCOMPARE(stateSpy.count(), 2); // closing + unconnected
            QCOMPARE(stateSpy.takeFirst().at(0).value<QBluetoothSocket::SocketState>(), QBluetoothSocket::ClosingState);
            QCOMPARE(stateSpy.takeFirst().at(0).value<QBluetoothSocket::SocketState>(), QBluetoothSocket::UnconnectedState);
            QCOMPARE(socket->state(), QBluetoothSocket::UnconnectedState);

        }
        else {
            QCOMPARE(stateSpy.count(), 1);
            QCOMPARE(stateSpy.takeFirst().at(0).value<QBluetoothSocket::SocketState>(), QBluetoothSocket::ClosingState);
            QCOMPARE(socket->state(), QBluetoothSocket::ClosingState);

            int disconnectTime = MaxConnectTime;
            while (disconnectedSpy.count() == 0 && disconnectTime > 0) {
                QTest::qWait(1000);
                disconnectTime -= 1000;
            }
            QCOMPARE(disconnectedSpy.count(), 1);
            QCOMPARE(stateSpy.count(), 1);
            QCOMPARE(stateSpy.takeFirst().at(0).value<QBluetoothSocket::SocketState>(), QBluetoothSocket::UnconnectedState);
        }

        if (!data.isEmpty()) {
            // Check that pending write completed.
            QCOMPARE(bytesWrittenSpy.count(), 1);
            QCOMPARE(bytesWrittenSpy.at(0).at(0).toLongLong(), qint64(data.length()));
        }

    }

    delete socket;

    // The remote service needs time to close the connection and resume listening
    QTest::qSleep(100);
}

void tst_QBluetoothSocket::tst_serviceConnection_data()
{
    QTest::addColumn<QBluetoothServiceInfo>("serviceInfo");

    QBluetoothServiceInfo serviceInfo;

    //serviceInfo.setDevice(QBluetoothDeviceInfo(QBluetoothAddress("001167602023"), QString(), 0));
    QBluetoothAddress address(BTADDRESS);
    serviceInfo.setDevice(QBluetoothDeviceInfo(address, QString(), 0));

    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;

    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap));
    protocolDescriptorList.append(QVariant::fromValue(protocol));

    protocol.clear();
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm)) << QVariant::fromValue(quint8(10));
    protocolDescriptorList.append(QVariant::fromValue(protocol));

    serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);

    QTest::newRow("service connection") << serviceInfo;
}

void tst_QBluetoothSocket::tst_serviceConnection()
{
    QFETCH(QBluetoothServiceInfo, serviceInfo);

    /* Construction */
    QBluetoothSocket *socket = new QBluetoothSocket;

    QSignalSpy stateSpy(socket, SIGNAL(stateChanged(QBluetoothSocket::SocketState)));

    QCOMPARE(socket->socketType(), QBluetoothServiceInfo::UnknownProtocol);
    QCOMPARE(socket->state(), QBluetoothSocket::UnconnectedState);

    /* Connection */
    QSignalSpy connectedSpy(socket, SIGNAL(connected()));
    QSignalSpy errorSpy(socket, SIGNAL(error(QBluetoothSocket::SocketError)));

    socket->connectToService(serviceInfo);

    QCOMPARE(stateSpy.count(), 1);
    QCOMPARE(stateSpy.takeFirst().at(0).value<QBluetoothSocket::SocketState>(), QBluetoothSocket::ConnectingState);
    QCOMPARE(socket->state(), QBluetoothSocket::ConnectingState);

    stateSpy.clear();

    int connectTime = MaxConnectTime;
    while (connectedSpy.count() == 0 && errorSpy.count() == 0 && connectTime > 0) {
        QTest::qWait(1000);
        connectTime -= 1000;
    }

    if (errorSpy.count() != 0) {
        qDebug() << errorSpy.takeFirst().at(0).toInt();
        QSKIP("Connection error");
    }
    QCOMPARE(connectedSpy.count(), 1);
    QCOMPARE(stateSpy.count(), 1);
    QCOMPARE(stateSpy.takeFirst().at(0).value<QBluetoothSocket::SocketState>(), QBluetoothSocket::ConnectedState);
    QCOMPARE(socket->state(), QBluetoothSocket::ConnectedState);

    stateSpy.clear();

    /* Disconnection */
    QSignalSpy disconnectedSpy(socket, SIGNAL(disconnected()));

    socket->disconnectFromService();

    QVERIFY(stateSpy.count() >= 1);
    QCOMPARE(stateSpy.takeFirst().at(0).value<QBluetoothSocket::SocketState>(), QBluetoothSocket::ClosingState);

    int disconnectTime = MaxConnectTime;
    while (disconnectedSpy.count() == 0 && disconnectTime > 0) {
        QTest::qWait(1000);
        disconnectTime -= 1000;
    }

    QCOMPARE(disconnectedSpy.count(), 1);
    QCOMPARE(stateSpy.count(), 1);
    QCOMPARE(stateSpy.takeFirst().at(0).value<QBluetoothSocket::SocketState>(), QBluetoothSocket::UnconnectedState);

    delete socket;

    // The remote service needs time to close the connection and resume listening
    QTest::qSleep(100);
}

void tst_QBluetoothSocket::tst_clientCommunication_data()
{
    QTest::addColumn<QStringList>("data");

    {
        QStringList data;
        data << QLatin1String("Test line one.\n");
        data << QLatin1String("Test line two, with longer data.\n");

        QTest::newRow("two line test") << data;
    }
}

void tst_QBluetoothSocket::tst_clientCommunication()
{
    QFETCH(QStringList, data);

    /* Construction */
    QBluetoothSocket *socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);

    QSignalSpy stateSpy(socket, SIGNAL(stateChanged(QBluetoothSocket::SocketState)));

    QCOMPARE(socket->socketType(), QBluetoothServiceInfo::RfcommProtocol);
    QCOMPARE(socket->state(), QBluetoothSocket::UnconnectedState);

    /* Connection */
    QSignalSpy connectedSpy(socket, SIGNAL(connected()));

    socket->connectToService(QBluetoothAddress(BTADDRESS), 10);    // echo service running on device 00:11:67:60:20:23

    QCOMPARE(stateSpy.count(), 1);
    QCOMPARE(qvariant_cast<QBluetoothSocket::SocketState>(stateSpy.takeFirst().at(0)), QBluetoothSocket::ConnectingState);
    QCOMPARE(socket->state(), QBluetoothSocket::ConnectingState);

    stateSpy.clear();

    int connectTime = MaxConnectTime;
    while (connectedSpy.count() == 0 && connectTime > 0) {
        QTest::qWait(1000);
        connectTime -= 1000;
    }

    QCOMPARE(connectedSpy.count(), 1);
    QCOMPARE(stateSpy.count(), 1);
    QCOMPARE(qvariant_cast<QBluetoothSocket::SocketState>(stateSpy.takeFirst().at(0)), QBluetoothSocket::ConnectedState);
    QCOMPARE(socket->state(), QBluetoothSocket::ConnectedState);

    stateSpy.clear();

    /* Read / Write */
    QCOMPARE(socket->bytesToWrite(), qint64(0));
    QCOMPARE(socket->bytesAvailable(), qint64(0));

    {
        /* Send line by line with event loop */
        foreach (const QString &line, data) {
            QSignalSpy readyReadSpy(socket, SIGNAL(readyRead()));
            QSignalSpy bytesWrittenSpy(socket, SIGNAL(bytesWritten(qint64)));

            socket->write(line.toUtf8());

//            QEXPECT_FAIL("", "TODO: need to implement write buffering", Continue);
            QCOMPARE(socket->bytesToWrite(), qint64(line.length()));

            int readWriteTime = MaxReadWriteTime;
            while ((bytesWrittenSpy.count() == 0 || readyReadSpy.count() == 0) && readWriteTime > 0) {
                QTest::qWait(1000);
                readWriteTime -= 1000;
            }

            QCOMPARE(bytesWrittenSpy.count(), 1);
            QCOMPARE(bytesWrittenSpy.at(0).at(0).toLongLong(), qint64(line.length()));

            readWriteTime = MaxReadWriteTime;
            while ((readyReadSpy.count() == 0) && readWriteTime > 0) {
                QTest::qWait(1000);
                readWriteTime -= 1000;
            }

            QCOMPARE(readyReadSpy.count(), 1);

            QCOMPARE(socket->bytesAvailable(), qint64(line.length()));

            QVERIFY(socket->canReadLine());

            QByteArray echoed = socket->readAll();

            QCOMPARE(line.toUtf8(), echoed);
        }
    }

#if 0
    {
        /* Send line by line without event loop */
        QSignalSpy readyReadSpy(socket, SIGNAL(readyRead()));
        QSignalSpy bytesWrittenSpy(socket, SIGNAL(bytesWritten(qint64)));

        qint64 bytesToWrite = 0;
        foreach (const QString &line, data) {
            socket->write(line.toUtf8());
            bytesToWrite += line.toUtf8().length();
            QCOMPARE(socket->bytesToWrite(), bytesToWrite);
        }

        int readWriteTime = MaxReadWriteTime;
        while (socket->bytesToWrite() != 0 && readyReadSpy.count() == 0 && readWriteTime > 0) {
            QTest::qWait(1000);
            readWriteTime -= 1000;
        }

        QCOMPARE(bytesWrittenSpy.count(), 1);
        QCOMPARE(bytesWrittenSpy.at(0).at(0).toLongLong(), bytesToWrite);
        QCOMPARE(readyReadSpy.count(), 1);

        QCOMPARE(socket->bytesAvailable(), bytesToWrite);

        QVERIFY(socket->canReadLine());

        QByteArray echoed = socket->readAll();

        QCOMPARE(data.join(QString()).toUtf8(), echoed);
    }
#endif

    {
        /* Send all at once */
        QSignalSpy readyReadSpy(socket, SIGNAL(readyRead()));
        QSignalSpy bytesWrittenSpy(socket, SIGNAL(bytesWritten(qint64)));

        QString joined = data.join(QString());
        socket->write(joined.toUtf8());

//        QEXPECT_FAIL("", "TODO: need to implement write buffering", Continue);
        QCOMPARE(socket->bytesToWrite(), qint64(joined.length()));

        int readWriteTime = MaxReadWriteTime;
        while ((bytesWrittenSpy.count() == 0 || readyReadSpy.count() == 0) && readWriteTime > 0) {
            QTest::qWait(1000);
            readWriteTime -= 1000;
        }

        QCOMPARE(bytesWrittenSpy.count(), 1);
        QCOMPARE(bytesWrittenSpy.at(0).at(0).toLongLong(), qint64(joined.length()));
        QCOMPARE(readyReadSpy.count(), 1);

        QCOMPARE(socket->bytesAvailable(), qint64(joined.length()));

        QVERIFY(socket->canReadLine());

        QByteArray echoed = socket->readAll();

        QCOMPARE(joined.toUtf8(), echoed);
    }

    /* Disconnection */
    QSignalSpy disconnectedSpy(socket, SIGNAL(disconnected()));

    socket->disconnectFromService();

    int disconnectTime = MaxConnectTime;
    while (disconnectedSpy.count() == 0 && disconnectTime > 0) {
        QTest::qWait(1000);
        disconnectTime -= 1000;
    }

    QCOMPARE(disconnectedSpy.count(), 1);
    QCOMPARE(stateSpy.count(), 2);
    QCOMPARE(qvariant_cast<QBluetoothSocket::SocketState>(stateSpy.takeFirst().at(0)), QBluetoothSocket::ClosingState);
    QCOMPARE(qvariant_cast<QBluetoothSocket::SocketState>(stateSpy.takeFirst().at(0)), QBluetoothSocket::UnconnectedState);

    delete socket;

    // The remote service needs time to close the connection and resume listening
    QTest::qSleep(100);

}

void tst_QBluetoothSocket::tst_localPeer_data()
{
//    QTest::addColumn<QString>("localName");
//    QTest::addColumn<QBluetoothAddress>("localAddress");
//    QTest::addColumn<quint16>("localPort");
    QTest::addColumn<QString>("peerName");
    QTest::addColumn<QBluetoothAddress>("peerAddress");
    QTest::addColumn<quint16>("peerPort");

    QTest::newRow("test") << QString(BTADDRESS) << QBluetoothAddress(BTADDRESS) << quint16(10);
}

void tst_QBluetoothSocket::tst_localPeer()
{
//    QFETCH(QString, localName);
//    QFETCH(QBluetoothAddress, localAddress);
//    QFETCH(quint16, localPort);
    QFETCH(QString, peerName);
    QFETCH(QBluetoothAddress, peerAddress);
    QFETCH(quint16, peerPort);
    Q_UNUSED(peerPort)

    QStringList args;
    args << "name" << peerAddress.toString();
    QProcess *hcitool = new QProcess();
    hcitool->start("hcitool", args);
    hcitool->waitForReadyRead();
    QString peerNameHCI = hcitool->readLine().trimmed();
    hcitool->close();
    delete hcitool;


    /* Construction */
    QBluetoothSocket *socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);

    QSignalSpy stateSpy(socket, SIGNAL(stateChanged(QBluetoothSocket::SocketState)));

    QCOMPARE(socket->socketType(), QBluetoothServiceInfo::RfcommProtocol);
    QCOMPARE(socket->state(), QBluetoothSocket::UnconnectedState);

    /* Connection */
    QSignalSpy connectedSpy(socket, SIGNAL(connected()));

    socket->connectToService(QBluetoothAddress(BTADDRESS), 11);

    QCOMPARE(stateSpy.count(), 1);
    QCOMPARE(qvariant_cast<QBluetoothSocket::SocketState>(stateSpy.takeFirst().at(0)), QBluetoothSocket::ConnectingState);
    QCOMPARE(socket->state(), QBluetoothSocket::ConnectingState);

    stateSpy.clear();

    int connectTime = MaxConnectTime;
    while (connectedSpy.count() == 0 && connectTime > 0) {
        QTest::qWait(1000);
        connectTime -= 1000;
    }

    QCOMPARE(connectedSpy.count(), 1);
    QCOMPARE(stateSpy.count(), 1);
    QCOMPARE(qvariant_cast<QBluetoothSocket::SocketState>(stateSpy.takeFirst().at(0)), QBluetoothSocket::ConnectedState);
    QCOMPARE(socket->state(), QBluetoothSocket::ConnectedState);

    QByteArray echoed = socket->readAll();
    QString data(echoed);
    QStringList list = data.split(QChar(' '), QString::SkipEmptyParts);
    // list is:
    // [0]local mac, [1]local port, [2]local name, [3]peer mac, [4]peer port

    QCOMPARE(socket->peerAddress(), QBluetoothAddress(list[0]));
    QCOMPARE(socket->peerPort(), list[1].toUShort());

    QCOMPARE(socket->localName(), list[2]);
    QCOMPARE(socket->localAddress(), QBluetoothAddress(list[3]));
    QCOMPARE(socket->localPort(), list[4].toUShort());
    QCOMPARE(socket->peerName(), peerNameHCI);

    /* Disconnection */
    QSignalSpy disconnectedSpy(socket, SIGNAL(disconnected()));

    socket->disconnectFromService();

    int disconnectTime = MaxConnectTime;
    while (disconnectedSpy.count() == 0 && disconnectTime > 0) {
        QTest::qWait(1000);
        disconnectTime -= 1000;
    }

    QCOMPARE(disconnectedSpy.count(), 1);
    QCOMPARE(stateSpy.count(), 2);
    QCOMPARE(qvariant_cast<QBluetoothSocket::SocketState>(stateSpy.takeFirst().at(0)), QBluetoothSocket::ClosingState);
    QCOMPARE(qvariant_cast<QBluetoothSocket::SocketState>(stateSpy.takeFirst().at(0)), QBluetoothSocket::UnconnectedState);

    delete socket;
}

void tst_QBluetoothSocket::tst_error()
{
    QBluetoothSocket socket;
    QSignalSpy errorSpy(&socket, SIGNAL(error(QBluetoothSocket::SocketError)));
    QCOMPARE(errorSpy.count(), 0);
    const QBluetoothSocket::SocketError e = socket.error();

    QVERIFY(e != QBluetoothSocket::HostNotFoundError
            && e != QBluetoothSocket::NetworkError
            && e != QBluetoothSocket::ServiceNotFoundError
            && e != QBluetoothSocket::UnknownSocketError);

    QVERIFY(socket.errorString() == QString());
}

QTEST_MAIN(tst_QBluetoothSocket)

#include "tst_qbluetoothsocket.moc"


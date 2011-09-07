/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QDebug>

#include <ql2capserver.h>
#include <qbluetoothsocket.h>

Q_DECLARE_METATYPE(QBluetooth::SecurityFlags);

// Max time to wait for connection
static const int MaxConnectTime = 60 * 1000;   // 1 minute in ms

class tst_QL2capServer : public QObject
{
    Q_OBJECT

public:
    tst_QL2capServer();
    ~tst_QL2capServer();

private slots:
    void initTestCase();

    void tst_construction();

    void tst_listen_data();
    void tst_listen();

    void tst_pendingConnections_data();
    void tst_pendingConnections();

    void tst_receive_data();
    void tst_receive();

    void tst_secureFlags();
};

tst_QL2capServer::tst_QL2capServer()
{
}

tst_QL2capServer::~tst_QL2capServer()
{
}

void tst_QL2capServer::initTestCase()
{
    qRegisterMetaType<QBluetooth::SecurityFlags>("QBluetooth::SecurityFlags");
}

void tst_QL2capServer::tst_construction()
{
    {
        QL2capServer server;

        QVERIFY(!server.isListening());
        QCOMPARE(server.maxPendingConnections(), 1);
        QVERIFY(!server.hasPendingConnections());
        QVERIFY(server.nextPendingConnection() == 0);
        QVERIFY(server.serverAddress().isNull());
        QCOMPARE(server.serverPort(), quint16(24160));
    }
}

void tst_QL2capServer::tst_listen_data()
{
    QTest::addColumn<QBluetoothAddress>("address");
    QTest::addColumn<quint16>("port");

    QTest::newRow("default") << QBluetoothAddress() << quint16(0);
    QTest::newRow("specified address") << QBluetoothAddress("00:11:B1:08:AD:B8") << quint16(0);
    QTest::newRow("specified port") << QBluetoothAddress() << quint16(24160);
    QTest::newRow("specified address/port") << QBluetoothAddress("00:11:B1:08:AD:B8") << quint16(10);
}

void tst_QL2capServer::tst_listen()
{
    QFETCH(QBluetoothAddress, address);
    QFETCH(quint16, port);

    {
        QL2capServer server;

        bool result = server.listen(address, port);

        QVERIFY(result);
        QVERIFY(server.isListening());

        if (!address.isNull())
            QCOMPARE(server.serverAddress(), address);
        else
            QVERIFY(!server.serverAddress().isNull());

        if (port != 0)
            QCOMPARE(server.serverPort(), port);
        else
            QVERIFY(server.serverPort() != 0);

        QCOMPARE(server.maxPendingConnections(), 1);

        QVERIFY(!server.hasPendingConnections());
        QVERIFY(server.nextPendingConnection() == 0);

        server.close();

        QVERIFY(!server.isListening());

        QVERIFY(server.serverAddress().isNull());
        QVERIFY(server.serverPort() == 0);

        QVERIFY(!server.hasPendingConnections());
        QVERIFY(server.nextPendingConnection() == 0);
    }
}

void tst_QL2capServer::tst_pendingConnections_data()
{
    QTest::addColumn<int>("maxConnections");

    QTest::newRow("1 connection") << 1;
    //QTest::newRow("2 connections") << 2;
}

void tst_QL2capServer::tst_pendingConnections()
{
    QFETCH(int, maxConnections);

    {
        QL2capServer server;

        server.setMaxPendingConnections(maxConnections);

        bool result = server.listen();

        QVERIFY(result);
        QVERIFY(server.isListening());

        qDebug() << "Listening on L2CAP channel:" << server.serverPort();

        QCOMPARE(server.maxPendingConnections(), maxConnections);

        QVERIFY(!server.hasPendingConnections());
        QVERIFY(server.nextPendingConnection() == 0);

        {
            /* wait for maxConnections simultaneous connections */
            qDebug() << "Waiting for" << maxConnections << "simultaneous connections.";

            QSignalSpy connectionSpy(&server, SIGNAL(newConnection()));

            int connectTime = MaxConnectTime;
            while (connectionSpy.count() < maxConnections && connectTime > 0) {
                QTest::qWait(1000);
                connectTime -= 1000;
            }

            QList<QBluetoothSocket *> sockets;
            while (server.hasPendingConnections())
                sockets.append(server.nextPendingConnection());

            QCOMPARE(connectionSpy.count(), maxConnections);
            QCOMPARE(sockets.count(), maxConnections);

            foreach (QBluetoothSocket *socket, sockets) {
                qDebug() << socket->state();
                QVERIFY(socket->state() == QBluetoothSocket::ConnectedState);
                QVERIFY(socket->openMode() == QIODevice::ReadWrite);
            }

            QVERIFY(!server.hasPendingConnections());
            QVERIFY(server.nextPendingConnection() == 0);

            while (!sockets.isEmpty()) {
                QBluetoothSocket *socket = sockets.takeFirst();
                socket->close();
                delete socket;
            }
        }

        server.close();
    }
}

void tst_QL2capServer::tst_receive_data()
{
    QTest::addColumn<QByteArray>("expected");

    QTest::newRow("test") << QByteArray("hello\r\n");
}

void tst_QL2capServer::tst_receive()
{
    QFETCH(QByteArray, expected);

    QL2capServer server;

    bool result = server.listen();

    QVERIFY(result);

    qDebug() << "Listening on L2CAP channel:" << server.serverPort();

    int connectTime = MaxConnectTime;
    while (!server.hasPendingConnections() && connectTime > 0) {
        QTest::qWait(1000);
        connectTime -= 1000;
    }

    QVERIFY(server.hasPendingConnections());

    QBluetoothSocket *socket = server.nextPendingConnection();

    QVERIFY(socket->state() == QBluetoothSocket::ConnectedState);
    QVERIFY(socket->openMode() == QIODevice::ReadWrite);

    QSignalSpy readyReadSpy(socket, SIGNAL(readyRead()));

    int readyReadTime = 60000;
    while (readyReadSpy.isEmpty() && readyReadTime > 0) {
        QTest::qWait(1000);
        readyReadTime -= 1000;
    }

    QVERIFY(!readyReadSpy.isEmpty());

    const QByteArray data = socket->readAll();

    QCOMPARE(data, expected);
}

void tst_QL2capServer::tst_secureFlags()
{
    QL2capServer server;
    QCOMPARE(server.securityFlags(), QBluetooth::NoSecurity);

    server.setSecurityFlags(QBluetooth::Encryption);
    QCOMPARE(server.securityFlags(), QBluetooth::Encryption);
}

QTEST_MAIN(tst_QL2capServer)

#include "tst_ql2capserver.moc"

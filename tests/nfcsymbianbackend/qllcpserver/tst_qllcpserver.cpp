/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtNfc module of the Qt Toolkit.
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

#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <qllcpserver.h>
#include <qllcpsocket.h>

#include "qnfctestcommon.h"
#include "qnfctestutil.h"

QString TestUri("urn:nfc:xsn:nokia:symbiantest");

QTNFC_USE_NAMESPACE

static qint64 countBytesWritten(QSignalSpy& bytesWrittenSpy)
    {
    qint64 ret = 0;
    for(int i = 0; i < bytesWrittenSpy.count(); i++)
        {
        ret+=bytesWrittenSpy[i].at(0).value<qint64>();
        }
    return ret;
    }

Q_DECLARE_METATYPE(QLlcpSocket::SocketError)
class tst_QLlcpServer : public QObject
{
    Q_OBJECT

public:
    tst_QLlcpServer();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();


    // basic acceptance test
    void newConnection();
    void newConnection_data();
    void newConnection_wait();
    void newConnection_wait_data();
    void api_coverage();

    //advance test case
    void multiConnection();

    // nagetive testcases
    void negTestCase1();
};

tst_QLlcpServer::tst_QLlcpServer()
{
}

void tst_QLlcpServer::initTestCase()
{
    qRegisterMetaType<QLlcpSocket::SocketError>("QLlcpSocket::SocketError");
}

void tst_QLlcpServer::cleanupTestCase()
{
}
void tst_QLlcpServer::newConnection_data()
{
    QString hint = "handshake 1";
    QTest::addColumn<QString>("uri");
    QTest::addColumn<QString>("hint");
    QTest::newRow("0") << TestUri << hint;
}

/*!
 Description: Unit test for NFC LLCP server async functions

 TestScenario: 1. Server will listen to a pre-defined URI
               2. Wait client to connect.
               3. Read message from client.
               4. Echo the same message back to client
               5. Wait client disconnect event.

 TestExpectedResults:
               1. The listen successfully set up.
               2. The message has be received from client.
               3. The echoed message has been sent to client.
               4. Connection disconnected and NO error signals emitted.
*/
void tst_QLlcpServer::newConnection()
{
    QFETCH(QString, uri);
    QFETCH(QString, hint);

    QLlcpServer server;
    qDebug() << "Create QLlcpServer completed";
    qDebug() << "Start listening...";
    QSignalSpy connectionSpy(&server, SIGNAL(newConnection()));

    bool ret = server.listen(uri);
    QVERIFY(ret);
    qDebug() << "Listen() return ok";

    QNfcTestUtil::ShowAutoMsg(hint, &connectionSpy, 1);

    QTRY_VERIFY(!connectionSpy.isEmpty());
    qDebug() << "try to call nextPendingConnection()";
    QLlcpSocket *socket = server.nextPendingConnection();
    QVERIFY(socket != NULL);
    QSignalSpy readyReadSpy(socket, SIGNAL(readyRead()));
    QSignalSpy errorSpy(socket, SIGNAL(error(QLlcpSocket::SocketError)));

    //Get data from client
    QTRY_VERIFY(!readyReadSpy.isEmpty());

    qDebug() << "Bytes available = " << socket->bytesAvailable();
    quint16 blockSize = 0;
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_4_6);
    while (socket->bytesAvailable() < (int)sizeof(quint16)){
        QSignalSpy readyRead(socket, SIGNAL(readyRead()));
        QTRY_VERIFY(!readyRead.isEmpty());
    }
    in >> blockSize;
    qDebug()<<"Read blockSize from client: " << blockSize;
    while (socket ->bytesAvailable() < blockSize){
        QSignalSpy readyRead(socket, SIGNAL(readyRead()));
        QTRY_VERIFY(!readyRead.isEmpty());
    }
    QString echo;
    in >> echo;
    qDebug() << "Read data from client:" << echo;
    //Send data to client
    QSignalSpy bytesWrittenSpy(socket, SIGNAL(bytesWritten(qint64)));
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (quint16)0;
    out << echo;
    qDebug()<<"Write echoed data back to client";
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    qint64 val = socket->write(block);
    qDebug("Write() return value = %d", val);
    QVERIFY(val == 0);

    QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
    qint64 written = countBytesWritten(bytesWrittenSpy);
    qDebug()<<"Server::bytesWritten signal return value = " << written;
    while (written < block.size())
        {
        QSignalSpy bytesWrittenSpy(socket, SIGNAL(bytesWritten(qint64)));
        QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
        written += countBytesWritten(bytesWrittenSpy);
        }
    QVERIFY(written == block.size());
    //Now data has been sent,check the if existing error
    QVERIFY(errorSpy.isEmpty());
    QTest::qWait(1500);//give some time to client to finish
    server.close();
}

/*!
 Description: Unit test for NFC LLCP server sync(waitXXX) functions

 TestScenario: 1. Server will listen to a pre-defined URI
               2. Wait client to connect.
               3. Read message from client.
               4. Echo the same message back to client
               5. Wait client disconnect event.

 TestExpectedResults:
               1. The listen successfully set up.
               2. The message has be received from client.
               3. The echoed message has been sent to client.
               4. Connection disconnected and NO error signals emitted.
*/
void tst_QLlcpServer::newConnection_wait()
    {
    QFETCH(QString, uri);
    QFETCH(QString, hint);

    QLlcpServer server;
    qDebug() << "Create QLlcpServer completed";
    qDebug() << "Start listening...";
    bool ret = server.listen(uri);
    QVERIFY(ret);
    qDebug() << "Listen() return ok";
    QSignalSpy connectionSpy(&server, SIGNAL(newConnection()));

    QNfcTestUtil::ShowAutoMsg(hint, &connectionSpy, 1);

    QTRY_VERIFY(!connectionSpy.isEmpty());
    qDebug() << "try to call nextPendingConnection()";
    QLlcpSocket *socket = server.nextPendingConnection();
    QVERIFY(socket != NULL);

    QSignalSpy errorSpy(socket, SIGNAL(error(QLlcpSocket::SocketError)));
    //Get data from client
    const int Timeout = 10 * 1000;

    quint16 blockSize = 0;
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_4_6);
    while (socket->bytesAvailable() < (int)sizeof(quint16)) {
        bool ret = socket->waitForReadyRead(Timeout);
        QVERIFY(ret);
    }

    in >> blockSize;
    qDebug()<<"Read blockSize from client: " << blockSize;
    while (socket ->bytesAvailable() < blockSize){
        bool ret = socket->waitForReadyRead(Timeout);
        QVERIFY(ret);
    }
    QString echo;
    in >> echo;
    qDebug() << "Read data from client:" << echo;
    //Send data to client
    QSignalSpy bytesWrittenSpy(socket, SIGNAL(bytesWritten(qint64)));

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (quint16)0;
    out << echo;
    qDebug()<<"Write echoed data back to client";
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    qint64 val = socket->write(block);
    qDebug("Write() return value = %d", val);
    QVERIFY(val == 0);

    ret = socket->waitForBytesWritten(Timeout);
    QVERIFY(ret);

    QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
    qint64 written = countBytesWritten(bytesWrittenSpy);

    while (written < block.size())
        {
        QSignalSpy bytesWrittenSpy(socket, SIGNAL(bytesWritten(qint64)));
        bool ret = socket->waitForBytesWritten(Timeout);
        QVERIFY(ret);
        QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
        written += countBytesWritten(bytesWrittenSpy);
        }
    QVERIFY(written == block.size());
    //Now data has been sent,check the if existing error
    if (!errorSpy.isEmpty())
        {
            QLlcpSocket::SocketError error = errorSpy.first().at(0).value<QLlcpSocket::SocketError>();
            qDebug("QLlcpSocket::SocketError =%d", error);
        }
    QVERIFY(errorSpy.isEmpty());
    QTest::qWait(1500);//give some time to client to finish
    server.close();
    }

void tst_QLlcpServer::newConnection_wait_data()
    {
    QTest::addColumn<QString>("uri");
    QTest::addColumn<QString>("hint");
    QTest::newRow("0") << TestUri
            << "handshake 2";
    }



/*!
 Description: LLCP Server API test & Socket readDatagram API test
 TestScenario:
     1) Read two datagrams from llcp client device
     2) Covered API: readDatagram(), serviceUri(), servicePort(), isListening(), serverError()
 */
void tst_QLlcpServer::api_coverage()
{
    QString uri = TestUri;
    QLlcpServer server;
    QSignalSpy connectionSpy(&server, SIGNAL(newConnection()));
    bool ret = server.listen(uri);
    QVERIFY(ret);

    QString message("api_coverage test");

    QNfcTestUtil::ShowAutoMsg(message, &connectionSpy, 1);
    QTRY_VERIFY(!connectionSpy.isEmpty());

    bool hasPending = server.hasPendingConnections();
    QVERIFY(hasPending);
    QLlcpSocket *socket = server.nextPendingConnection();
    QVERIFY(socket != NULL);
    QSignalSpy readyReadSpy(socket, SIGNAL(readyRead()));
    //Get data from client
    QTRY_VERIFY(!readyReadSpy.isEmpty());

    qint64 size = socket->bytesAvailable();
    QTRY_VERIFY(size > 0);
    QByteArray datagram;
    datagram.resize(size);
    qint64 readSize = socket->readDatagram(datagram.data(), datagram.size());
    QVERIFY(readSize == size);

    qDebug()<<"Server Uri = " << server.serviceUri();
    QCOMPARE(uri,server.serviceUri());

    quint8 unsupportedPort = 0;
    QCOMPARE(unsupportedPort,server.serverPort());

    QVERIFY(server.isListening() == true);
    QVERIFY(server.serverError() == QLlcpSocket::UnknownSocketError);
    server.close();
}


/*!
 Description: listen negative test - listen twice
*/
void tst_QLlcpServer::negTestCase1()
{

    QString uri = TestUri;
    QLlcpServer server;
    QSignalSpy connectionSpy(&server, SIGNAL(newConnection()));
    bool ret = server.listen(uri);
    QVERIFY(ret);

    QString message("negTestCase1 test");

    QNfcTestUtil::ShowAutoMsg(message, &connectionSpy, 1);

    ret = server.listen(uri);
    QVERIFY(ret == false);
    QTest::qWait(1 * 1000);//give some time to wait new connection
    server.close();
}
/*!
 Description: Add a llcp2 server which will always
 run( actually run twice ), then a client in a device connect,
 after the case finish, another device connect the
 server again.

 CounterPart: tst_qllcpsockettype2 echo:"0"
 run this case twice to test multiConnection()
*/
void tst_QLlcpServer::multiConnection()
    {
    QString uri = TestUri;
    QString hint ="multiConnection test";

    QLlcpServer server;
    qDebug() << "Create QLlcpServer completed";
    qDebug() << "Start listening...";
    QSignalSpy connectionSpy(&server, SIGNAL(newConnection()));

    bool ret = server.listen(uri);
    QVERIFY(ret);
    qDebug() << "Listen() return ok";
    const int KLoopCount = 2;
    int loopCount = 0;
    while(loopCount < KLoopCount)
        {
        qDebug() << "#########Loop count = " << loopCount <<" #########";
        QNfcTestUtil::ShowAutoMsg(hint, &connectionSpy, 1);

        QTRY_VERIFY(!connectionSpy.isEmpty());
        qDebug() << "try to call nextPendingConnection()";
        QLlcpSocket *socket = server.nextPendingConnection();
        QVERIFY(socket != NULL);
        QSignalSpy readyReadSpy(socket, SIGNAL(readyRead()));
        QSignalSpy errorSpy(socket, SIGNAL(error(QLlcpSocket::SocketError)));

        //Get data from client
        QTRY_VERIFY(!readyReadSpy.isEmpty());

        qDebug() << "Bytes available = " << socket->bytesAvailable();
        quint16 blockSize = 0;
        QDataStream in(socket);
        in.setVersion(QDataStream::Qt_4_6);
        while (socket->bytesAvailable() < (int)sizeof(quint16)){
            QSignalSpy readyRead(socket, SIGNAL(readyRead()));
            QTRY_VERIFY(!readyRead.isEmpty());
        }
        in >> blockSize;
        qDebug()<<"Read blockSize from client: " << blockSize;
        while (socket ->bytesAvailable() < blockSize){
            QSignalSpy readyRead(socket, SIGNAL(readyRead()));
            QTRY_VERIFY(!readyRead.isEmpty());
        }
        QString echo;
        in >> echo;
        qDebug() << "Read data from client:" << echo;
        //Send data to client
        QSignalSpy bytesWrittenSpy(socket, SIGNAL(bytesWritten(qint64)));
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_6);
        out << (quint16)0;
        out << echo;
        qDebug()<<"Write echoed data back to client";
        out.device()->seek(0);
        out << (quint16)(block.size() - sizeof(quint16));
        qint64 val = socket->write(block);
        qDebug("Write() return value = %d", val);
        QVERIFY(val == 0);

        QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
        qint64 written = countBytesWritten(bytesWrittenSpy);
        qDebug()<<"Server::bytesWritten signal return value = " << written;
        while (written < block.size())
            {
            QSignalSpy bytesWrittenSpy(socket, SIGNAL(bytesWritten(qint64)));
            QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
            written += countBytesWritten(bytesWrittenSpy);
            }
        QVERIFY(written == block.size());
        //Now data has been sent,check the if existing error
        QVERIFY(errorSpy.isEmpty());

        connectionSpy.removeFirst();
        loopCount++;
        }
    QTest::qWait(1500);//give some time to client to finish
    server.close();
    }
QTEST_MAIN(tst_QLlcpServer);

#include "tst_qllcpserver.moc"

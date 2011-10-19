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

#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>

#include <qllcpsocket.h>
#include <qnearfieldmanager.h>
#include <qnearfieldtarget.h>
#include "qnfctestcommon.h"
#include "qnfctestutil.h"

QTNFC_USE_NAMESPACE

Q_DECLARE_METATYPE(QNearFieldTarget*)
Q_DECLARE_METATYPE(QLlcpSocket::SocketState)
Q_DECLARE_METATYPE(QLlcpSocket::SocketError)

QString TestUri("urn:nfc:xsn:nokia:symbiantest");
static qint64 countBytesWritten(QSignalSpy& bytesWrittenSpy)
    {
    qint64 ret = 0;
    for(int i = 0; i < bytesWrittenSpy.count(); i++)
        {
        ret+=bytesWrittenSpy[i].at(0).value<qint64>();
        }
    return ret;
    }
class tst_qllcpsockettype2 : public QObject
{
    Q_OBJECT

public:
    tst_qllcpsockettype2();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    // basic acceptance testcases
    void echo();
    void echo_data();
    void echo_wait();
    void echo_wait_data();
    void api_coverage();

    void multipleWrite();
    void multipleWrite_data();
    // advanced testcases
    void waitReadyReadInSlot();
    void deleteSocketWhenInUse();
    void multiSocketToOneServer();
    // nagetive testcases
    void connectTest();
    void negTestCase1();
    void negTestCase2();
    void negTestCase3();

    void sendEmptyData();

 private:
    QNearFieldTarget *m_target; // not own
};

tst_qllcpsockettype2::tst_qllcpsockettype2()
{
    qRegisterMetaType<QNearFieldTarget*>("QNearFieldTarget*");
    qRegisterMetaType<QLlcpSocket::SocketError>("QLlcpSocket::SocketError");
    qRegisterMetaType<QLlcpSocket::SocketState>("QLlcpSocket::SocketState");
}

void tst_qllcpsockettype2::initTestCase()
{
    qDebug()<<"tst_qllcpsockettype2::initTestCase() Begin";
    QString message("Please touch a NFC device with llcp client enabled");
    QNearFieldManager nfcManager;
    QSignalSpy targetDetectedSpy(&nfcManager, SIGNAL(targetDetected(QNearFieldTarget*)));
    nfcManager.startTargetDetection(QNearFieldTarget::NfcForumDevice);

    QNfcTestUtil::ShowAutoMsg(message, &targetDetectedSpy, 1);
    QTRY_VERIFY(!targetDetectedSpy.isEmpty());

    m_target = targetDetectedSpy.at(targetDetectedSpy.count() - 1).at(0).value<QNearFieldTarget*>();
    QVERIFY(m_target != NULL);
    QVERIFY(m_target->accessMethods() & QNearFieldTarget::LlcpAccess);
    QVERIFY(m_target->uid() == QByteArray());
    QVERIFY(m_target->type() == QNearFieldTarget::NfcForumDevice);
    qDebug()<<"tst_qllcpsockettype2::initTestCase()   End";
}


void tst_qllcpsockettype2::cleanupTestCase()
{
}


/*!
 Description: Unit test for NFC LLCP socket async functions

 TestScenario:
               1. Echo client will connect to the Echo server
               2. Echo client will send the "echo" message to the server
               3. Echo client will receive the same message echoed from the server
               4. Echo client will disconnect the connection.

 TestExpectedResults:
               2. The message has be sent to server.
               3. The echoed message has been received from server.
               4. Connection disconnected and NO error signals emitted.

 CounterPart test: tst_QLlcpServer::newConnection() or newConnection_wait()
*/
void tst_qllcpsockettype2::echo()
{
    QFETCH(QString, uri);
    QFETCH(QString, echo);

    QString message("echo test");

    QNfcTestUtil::ShowAutoMsg(message);
    QLlcpSocket socket(this);
    QSignalSpy connectedSpy(&socket, SIGNAL(connected()));
    QSignalSpy errorSpy(&socket, SIGNAL(error(QLlcpSocket::SocketError)));
    QSignalSpy readyReadSpy(&socket, SIGNAL(readyRead()));

    QSignalSpy bytesWrittenSpy(&socket, SIGNAL(bytesWritten(qint64)));

    socket.connectToService(m_target, uri);

    // coverage add: WaitForBytesWritten will fail when connecting
    const int secs = 1 * 1000;
    bool waitRet = socket.waitForBytesWritten(secs);
    QVERIFY( waitRet == false);

    QTRY_VERIFY(!connectedSpy.isEmpty());

    // coverage add: connect to Service again when already connected will cause fail
    socket.connectToService(m_target, uri);
    QTRY_VERIFY(!errorSpy.isEmpty());
    errorSpy.clear();

    //Send data to server
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (quint16)0;
    out << echo;

    qDebug("Client-- write quint16 length = %d", sizeof(quint16));
    qDebug("Client-- write echo string = %s", qPrintable(echo));
    qDebug("Client-- write echo string length= %d", echo.length());
    qDebug("Client-- write data length = %d", block.length());
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    qint64 val = socket.write(block);
    QVERIFY( val == 0);

    QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
    qint64 written = countBytesWritten(bytesWrittenSpy);

    qDebug()<<"bytesWritten signal return value = "<<written;
    while (written < block.size())
        {
        QSignalSpy bytesWrittenSpy(&socket, SIGNAL(bytesWritten(qint64)));
        QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
        qint64 w = countBytesWritten(bytesWrittenSpy);
        qDebug()<<"got bytesWritten signal = "<<w;
        written += w;
        }
    qDebug()<<"Overall bytesWritten = "<<written;
    qDebug()<<"Overall block size = "<<block.size();
    QTRY_VERIFY(written == block.size());
    //Get the echoed data from server
    QTRY_VERIFY(!readyReadSpy.isEmpty());
    quint16 blockSize = 0;
    QDataStream in(&socket);
    in.setVersion(QDataStream::Qt_4_6);
    while (socket.bytesAvailable() < (int)sizeof(quint16)){
        QSignalSpy readyRead(&socket, SIGNAL(readyRead()));
        QTRY_VERIFY(!readyRead.isEmpty());
    }
    in >> blockSize;
    qDebug() << "Client-- read blockSize=" << blockSize;
    while (socket.bytesAvailable() < blockSize){
        QSignalSpy readyRead(&socket, SIGNAL(readyRead()));
        QTRY_VERIFY(!readyRead.isEmpty());
    }
    QString echoed;
    in >> echoed;

    qDebug() << "Client-- read echoed string =" << echoed;
    //test the echoed string is same as the original string
    QVERIFY(echo == echoed);

    socket.disconnectFromService();

    //Now data has been sent,check the if existing error
    QVERIFY(errorSpy.isEmpty());
}

void tst_qllcpsockettype2::echo_data()
{
    QTest::addColumn<QString>("uri");
    QTest::addColumn<QString>("echo");
    QTest::newRow("0") << TestUri
            << "echo";
    QString longStr4k;
    for (int i = 0; i < 4000; i++)
        longStr4k.append((char)(i%26 + 'a'));
    QTest::newRow("1") << TestUri << longStr4k;
}

/*!
 Description: Unit test for NFC LLCP socket sync(waitXXX) functions

 TestScenario: 1. Touch a NFC device with LLCP Echo service actived
               2. Echo client will connect to the Echo server
               3. Echo client will send the "echo" message to the server
               4. Echo client will receive the same message echoed from the server
               5. Echo client will disconnect the connection.

 TestExpectedResults:
               1. The connection successfully set up.
               2. The message has be sent to server.
               3. The echoed message has been received from server.
               4. Connection disconnected and NO error signals emitted.

 CounterPart test: tst_QLlcpServer::newConnection() or newConnection_wait()
*/
void tst_qllcpsockettype2::echo_wait()
    {
    QFETCH(QString, uri);
    QFETCH(QString, echo);

    QString message("echo_wait test");

    QNfcTestUtil::ShowAutoMsg(message);
    QLlcpSocket socket(this);
    const int Timeout = 10 * 1000;

    socket.connectToService(m_target, uri);

    bool ret = socket.waitForConnected(Timeout);
    QVERIFY(ret);

    QSignalSpy errorSpy(&socket, SIGNAL(error(QLlcpSocket::SocketError)));

    //Send data to server
    QSignalSpy bytesWrittenSpy(&socket, SIGNAL(bytesWritten(qint64)));

    //Send data to server
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (quint16)0;
    out << echo;

    qDebug("Client-- write quint16 length = %d", sizeof(quint16));
    qDebug("Client-- write echo string = %s", qPrintable(echo));
    qDebug("Client-- write echo string length= %d", echo.length());
    qDebug("Client-- write data length = %d", block.length());
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    qint64 val = socket.write(block);
    QVERIFY( val == 0);

    ret = socket.waitForBytesWritten(Timeout);
    QVERIFY(ret);

    QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
    qint64 written = countBytesWritten(bytesWrittenSpy);

    while (written < block.size())
        {
        QSignalSpy bytesWrittenSpy(&socket, SIGNAL(bytesWritten(qint64)));
        ret = socket.waitForBytesWritten(Timeout);
        QVERIFY(ret);
        QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
        written += countBytesWritten(bytesWrittenSpy);
        }
    QVERIFY(written == block.size());
    //Get the echoed data from server
    quint16 blockSize = 0;
    QDataStream in(&socket);
    in.setVersion(QDataStream::Qt_4_6);
    while (socket.bytesAvailable() < (int)sizeof(quint16)){
        ret = socket.waitForReadyRead(Timeout);
        QVERIFY(ret);
    }
    in >> blockSize;
    qDebug() << "Client-- read blockSize=" << blockSize;
    while (socket.bytesAvailable() < blockSize){
        ret = socket.waitForReadyRead(Timeout);
        QVERIFY(ret);
    }
    QString echoed;
    in >> echoed;
    qDebug() << "Client-- read echoed string =" << echoed;
    //test the echoed string is same as the original string
    QVERIFY(echo == echoed);

    socket.disconnectFromService();
    ret = socket.waitForDisconnected(Timeout);
    QVERIFY(ret);
    //Now data has been sent,check the if existing error
    QVERIFY(errorSpy.isEmpty());
    }

void tst_qllcpsockettype2::echo_wait_data()
{
    QTest::addColumn<QString>("uri");
    QTest::addColumn<QString>("echo");
    QTest::newRow("0") << TestUri
            << "echo";
    QString longStr4k;
    for (int i = 0; i < 4000; i++)
        longStr4k.append((char)(i%26 + 'a'));
    QTest::newRow("1") << TestUri << longStr4k;
}



/*!
 Description: LLCP Socket API & State Machine test (ReadDatagram tested in server side)
 TestScenario:
     Covered API: state(), error(), writeDatagram(const char *data, qint64 size);
                  writeDatagram(const QByteArray &datagram);

 CounterPart test: tst_QLlcpServer::api_coverage()
 */
void tst_qllcpsockettype2::api_coverage()
{

    QString message("api_coverage test");

    QNfcTestUtil::ShowAutoMsg(message);

    QLlcpSocket socket(this);
    QCOMPARE(socket.state(), QLlcpSocket::UnconnectedState);
    QSignalSpy stateChangedSpy(&socket, SIGNAL(stateChanged(QLlcpSocket::SocketState)));

    QSignalSpy connectedSpy(&socket, SIGNAL(connected()));
    socket.connectToService(m_target, TestUri);
    QTRY_VERIFY(!connectedSpy.isEmpty());

    QVERIFY(stateChangedSpy.count() == 2);
    QLlcpSocket::SocketState  state1 = stateChangedSpy.at(0).at(0).value<QLlcpSocket::SocketState>();
    QLlcpSocket::SocketState  state2 = stateChangedSpy.at(1).at(0).value<QLlcpSocket::SocketState>();
    QCOMPARE(state1, QLlcpSocket::ConnectingState);
    QCOMPARE(state2, QLlcpSocket::ConnectedState);

    QSignalSpy bytesWrittenSpy(&socket, SIGNAL(bytesWritten(qint64)));
    message = "Connection oriented write test string";
    QByteArray array;
    array.append(message);
    qint64 ret = socket.writeDatagram(array.constData(),array.size());
    QVERIFY( ret == 0);

    QTRY_VERIFY(bytesWrittenSpy.count() == 1);
    qint64 w = bytesWrittenSpy.first().at(0).value<qint64>();
    QVERIFY(w == array.size());
    stateChangedSpy.clear();
    socket.disconnectFromService();
    QVERIFY(stateChangedSpy.count() == 1);
    state1 = stateChangedSpy.at(0).at(0).value<QLlcpSocket::SocketState>();
    QCOMPARE(state1, QLlcpSocket::UnconnectedState);

    QCOMPARE(socket.error(),QLlcpSocket::UnknownSocketError);

}


/*!
 Description:  connect to Service testcases
 TestScenario:  1) double connect cause failure
                2) disconnect the connecting socket should not cause failure
                3) readDatagram after disconnection will cause failure.

 CounterPart test: tst_QLlcpServer::negTestCase1()
 */
void tst_qllcpsockettype2::connectTest()
{
    QString message("connectTest");

    QNfcTestUtil::ShowAutoMsg(message);

    QLlcpSocket socket(this);
    QCOMPARE(socket.state(), QLlcpSocket::UnconnectedState);
    QSignalSpy errorSpy(&socket, SIGNAL(error(QLlcpSocket::SocketError)));

    QSignalSpy connectedSpy(&socket, SIGNAL(connected()));
    socket.connectToService(m_target, TestUri);

    //connect to the service again when previous one is connecting
    socket.connectToService(m_target, TestUri);
    QTRY_VERIFY(!errorSpy.isEmpty());

    errorSpy.clear();
    // make sure it is still connecting
    if(connectedSpy.isEmpty())
    {
       socket.disconnectFromService();
    }
    QTRY_VERIFY(errorSpy.isEmpty());

    //double disconnect should not cause error
    socket.disconnectFromService();
    QTRY_VERIFY(errorSpy.isEmpty());

    // readDatagram must be called before successul connection to server
    QByteArray datagram;
    datagram.resize(127);
    qint64 ret = socket.readDatagram(datagram.data(), datagram.size());
    QVERIFY(ret == -1);
}

/*!
 Description: Unit test for NFC LLCP socket write several times

 TestScenario:
               1. Echo client will connect to the Echo server
               2. Echo client will send the "echo" message to the server
               3. Echo client will receive the same message echoed from the server
               4. Echo client will disconnect the connection.

 TestExpectedResults:
               2. The message has be sent to server.
               3. The echoed message has been received from server.
               4. Connection disconnected and NO error signals emitted.

 CounterPart test: tst_QLlcpServer::newConnection() or newConnection_wait()
*/
void tst_qllcpsockettype2::multipleWrite()
    {
    QFETCH(QString, uri);
    QFETCH(QString, echo);
    QString message("multipleWrite test");
    QNfcTestUtil::ShowAutoMsg(message);

    QLlcpSocket socket(this);

    QSignalSpy connectedSpy(&socket, SIGNAL(connected()));
    socket.connectToService(m_target, uri);
    QTRY_VERIFY(!connectedSpy.isEmpty());

    QSignalSpy bytesWrittenSpy(&socket, SIGNAL(bytesWritten(qint64)));

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (quint16)0;
    out << echo;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    qint64 ret = socket.writeDatagram(block.constData(), block.size()/2);
    QVERIFY( ret == 0);
    ret = socket.writeDatagram(block.constData() + block.size()/2, block.size() - block.size()/2);
    QVERIFY( ret == 0);

    QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
    qint64 written = countBytesWritten(bytesWrittenSpy);;

    qDebug()<<"bytesWritten signal return value = "<<written;
    while (written < block.size())
        {
        QSignalSpy bytesWrittenSpy(&socket, SIGNAL(bytesWritten(qint64)));
        QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
        qint64 w = countBytesWritten(bytesWrittenSpy);
        qDebug()<<"got bytesWritten signal = "<<w;
        written += w;
        }
    qDebug()<<"Overall bytesWritten = "<<written;
    qDebug()<<"Overall block size = "<<block.size();
    QCOMPARE(written, (qint64)block.size());

    //Get the echoed data from server
    const int Timeout = 10 * 1000;
    quint16 blockSize = 0;
    QDataStream in(&socket);
    in.setVersion(QDataStream::Qt_4_6);
    while (socket.bytesAvailable() < (int)sizeof(quint16)){
        ret = socket.waitForReadyRead(Timeout);
        QVERIFY(ret);
    }
    in >> blockSize;
    qDebug() << "Client-- read blockSize=" << blockSize;
    while (socket.bytesAvailable() < blockSize){
        ret = socket.waitForReadyRead(Timeout);
        QVERIFY(ret);
    }
    QString echoed;
    in >> echoed;
    qDebug() << "Client-- read echoed string =" << echoed;
    //test the echoed string is same as the original string
    QCOMPARE(echoed, echo);
    socket.disconnectFromService();
    ret = socket.waitForDisconnected(Timeout);
    QVERIFY(ret);
    }

void tst_qllcpsockettype2::multipleWrite_data()
{
    QTest::addColumn<QString>("uri");
    QTest::addColumn<QString>("echo");
    QTest::newRow("0") << TestUri
            << "1234567890";
    QString longStr4k;
    for (int i = 0; i < 4000; i++)
        longStr4k.append((char)(i%26 + 'a'));
    QTest::newRow("1") << TestUri << longStr4k;
}
/*!
 Description:  coverage test - cover sender DoCancel() method
 CounterPart test: tst_QLlcpServer::negTestCase1()
*/
void tst_qllcpsockettype2::negTestCase1()
{
    QString message("negTestCase1 test");
    QNfcTestUtil::ShowAutoMsg(message);

    QLlcpSocket socket(this);

    QSignalSpy connectedSpy(&socket, SIGNAL(connected()));
    socket.connectToService(m_target, TestUri);
    QTRY_VERIFY(!connectedSpy.isEmpty());

    QSignalSpy bytesWrittenSpy(&socket, SIGNAL(bytesWritten(qint64)));
    message = "1234567890";
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (quint16)0;
    out << message;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    qint64 ret = socket.writeDatagram(block.constData(), block.size());
    QVERIFY( ret != -1);
    //cover SenderAO DoCancel() method
}

/*!
 Description: readDatagram/writeDatagram negative test - read/write without connectToService

 CounterPart test: No need counterpart in server side
*/
void tst_qllcpsockettype2::negTestCase2()
{
    QLlcpSocket socket(this);
    QByteArray datagram("Connection oriented negTestCase2");
    qint64 ret = socket.writeDatagram(datagram);
    QTRY_VERIFY(ret == -1);
    ret = socket.readDatagram(datagram.data(),datagram.size());
    QTRY_VERIFY(ret == -1);
}


/*!
 Description: negative testcase II - invalid usage of connection-less API
 CounterPart test: tst_QLlcpServer::negTestCase1()
*/
void tst_qllcpsockettype2::negTestCase3()
{
    QLlcpSocket socket(this);

    QString message("negTestCase3 test");
    QNfcTestUtil::ShowAutoMsg(message);

    QSignalSpy errorSpy(&socket, SIGNAL(error(QLlcpSocket::SocketError)));
    socket.connectToService(m_target, TestUri);

    bool ret = socket.bind(35);
    QVERIFY(ret == false);

    ret = socket.hasPendingDatagrams();
    QVERIFY(ret == false);

    qint64 size = socket.pendingDatagramSize();
    QVERIFY(size == -1);

    message = "Oops, Invalid usage for writeDatagram";
    const char* data = (const char *) message.data();
    qint64 strSize = message.size();
    size = socket.writeDatagram(data,strSize,m_target, 35);
    QVERIFY(size == -1);
    QByteArray datagram(data);
    size = socket.writeDatagram(datagram,m_target, 35);
    QVERIFY(size == -1);

    quint8 port = 35;
    size = socket.readDatagram(datagram.data(),datagram.size(),&m_target, &port);
    QVERIFY(size == -1);

}
/*!
 Description: Send empty data to server
 CounterPart test: tst_QLlcpServer::negTestCase1()
*/
void tst_qllcpsockettype2::sendEmptyData()
    {

    QString message("sendEmptyData test");

    QNfcTestUtil::ShowAutoMsg(message);
    QLlcpSocket socket(this);
    QSignalSpy connectedSpy(&socket, SIGNAL(connected()));
    QSignalSpy errorSpy(&socket, SIGNAL(error(QLlcpSocket::SocketError)));
    QSignalSpy readyReadSpy(&socket, SIGNAL(readyRead()));

    QSignalSpy bytesWrittenSpy(&socket, SIGNAL(bytesWritten(qint64)));

    socket.connectToService(m_target, TestUri);

    QTRY_VERIFY(!connectedSpy.isEmpty());

    //Send data to server
    QByteArray block;

    qDebug("Client-- write data length = %d", block.length());

    qint64 val = socket.write(block);
    QVERIFY(val == -1);

    socket.disconnectFromService();

    //Now data has been sent,check the if existing error
    QVERIFY(errorSpy.isEmpty());
    }
class ReadyReadSlot : public QObject
{
    Q_OBJECT
public:
    ReadyReadSlot(QLlcpSocket& s): m_socket(s),m_signalCount(0)
        {
        connect(&m_socket,SIGNAL(readyRead()),this,SLOT(gotReadyRead()));
        }
private slots:
    void gotReadyRead()
        {
        m_signalCount++;
        qDebug()<<"Got ReadyRead() signal number = "<<m_signalCount;
        const int Timeout = 50;//3* 1000 seems too long, nfc server will panic spray signal
        bool ret = m_socket.waitForReadyRead(Timeout);
        if (!ret)
            {
            qDebug()<<"WaitForReadyRead() in slot of ReadyRead signal return false";
            }
        else
            {
            qDebug()<<"WaitForReadyRead() in slot of ReadyRead signal return true";
            }
        }
private:
    QLlcpSocket& m_socket;
    int m_signalCount;
};
/*!
 Description: Test WaitForReadyRead() in slot of
 ReadyRead signal, make sure the signal will not
 be emitted twice in the slot function.
 CounterPart test: tst_QLlcpServer::newConnection()
*/
void tst_qllcpsockettype2::waitReadyReadInSlot()
{
    QString message("waitReadyReadInSlot test");

    QNfcTestUtil::ShowAutoMsg(message);
    QLlcpSocket socket(this);
    QSignalSpy connectedSpy(&socket, SIGNAL(connected()));
    QSignalSpy errorSpy(&socket, SIGNAL(error(QLlcpSocket::SocketError)));

    QSignalSpy readyReadSpy(&socket, SIGNAL(readyRead()));
    ReadyReadSlot slot(socket);

    QSignalSpy bytesWrittenSpy(&socket, SIGNAL(bytesWritten(qint64)));

    socket.connectToService(m_target, TestUri);

    QTRY_VERIFY(!connectedSpy.isEmpty());

    QString echo;
    for (int i = 0; i < 2000; i++)
        echo.append((char)(i%26 + 'a'));
    //Send data to server
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (quint16)0;
    out << echo;

    qDebug("Client-- write quint16 length = %d", sizeof(quint16));
    qDebug("Client-- write echo string = %s", qPrintable(echo));
    qDebug("Client-- write echo string length= %d", echo.length());
    qDebug("Client-- write data length = %d", block.length());
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    qint64 val = socket.write(block);
    QVERIFY( val == 0);

    QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
    qint64 written = countBytesWritten(bytesWrittenSpy);

    qDebug()<<"bytesWritten signal return value = "<<written;
    while (written < block.size())
        {
        QSignalSpy bytesWrittenSpy(&socket, SIGNAL(bytesWritten(qint64)));
        QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
        qint64 w = countBytesWritten(bytesWrittenSpy);
        qDebug()<<"got bytesWritten signal = "<<w;
        written += w;
        }
    qDebug()<<"Overall bytesWritten = "<<written;
    qDebug()<<"Overall block size = "<<block.size();
    QTRY_VERIFY(written == block.size());
    //Get the echoed data from server
    QTRY_VERIFY(!readyReadSpy.isEmpty());
    quint16 blockSize = 0;
    QDataStream in(&socket);
    in.setVersion(QDataStream::Qt_4_6);
    while (socket.bytesAvailable() < (int)sizeof(quint16)){
        QSignalSpy readyRead(&socket, SIGNAL(readyRead()));
        QTRY_VERIFY(!readyRead.isEmpty());
    }
    in >> blockSize;
    qDebug() << "Client-- read blockSize=" << blockSize;
    while (socket.bytesAvailable() < blockSize){
        QSignalSpy readyRead(&socket, SIGNAL(readyRead()));
        QTRY_VERIFY(!readyRead.isEmpty());
    }
    QString echoed;
    in >> echoed;

    qDebug() << "Client-- read echoed string =" << echoed;
    //test the echoed string is same as the original string
    QVERIFY(echo == echoed);

    socket.disconnectFromService();

    //Now data has been sent,check the if existing error
    QVERIFY(errorSpy.isEmpty());
}

class BytesWrittenSlot : public QObject
{
    Q_OBJECT
public:
    BytesWrittenSlot(QLlcpSocket* s): m_socket(s)
        {
        connect(m_socket,SIGNAL(bytesWritten(qint64)),this,SLOT(gotBytesWritten(qint64)));
        }
private slots:
    void gotBytesWritten(qint64 w)
        {
        qDebug()<<"In BytesWrittenSlot: Delete the socket when still alive...";
        delete m_socket;
        }
private:
    QLlcpSocket* m_socket;
};
/*!
 Description: Add a case to test delete the socket in the slot when the transmission is still alive.
 CounterPart test: tst_QLlcpServer::negTestCase1()
*/
void tst_qllcpsockettype2::deleteSocketWhenInUse()
    {
    QString message("deleteSocketWhenInUse test");

    QNfcTestUtil::ShowAutoMsg(message);
    QLlcpSocket* socket = new QLlcpSocket;
    QSignalSpy connectedSpy(socket, SIGNAL(connected()));
    QSignalSpy errorSpy(socket, SIGNAL(error(QLlcpSocket::SocketError)));

    BytesWrittenSlot slot(socket);
    QSignalSpy bytesWrittenSpy(socket, SIGNAL(bytesWritten(qint64)));

    socket->connectToService(m_target, TestUri);

    QTRY_VERIFY(!connectedSpy.isEmpty());

    QString echo;
    for (int i = 0; i < 2000; i++)
        echo.append((char)(i%26 + 'a'));
    //Send data to server
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (quint16)0;
    out << echo;

    qDebug("Client-- write quint16 length = %d", sizeof(quint16));
    qDebug("Client-- write echo string = %s", qPrintable(echo));
    qDebug("Client-- write echo string length= %d", echo.length());
    qDebug("Client-- write data length = %d", block.length());
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    qint64 val = socket->write(block);
    QVERIFY( val == 0);
    QTest::qWait(5 * 1000);//give some time to wait bytesWritten signal
    QVERIFY(errorSpy.isEmpty());
    }
/*!
 Description: Add a server in one device, then two
 client socket in another device to connect the server at same time
 CounterPart test: tst_QLlcpServer::multiConnection()
*/
void tst_qllcpsockettype2::multiSocketToOneServer()
    {
    QString message("multiSocketToOneServer test");

    QNfcTestUtil::ShowAutoMsg(message);
    QLlcpSocket* socket1 = new QLlcpSocket;
    QLlcpSocket* socket2 = new QLlcpSocket;
    QSignalSpy connectedSpy1(socket1, SIGNAL(connected()));
    QSignalSpy errorSpy1(socket1, SIGNAL(error(QLlcpSocket::SocketError)));
    QSignalSpy readyReadSpy1(socket1, SIGNAL(readyRead()));
    QSignalSpy bytesWrittenSpy1(socket1, SIGNAL(bytesWritten(qint64)));

    QSignalSpy connectedSpy2(socket2, SIGNAL(connected()));
    QSignalSpy errorSpy2(socket2, SIGNAL(error(QLlcpSocket::SocketError)));
    QSignalSpy readyReadSpy2(socket2, SIGNAL(readyRead()));
    QSignalSpy bytesWrittenSpy2(socket2, SIGNAL(bytesWritten(qint64)));

    socket1->connectToService(m_target, TestUri);
    socket2->connectToService(m_target, TestUri);

    //test connect when some socket still processing data
    QLlcpSocket* socket3 = new QLlcpSocket;

    QTRY_VERIFY(!connectedSpy1.isEmpty() || !connectedSpy2.isEmpty());
    if (!connectedSpy1.isEmpty())
        {
        qDebug() << "socket 1 connected";
        QString echo1 = "aaaaaaa";
        //Send data to server
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_6);
        out << (quint16)0;
        out << echo1;

        qDebug("Client-- write quint16 length = %d", sizeof(quint16));
        qDebug("Client-- write echo string = %s", qPrintable(echo1));
        qDebug("Client-- write echo string length= %d", echo1.length());
        qDebug("Client-- write data length = %d", block.length());
        out.device()->seek(0);
        out << (quint16)(block.size() - sizeof(quint16));

        qint64 val = socket1->write(block);
        QVERIFY( val == 0);

        QTRY_VERIFY(!bytesWrittenSpy1.isEmpty());
        qint64 written = countBytesWritten(bytesWrittenSpy1);

        socket3->connectToService(m_target, TestUri);

        qDebug()<<"bytesWritten signal return value = "<<written;
        while (written < block.size())
            {
            QSignalSpy bytesWrittenSpy(socket1, SIGNAL(bytesWritten(qint64)));
            QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
            qint64 w = countBytesWritten(bytesWrittenSpy);
            qDebug()<<"got bytesWritten signal = "<<w;
            written += w;
            }
        qDebug()<<"Overall bytesWritten = "<<written;
        qDebug()<<"Overall block size = "<<block.size();
        QTRY_VERIFY(written == block.size());
        //Get the echoed data from server
        QTRY_VERIFY(!readyReadSpy1.isEmpty());
        quint16 blockSize = 0;
        QDataStream in(socket1);
        in.setVersion(QDataStream::Qt_4_6);
        while (socket1->bytesAvailable() < (int)sizeof(quint16)){
            QSignalSpy readyRead(socket1, SIGNAL(readyRead()));
            QTRY_VERIFY(!readyRead.isEmpty());
        }
        in >> blockSize;
        qDebug() << "Client-- read blockSize=" << blockSize;
        while (socket1->bytesAvailable() < blockSize){
            QSignalSpy readyRead(socket1, SIGNAL(readyRead()));
            QTRY_VERIFY(!readyRead.isEmpty());
        }
        QString echoed;
        in >> echoed;

        qDebug() << "Client-- read echoed string =" << echoed;
        //test the echoed string is same as the original string
        QCOMPARE(echo1, echoed);

        //
        //try to handle the socket2
        //
        QTRY_VERIFY(!connectedSpy2.isEmpty());
        qDebug() << "socket 2 connected";
        QString echo2 = "bbbbbbb";
        //Send data to server
        QByteArray block2;
        QDataStream out2(&block2, QIODevice::WriteOnly);
        out2.setVersion(QDataStream::Qt_4_6);
        out2 << (quint16)0;
        out2 << echo2;

        qDebug("Client-- write quint16 length = %d", sizeof(quint16));
        qDebug("Client-- write echo string = %s", qPrintable(echo2));
        qDebug("Client-- write echo string length= %d", echo2.length());
        qDebug("Client-- write data length = %d", block2.length());
        out2.device()->seek(0);
        out2 << (quint16)(block2.size() - sizeof(quint16));

        qint64 val2 = socket2->write(block2);
        QVERIFY( val2 == 0);

        QTRY_VERIFY(!bytesWrittenSpy2.isEmpty());
        qint64 written2 = countBytesWritten(bytesWrittenSpy2);

        qDebug()<<"bytesWritten signal return value = "<<written2;
        while (written2 < block2.size())
            {
            QSignalSpy bytesWrittenSpy(socket2, SIGNAL(bytesWritten(qint64)));
            QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
            qint64 w = countBytesWritten(bytesWrittenSpy);
            qDebug()<<"got bytesWritten signal = "<<w;
            written2 += w;
            }
        qDebug()<<"Overall bytesWritten = "<<written2;
        qDebug()<<"Overall block size = "<<block2.size();
        QTRY_VERIFY(written2 == block2.size());
        //Get the echoed data from server
        QTRY_VERIFY(!readyReadSpy2.isEmpty());
        quint16 blockSize2 = 0;
        QDataStream in2(socket2);
        in2.setVersion(QDataStream::Qt_4_6);
        while (socket2->bytesAvailable() < (int)sizeof(quint16)){
            QSignalSpy readyRead(socket2, SIGNAL(readyRead()));
            QTRY_VERIFY(!readyRead.isEmpty());
        }
        in2 >> blockSize2;
        qDebug() << "Client-- read blockSize=" << blockSize2;
        while (socket2->bytesAvailable() < blockSize2){
            QSignalSpy readyRead(socket2, SIGNAL(readyRead()));
            QTRY_VERIFY(!readyRead.isEmpty());
        }
        QString echoed2;
        in2 >> echoed2;

        qDebug() << "Client-- read echoed string =" << echoed;
        //test the echoed string is same as the original string
        QCOMPARE(echo2, echoed2);
        }
    else
        {

        qDebug() << "socket 2 connected";
        QString echo2 = "bbbbbbb";
        //Send data to server
        QByteArray block2;
        QDataStream out2(&block2, QIODevice::WriteOnly);
        out2.setVersion(QDataStream::Qt_4_6);
        out2 << (quint16)0;
        out2 << echo2;

        qDebug("Client-- write quint16 length = %d", sizeof(quint16));
        qDebug("Client-- write echo string = %s", qPrintable(echo2));
        qDebug("Client-- write echo string length= %d", echo2.length());
        qDebug("Client-- write data length = %d", block2.length());
        out2.device()->seek(0);
        out2 << (quint16)(block2.size() - sizeof(quint16));

        qint64 val2 = socket2->write(block2);
        QVERIFY( val2 == 0);

        QTRY_VERIFY(!bytesWrittenSpy2.isEmpty());
        qint64 written2 = countBytesWritten(bytesWrittenSpy2);

        socket3->connectToService(m_target, TestUri);

        qDebug()<<"bytesWritten signal return value = "<<written2;
        while (written2 < block2.size())
            {
            QSignalSpy bytesWrittenSpy(socket2, SIGNAL(bytesWritten(qint64)));
            QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
            qint64 w = countBytesWritten(bytesWrittenSpy);
            qDebug()<<"got bytesWritten signal = "<<w;
            written2 += w;
            }
        qDebug()<<"Overall bytesWritten = "<<written2;
        qDebug()<<"Overall block size = "<<block2.size();
        QTRY_VERIFY(written2 == block2.size());
        //Get the echoed data from server
        QTRY_VERIFY(!readyReadSpy2.isEmpty());
        quint16 blockSize2 = 0;
        QDataStream in2(socket2);
        in2.setVersion(QDataStream::Qt_4_6);
        while (socket2->bytesAvailable() < (int)sizeof(quint16)){
            QSignalSpy readyRead(socket2, SIGNAL(readyRead()));
            QTRY_VERIFY(!readyRead.isEmpty());
        }
        in2 >> blockSize2;
        qDebug() << "Client-- read blockSize=" << blockSize2;
        while (socket2->bytesAvailable() < blockSize2){
            QSignalSpy readyRead(socket2, SIGNAL(readyRead()));
            QTRY_VERIFY(!readyRead.isEmpty());
        }
        QString echoed2;
        in2 >> echoed2;

        qDebug() << "Client-- read echoed string =" << echoed2;
        //test the echoed string is same as the original string
        QCOMPARE(echo2, echoed2);

        //
        //try to handle the socket1
        //
        QTRY_VERIFY(!connectedSpy1.isEmpty());
        qDebug() << "socket 1 connected";
        QString echo1 = "aaaaaaa";
        //Send data to server
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_6);
        out << (quint16)0;
        out << echo1;

        qDebug("Client-- write quint16 length = %d", sizeof(quint16));
        qDebug("Client-- write echo string = %s", qPrintable(echo1));
        qDebug("Client-- write echo string length= %d", echo1.length());
        qDebug("Client-- write data length = %d", block.length());
        out.device()->seek(0);
        out << (quint16)(block.size() - sizeof(quint16));

        qint64 val = socket1->write(block);
        QVERIFY( val == 0);

        QTRY_VERIFY(!bytesWrittenSpy1.isEmpty());
        qint64 written = countBytesWritten(bytesWrittenSpy1);

        qDebug()<<"bytesWritten signal return value = "<<written;
        while (written < block.size())
            {
            QSignalSpy bytesWrittenSpy(socket1, SIGNAL(bytesWritten(qint64)));
            QTRY_VERIFY(!bytesWrittenSpy.isEmpty());
            qint64 w = countBytesWritten(bytesWrittenSpy);
            qDebug()<<"got bytesWritten signal = "<<w;
            written += w;
            }
        qDebug()<<"Overall bytesWritten = "<<written;
        qDebug()<<"Overall block size = "<<block.size();
        QTRY_VERIFY(written == block.size());
        //Get the echoed data from server
        QTRY_VERIFY(!readyReadSpy1.isEmpty());
        quint16 blockSize = 0;
        QDataStream in(socket1);
        in.setVersion(QDataStream::Qt_4_6);
        while (socket1->bytesAvailable() < (int)sizeof(quint16)){
            QSignalSpy readyRead(socket1, SIGNAL(readyRead()));
            QTRY_VERIFY(!readyRead.isEmpty());
        }
        in >> blockSize;
        qDebug() << "Client-- read blockSize=" << blockSize;
        while (socket1->bytesAvailable() < blockSize){
            QSignalSpy readyRead(socket1, SIGNAL(readyRead()));
            QTRY_VERIFY(!readyRead.isEmpty());
        }
        QString echoed;
        in >> echoed;

        qDebug() << "Client-- read echoed string =" << echoed;
        //test the echoed string is same as the original string
        QCOMPARE(echo1, echoed);

        }
    QVERIFY(errorSpy1.isEmpty());
    QVERIFY(errorSpy2.isEmpty());
    delete socket1;
    delete socket2;
    delete socket3;
    }
QTEST_MAIN(tst_qllcpsockettype2);

#include "tst_qllcpsockettype2.moc"

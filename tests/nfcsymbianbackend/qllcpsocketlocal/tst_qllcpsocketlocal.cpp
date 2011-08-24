/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

Q_DECLARE_METATYPE(QNearFieldTarget*)
Q_DECLARE_METATYPE(QLlcpSocket::SocketError )
Q_DECLARE_METATYPE(QLlcpSocket::SocketState )

class tst_qllcpsocketlocal : public QObject
{
    Q_OBJECT

public:
    tst_qllcpsocketlocal();
private Q_SLOTS:


    void multipleClient();
    void echoClient();
    void echoClient_data();
    void testCase2();   //work with  tst_qllcpsocketremote testCase2
    void testCase3();
    void coverageTest1();
    //advanced test
    void deleteSocketWhenInUse();
    void waitBytesWrittenInSlot();
    void multiSocketToOneServer();
    //negtive test
    void negTestCase1();
    void negTestCase2();
    //void negTestCase3();
    //void negTestCase4();

    void initTestCase();
    void cleanupTest();

private:
    void writeMessage(QLlcpSocket& localSocket,QString& payLoad);
private:
     QNearFieldTarget *m_target; // not own
     quint8 m_port;
};

tst_qllcpsocketlocal::tst_qllcpsocketlocal()
{
    qRegisterMetaType<QNearFieldTarget *>("QNearFieldTarget*");
    qRegisterMetaType<QLlcpSocket::SocketError>("QLlcpSocket::SocketError");
    qRegisterMetaType<QLlcpSocket::SocketState>("QLlcpSocket::SocketState");
}

/*!
 Description: Init test case for NFC LLCP connection-less mode socket - local peer

 TestScenario:
     Touch a NFC device with LLCP connection-less service actived

 TestExpectedResults:
     Signal of target detected has been found.
*/
void tst_qllcpsocketlocal::initTestCase()
{
    qDebug()<<"tst_qllcpsocketlocal::initTestCase() Begin";
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

    m_port = 35;
    qDebug()<<"tst_qllcpsocketlocal::initTestCase() End";
}

/*!
 Description: Send the message and Receive the acknowledged identical message

 TestScenario:
           1. Local peer temps to read datagram without bind
           2. Local peer binds to the remote peer
           3. Local peer sends the "testcase1 string" message to the remote peer
           4. Local peer receives the above message sending from the remote peer

 TestExpectedResults:
           1. Local peer fails to read datagram without bind
           2. Local peer binds to local port successfully.
           3. The message has be sent to remote peer.
           4. The message has been received from remote peer.
*/
void tst_qllcpsocketlocal::echoClient()
{
    QFETCH(QString, echoPayload);
    QLlcpSocket localSocket;

    // STEP 1.  readDatagram must be called before bind
    QByteArray tmpForReadArray;
    tmpForReadArray.resize(127);
    qint64 ret = localSocket.readDatagram(tmpForReadArray.data(), tmpForReadArray.size());
    QVERIFY(ret == -1);

    QCOMPARE(localSocket.state(), QLlcpSocket::UnconnectedState);
    QSignalSpy stateChangedSpy(&localSocket, SIGNAL(stateChanged(QLlcpSocket::SocketState)));

    // STEP 2:  bind the local port for current socket
    QSignalSpy readyReadSpy(&localSocket, SIGNAL(readyRead()));
    bool retBool = localSocket.bind(m_port);
    QVERIFY(retBool);
    QVERIFY(!stateChangedSpy.isEmpty());
    QCOMPARE(localSocket.state(), QLlcpSocket::BoundState);

    // Wait remote part bind
    QString messageBox("Wait remote bind");
    QNfcTestUtil::ShowAutoMsg(messageBox);

    // STEP 3: Local peer sends the  message to the remote peer
    writeMessage(localSocket,echoPayload);
    QSignalSpy errorSpy(&localSocket, SIGNAL(error(QLlcpSocket::SocketState)));

    // STEP 4: Start read payload from server
    QString messageBox2("Wait remote send buffer");
    QNfcTestUtil::ShowAutoMsg(messageBox2, &readyReadSpy);

    QByteArray inPayload;
    while(localSocket.hasPendingDatagrams()) {
        QByteArray tempBuffer;
        tempBuffer.resize(localSocket.pendingDatagramSize());
        quint8 remotePort = 0;
        QSignalSpy readyRead(&localSocket, SIGNAL(readyRead()));
        qint64 readSize = localSocket.readDatagram(tempBuffer.data()
                                                   , tempBuffer.size()
                                                   , &m_target, &remotePort);
        QVERIFY(readSize != -1);
        QVERIFY(remotePort > 0);
        inPayload.append(tempBuffer);
        qDebug() << "Client-- read inPayload size=" << inPayload.size();
        qDebug() << "Client-- read remotePort=" << remotePort;
        if (inPayload.size() >= (int)sizeof(quint16)) {
            break;
        }
        else
        {
            qDebug() << "Client-- not enough header";
            QTRY_VERIFY(!readyRead.isEmpty());
        }
    }

    QDataStream in(inPayload);
    in.setVersion(QDataStream::Qt_4_6);
    quint16 headerSize = 0; // size of real echo payload
    in >> headerSize;
    qDebug() << "Client-- read headerSize=" << headerSize;
    while (inPayload.size() < headerSize + (int)sizeof(quint16)){
        QSignalSpy readyRead(&localSocket, SIGNAL(readyRead()));
        QTRY_VERIFY(!readyRead.isEmpty());
        if(localSocket.hasPendingDatagrams()) {
            QByteArray tempBuffer;
            tempBuffer.resize(localSocket.pendingDatagramSize());
            quint8 remotePort = 0;
            qint64 readSize = localSocket.readDatagram(tempBuffer.data()
                                                       , tempBuffer.size()
                                                       , &m_target, &remotePort);
            QVERIFY(readSize != -1);
            QVERIFY(remotePort > 0);
            inPayload.append(tempBuffer);
            qDebug() << "Client-- read long blocksize=" << inPayload.size();
        }
    }

    QDataStream in2(inPayload);
    in2.setVersion(QDataStream::Qt_4_6);
    in2 >> headerSize;
    QString inEchoPayload;
    in2 >> inEchoPayload;
    qDebug() << "Client-- in echo string len:" << inEchoPayload.length();
    qDebug("Client-- in echo string = %s", qPrintable(inEchoPayload));
    //test the echoed string is same as the original string
    QVERIFY(echoPayload == inEchoPayload);
    // make sure the no error signal emitted
    QVERIFY(errorSpy.isEmpty());
}

void tst_qllcpsocketlocal::echoClient_data()
{
    QTest::addColumn<QString>("echoPayload");
    QTest::newRow("0") << "test payload";
    QString longStr4k;
    for (int i = 0; i < 4000; i++)
        longStr4k.append((char)(i%26 + 'a'));
    QTest::newRow("1") << longStr4k;
}

void tst_qllcpsocketlocal::writeMessage(
                          QLlcpSocket& localSocket,
                          QString& payLoad)
{
    // STEP 2: Local peer sends the  message to the remote peer
    QSignalSpy errorSpy(&localSocket, SIGNAL(error(QLlcpSocket::SocketState)));
    QSignalSpy bytesWrittenSpy(&localSocket, SIGNAL(bytesWritten(qint64)));

    //Prepare data to send
    QByteArray outPayload;
    QDataStream out(&outPayload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (quint16)0;
    out << payLoad;

    qDebug("Client-- write quint16 length = %d", sizeof(quint16));
    qDebug("Client-- write echo string = %s", qPrintable(payLoad));
    qDebug("Client-- write echo string length= %d", payLoad.length());
    qDebug("Client-- write payload length = %d", outPayload.length());
    out.device()->seek(0);
    out << (quint16)(outPayload.size() - sizeof(quint16));

    qint64 ret = localSocket.writeDatagram(outPayload, m_target, m_port);
    QVERIFY(ret == 0);

    QTRY_VERIFY(!bytesWrittenSpy.isEmpty());

    qint64 written = 0;
    qint32 lastSignalCount = 0;
    while(true)
    {
       for(int i = lastSignalCount; i < bytesWrittenSpy.count(); i++) {
           written += bytesWrittenSpy.at(i).at(0).value<qint64>();
       }
       lastSignalCount = bytesWrittenSpy.count();
       qDebug() << "current signal count = " << lastSignalCount;
       qDebug() << "written payload size = " << written;
       if (written < outPayload.size()) {
           QTRY_VERIFY(bytesWrittenSpy.count() > lastSignalCount);
       }
       else {
           break;
       }
    }
    qDebug() << "Overall bytesWritten = " << written;
    qDebug() << "Overall block size = " << outPayload.size();
    QVERIFY(written == outPayload.size());
}


/*!
 Description: Send the message and Receive the acknowledged identical message

 TestScenario:
           1. Local peer sends the very long message to the remote peer
           2. Local peer sends the second very long message to the remote peer

 TestExpectedResults:
           1. The message has be sent to remote peer.
           2. The second message has been received from remote peer.
*/
void tst_qllcpsocketlocal::multipleClient()
{
    QLlcpSocket localSocket;
    QString longStr1k;
    for (int i = 0; i < 1000; i++)
        longStr1k.append((char)(i%26 + 'a'));

    QString longStr2k;
    for (int i = 0; i < 2000; i++)
        longStr2k.append((char)(i%26 + 'b'));

    QCOMPARE(localSocket.state(), QLlcpSocket::UnconnectedState);
    QSignalSpy stateChangedSpy(&localSocket, SIGNAL(stateChanged(QLlcpSocket::SocketState)));

    // STEP 1:  bind the local port for current socket
    bool retBool = localSocket.bind(m_port);
    QVERIFY(retBool);
    QVERIFY(!stateChangedSpy.isEmpty());
    QCOMPARE(localSocket.state(), QLlcpSocket::BoundState);

    // Wait remote part bind
    QString messageBox("Wait remote bind");
    QNfcTestUtil::ShowAutoMsg(messageBox);

    writeMessage(localSocket,longStr1k);
    writeMessage(localSocket,longStr2k);
}


void tst_qllcpsocketlocal::testCase2()
{
    QLlcpSocket localSocket;
    quint8 localPort = 38;

    // STEP 1:
    //QSignalSpy bytesWrittenSpy(&localSocket, SIGNAL(bytesWritten(qint64)));
    QString message("testcase2 string str1");
    QByteArray tmpArray(message.toAscii());
    const char* data =  tmpArray.data();
    qint64 strSize = message.size();
    qint64 val = localSocket.writeDatagram(data, strSize, m_target, localPort);
    QVERIFY(val != -1);

    // STEP 2:
    QString message2("testcase2 string str2");
    QByteArray tmpArray2(message2.toAscii());
    const char* data2 =  tmpArray2.data();
    qint64 strSize2 = message2.size();
    qint64 val2 = localSocket.writeDatagram(data2, strSize2, m_target, localPort);
    QVERIFY(val2 != -1);

    // STEP 3:
    const int Timeout = 2 * 1000;
    bool ret = localSocket.waitForBytesWritten(Timeout);
    QVERIFY(ret);

     // STEP 4:
    ret = localSocket.waitForBytesWritten(Timeout);
    QVERIFY(ret);

    QString messageBox("handshake 3");
    QNfcTestUtil::ShowAutoMsg(messageBox);

    // STEP 5: Try to cover waitForBytesWritten() in bound mode
    const int Timeout1 = 1 * 1000;
    localSocket.waitForBytesWritten(Timeout1);
    QVERIFY(ret);
}

/*!
 Description: coverage testcase - targeted for sender doCancel
*/
void tst_qllcpsocketlocal::testCase3()
{
    QLlcpSocket localSocket;
    // STEP 1:
    QString message("string1");
    QByteArray tmpArray(message.toAscii());
    const char* data =  tmpArray.data();
    qint64 strSize = message.size();
    localSocket.writeDatagram(data,strSize,m_target, m_port);
}

/*!
 Description: coverage testcase - invalid usage of connection-oriented API
*/
void tst_qllcpsocketlocal::coverageTest1()
{
    QLlcpSocket localSocket;

    QSignalSpy errorSpy(&localSocket, SIGNAL(error(QLlcpSocket::SocketError)));
    localSocket.connectToService(m_target,"uri");
    QTRY_VERIFY(errorSpy.count() == 1);
    QVERIFY(localSocket.error() == QLlcpSocket::UnknownSocketError);

    localSocket.disconnectFromService();
    QTRY_VERIFY(errorSpy.count() == 2);

    QVERIFY(localSocket.waitForConnected() == false);
    QVERIFY(localSocket.waitForDisconnected() == false);

    QString message = "Oops, must follow a port parameter";
    QByteArray tmpArray(message.toAscii());
    const char* data =  tmpArray.data();
    qint64 strSize = message.size();
    qint64 ret = localSocket.writeDatagram(data,strSize);
    QVERIFY(ret == -1);
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
 CounterPart test: tst_qllcpsocketremote::dumpServer()
*/
void tst_qllcpsocketlocal::deleteSocketWhenInUse()
    {
    QString message("deleteSocketWhenInUse test");
    QNfcTestUtil::ShowAutoMsg(message);

    QLlcpSocket* socket = new QLlcpSocket;

    bool retBool = socket->bind(m_port);
    QVERIFY(retBool);

    // STEP 3: Local peer sends the  message to the remote peer
    QSignalSpy errorSpy(socket, SIGNAL(error(QLlcpSocket::SocketState)));

    BytesWrittenSlot slot(socket);

    QString echo;
    for (int i = 0; i < 2000; i++)
        echo.append((char)(i%26 + 'a'));
    //Prepare data to send
    QByteArray outPayload;
    QDataStream out(&outPayload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (quint16)0;
    out << echo;

    out.device()->seek(0);
    out << (quint16)(outPayload.size() - sizeof(quint16));

    bool ret = socket->writeDatagram(outPayload, m_target, m_port);
    QVERIFY(ret == 0);

    QTest::qWait(5 * 1000);//give some time to wait bytesWritten signal
    QVERIFY(errorSpy.isEmpty());

    }

class WaitBytesWrittenSlot : public QObject
{
    Q_OBJECT
public:
    WaitBytesWrittenSlot(QLlcpSocket& s): m_socket(s),m_signalCount(0)
        {
        connect(&m_socket,SIGNAL(bytesWritten(qint64)),this,SLOT(gotBytesWritten(qint64)));
        }
private slots:
void gotBytesWritten(qint64 w)
        {
        m_signalCount++;
        qDebug()<<"Got BytesWritten() signal number = "<<m_signalCount;
        const int Timeout = 50;//3* 1000 seems too long, nfc server will panic spray signal
        bool ret = m_socket.waitForBytesWritten(Timeout);
        if (!ret)
            {
            qDebug()<<"WaitBytesWrittenSlot() in slot of ReadyRead signal return false";
            }
        else
            {
            qDebug()<<"WaitBytesWrittenSlot() in slot of ReadyRead signal return true";
            }
        }
private:
    QLlcpSocket& m_socket;
    int m_signalCount;
};
/*!
 Description: Test WaitForBytesWritten() in slot of
 bytesWritten signal, make sure the signal will not
 be emitted twice in the slot function.
 CounterPart test: tst_qllcpsocketremote::echoServer()
*/
void tst_qllcpsocketlocal::waitBytesWrittenInSlot()
{
    QLlcpSocket localSocket;

    QCOMPARE(localSocket.state(), QLlcpSocket::UnconnectedState);
    QSignalSpy stateChangedSpy(&localSocket, SIGNAL(stateChanged(QLlcpSocket::SocketState)));

    QSignalSpy readyReadSpy(&localSocket, SIGNAL(readyRead()));
    bool retBool = localSocket.bind(m_port);
    QVERIFY(retBool);
    QVERIFY(!stateChangedSpy.isEmpty());
    QCOMPARE(localSocket.state(), QLlcpSocket::BoundState);

    // Wait remote part bind
    QString messageBox("Wait remote bind");
    QNfcTestUtil::ShowAutoMsg(messageBox);

    // STEP 3: Local peer sends the  message to the remote peer
    QSignalSpy errorSpy(&localSocket, SIGNAL(error(QLlcpSocket::SocketState)));
    QSignalSpy bytesWrittenSpy(&localSocket, SIGNAL(bytesWritten(qint64)));

    WaitBytesWrittenSlot slot(localSocket);

    QString echoPayload;
    for (int i = 0; i < 2000; i++)
        echoPayload.append((char)(i%26 + 'a'));
    //Prepare data to send
    QByteArray outPayload;
    QDataStream out(&outPayload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (quint16)0;
    out << echoPayload;

    out.device()->seek(0);
    out << (quint16)(outPayload.size() - sizeof(quint16));

    bool ret = localSocket.writeDatagram(outPayload, m_target, m_port);
    QVERIFY(ret == 0);

    QTRY_VERIFY(!bytesWrittenSpy.isEmpty());

    qint64 written = 0;
    qint32 lastSignalCount = 0;
    while(true)
    {
        for(int i = lastSignalCount; i < bytesWrittenSpy.count(); i++) {
            written += bytesWrittenSpy.at(i).at(0).value<qint64>();
        }
        lastSignalCount = bytesWrittenSpy.count();
        qDebug() << "current signal count = " << lastSignalCount;
        qDebug() << "written payload size = " << written;
        if (written < outPayload.size()) {
            QTRY_VERIFY(bytesWrittenSpy.count() > lastSignalCount);
        }
        else {
            break;
        }
    }
    qDebug() << "Overall bytesWritten = " << written;
    qDebug() << "Overall block size = " << outPayload.size();
    QVERIFY(written == outPayload.size());

    // STEP 4: Start read payload from server
    QString messageBox2("Wait remote send buffer");
    QNfcTestUtil::ShowAutoMsg(messageBox2, &readyReadSpy);

    QByteArray inPayload;
    while(localSocket.hasPendingDatagrams()) {
        QByteArray tempBuffer;
        tempBuffer.resize(localSocket.pendingDatagramSize());
        quint8 remotePort = 0;
        QSignalSpy readyRead(&localSocket, SIGNAL(readyRead()));
        qint64 readSize = localSocket.readDatagram(tempBuffer.data()
                                                   , tempBuffer.size()
                                                   , &m_target, &remotePort);
        QVERIFY(readSize != -1);
        QVERIFY(remotePort > 0);
        inPayload.append(tempBuffer);
        qDebug() << "Client-- read inPayload size=" << inPayload.size();
        qDebug() << "Client-- read remotePort=" << remotePort;
        if (inPayload.size() >= (int)sizeof(quint16)) {
            break;
        }
        else
        {
            qDebug() << "Client-- not enough header";
            QTRY_VERIFY(!readyRead.isEmpty());
        }
    }

    QDataStream in(inPayload);
    in.setVersion(QDataStream::Qt_4_6);
    quint16 headerSize = 0; // size of real echo payload
    in >> headerSize;
    qDebug() << "Client-- read headerSize=" << headerSize;
    while (inPayload.size() < headerSize + (int)sizeof(quint16)){
        QSignalSpy readyRead(&localSocket, SIGNAL(readyRead()));
        QTRY_VERIFY(!readyRead.isEmpty());
        if(localSocket.hasPendingDatagrams()) {
            QByteArray tempBuffer;
            tempBuffer.resize(localSocket.pendingDatagramSize());
            quint8 remotePort = 0;
            qint64 readSize = localSocket.readDatagram(tempBuffer.data()
                                                       , tempBuffer.size()
                                                       , &m_target, &remotePort);
            QVERIFY(readSize != -1);
            QVERIFY(remotePort > 0);
            inPayload.append(tempBuffer);
            qDebug() << "Client-- read long blocksize=" << inPayload.size();
        }
    }

    QDataStream in2(inPayload);
    in2.setVersion(QDataStream::Qt_4_6);
    in2 >> headerSize;
    QString inEchoPayload;
    in2 >> inEchoPayload;
    qDebug() << "Client-- in echo string len:" << inEchoPayload.length();
    qDebug("Client-- in echo string = %s", qPrintable(inEchoPayload));
    //test the echoed string is same as the original string
    QVERIFY(echoPayload == inEchoPayload);
    // make sure the no error signal emitted
    QVERIFY(errorSpy.isEmpty());
}
/*!
 Description: writeDatagram negative testcase I - invalid port num & wait* functions
*/
void tst_qllcpsocketlocal::negTestCase1()
{
    QLlcpSocket localSocket;
    QString message = "Oops, Invalid port num for writeDatagram";
    QByteArray tmpArray(message.toAscii());
    const char* data =  tmpArray.data();
    qint64 strSize = message.size();
    qint8 invalidPort = -1;
    qint64 ret = localSocket.writeDatagram(data,strSize,m_target, invalidPort);
    QVERIFY(ret == -1);

    const int Timeout = 1 * 500;
    bool retBool = localSocket.waitForBytesWritten(Timeout);
    QVERIFY(!retBool);

    //Cover QLLCPUnConnected::WaitForReadyRead
    retBool = localSocket.waitForReadyRead(Timeout);
    QVERIFY(!retBool);

    //Cover QLLCPBind::WaitForReadyRead()
    retBool = localSocket.waitForReadyRead(Timeout);
    QVERIFY(!retBool);
}

/*!
 Description: bind negative test - double bind
*/
void tst_qllcpsocketlocal::negTestCase2()
{
    QLlcpSocket localSocket1;
    QLlcpSocket localSocket2;
    // bind again will cause failure
    bool ret2 = localSocket1.bind(m_port);
    QVERIFY(ret2);
    ret2 = localSocket2.bind(m_port);
    QVERIFY(!ret2);
}

/*!
 Description: bind negative test - invalid port num
*/
/*
void tst_qllcpsocketlocal::negTestCase3()
{
    QLlcpSocket localSocket;
    bool ret = localSocket.bind(65);
    QVERIFY(ret == false);
}
*/

/*!
 Description: bind negative test - invalid port num II
*/
/*
void tst_qllcpsocketlocal::negTestCase4()
{
    QLlcpSocket localSocket;
    int reservedPort = 15;
    bool ret = localSocket.bind(reservedPort);
    QVERIFY(ret == false);
}
*/
void tst_qllcpsocketlocal::multiSocketToOneServer()
{
    QString echoPayload = "multiSocketToOneServer";
    {
    QLlcpSocket localSocket;


    QCOMPARE(localSocket.state(), QLlcpSocket::UnconnectedState);
    QSignalSpy stateChangedSpy(&localSocket, SIGNAL(stateChanged(QLlcpSocket::SocketState)));

    // STEP 2:  bind the local port for current socket
    QSignalSpy readyReadSpy(&localSocket, SIGNAL(readyRead()));
    bool retBool = localSocket.bind(m_port);
    QVERIFY(retBool);
    QVERIFY(!stateChangedSpy.isEmpty());
    QCOMPARE(localSocket.state(), QLlcpSocket::BoundState);

    // Wait remote part bind
    QString messageBox("Wait remote bind");
    QNfcTestUtil::ShowAutoMsg(messageBox);

    // STEP 3: Local peer sends the  message to the remote peer
    QSignalSpy errorSpy(&localSocket, SIGNAL(error(QLlcpSocket::SocketState)));
    QSignalSpy bytesWrittenSpy(&localSocket, SIGNAL(bytesWritten(qint64)));

    //Prepare data to send
    QByteArray outPayload;
    QDataStream out(&outPayload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (quint16)0;
    out << echoPayload;

    qDebug("Client-- write quint16 length = %d", sizeof(quint16));
    qDebug("Client-- write echo string length= %d", echoPayload.length());
    qDebug("Client-- write payload length = %d", outPayload.length());
    out.device()->seek(0);
    out << (quint16)(outPayload.size() - sizeof(quint16));

    bool ret = localSocket.writeDatagram(outPayload, m_target, m_port);
    QVERIFY(ret == 0);

    QTRY_VERIFY(!bytesWrittenSpy.isEmpty());

    qint64 written = 0;
    qint32 lastSignalCount = 0;
    while(true)
    {
        for(int i = lastSignalCount; i < bytesWrittenSpy.count(); i++) {
            written += bytesWrittenSpy.at(i).at(0).value<qint64>();
        }
        lastSignalCount = bytesWrittenSpy.count();
        qDebug() << "current signal count = " << lastSignalCount;
        qDebug() << "written payload size = " << written;
        if (written < outPayload.size()) {
            QTRY_VERIFY(bytesWrittenSpy.count() > lastSignalCount);
        }
        else {
            break;
        }
    }
    qDebug() << "Overall bytesWritten = " << written;
    qDebug() << "Overall block size = " << outPayload.size();
    QVERIFY(written == outPayload.size());

    // STEP 4: Start read payload from server
    QString messageBox2("Wait remote send buffer");
    QNfcTestUtil::ShowAutoMsg(messageBox2, &readyReadSpy);

    QByteArray inPayload;
    while(localSocket.hasPendingDatagrams()) {
        QByteArray tempBuffer;
        tempBuffer.resize(localSocket.pendingDatagramSize());
        quint8 remotePort = 0;
        QSignalSpy readyRead(&localSocket, SIGNAL(readyRead()));
        qint64 readSize = localSocket.readDatagram(tempBuffer.data()
                                                   , tempBuffer.size()
                                                   , &m_target, &remotePort);
        QVERIFY(readSize != -1);
        QVERIFY(remotePort > 0);
        inPayload.append(tempBuffer);
        qDebug() << "Client-- read inPayload size=" << inPayload.size();
        qDebug() << "Client-- read remotePort=" << remotePort;
        if (inPayload.size() >= (int)sizeof(quint16)) {
            break;
        }
        else
        {
            qDebug() << "Client-- not enough header";
            QTRY_VERIFY(!readyRead.isEmpty());
        }
    }

    QDataStream in(inPayload);
    in.setVersion(QDataStream::Qt_4_6);
    quint16 headerSize = 0; // size of real echo payload
    in >> headerSize;
    qDebug() << "Client-- read headerSize=" << headerSize;
    while (inPayload.size() < headerSize + (int)sizeof(quint16)){
        QSignalSpy readyRead(&localSocket, SIGNAL(readyRead()));
        QTRY_VERIFY(!readyRead.isEmpty());
        if(localSocket.hasPendingDatagrams()) {
            QByteArray tempBuffer;
            tempBuffer.resize(localSocket.pendingDatagramSize());
            quint8 remotePort = 0;
            qint64 readSize = localSocket.readDatagram(tempBuffer.data()
                                                       , tempBuffer.size()
                                                       , &m_target, &remotePort);
            QVERIFY(readSize != -1);
            QVERIFY(remotePort > 0);
            inPayload.append(tempBuffer);
            qDebug() << "Client-- read long blocksize=" << inPayload.size();
        }
    }

    QDataStream in2(inPayload);
    in2.setVersion(QDataStream::Qt_4_6);
    in2 >> headerSize;
    QString inEchoPayload;
    in2 >> inEchoPayload;
    qDebug() << "Client-- in echo string len:" << inEchoPayload.length();
    qDebug("Client-- in echo string = %s", qPrintable(inEchoPayload));
    //test the echoed string is same as the original string
    QVERIFY(echoPayload == inEchoPayload);
    // make sure the no error signal emitted
    QVERIFY(errorSpy.isEmpty());

    }
    {
    QLlcpSocket localSocket;


    QCOMPARE(localSocket.state(), QLlcpSocket::UnconnectedState);
    QSignalSpy stateChangedSpy(&localSocket, SIGNAL(stateChanged(QLlcpSocket::SocketState)));

    // STEP 2:  bind the local port for current socket
    QSignalSpy readyReadSpy(&localSocket, SIGNAL(readyRead()));
    bool retBool = localSocket.bind(m_port);
    QVERIFY(retBool);
    QVERIFY(!stateChangedSpy.isEmpty());
    QCOMPARE(localSocket.state(), QLlcpSocket::BoundState);

    // Wait remote part bind
    QString messageBox("Wait remote bind");
    QNfcTestUtil::ShowAutoMsg(messageBox);

    // STEP 3: Local peer sends the  message to the remote peer
    QSignalSpy errorSpy(&localSocket, SIGNAL(error(QLlcpSocket::SocketState)));
    QSignalSpy bytesWrittenSpy(&localSocket, SIGNAL(bytesWritten(qint64)));

    //Prepare data to send
    QByteArray outPayload;
    QDataStream out(&outPayload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);
    out << (quint16)0;
    out << echoPayload;

    qDebug("Client-- write quint16 length = %d", sizeof(quint16));
    qDebug("Client-- write echo string length= %d", echoPayload.length());
    qDebug("Client-- write payload length = %d", outPayload.length());
    out.device()->seek(0);
    out << (quint16)(outPayload.size() - sizeof(quint16));

    bool ret = localSocket.writeDatagram(outPayload, m_target, m_port);
    QVERIFY(ret == 0);

    QTRY_VERIFY(!bytesWrittenSpy.isEmpty());

    qint64 written = 0;
    qint32 lastSignalCount = 0;
    while(true)
    {
        for(int i = lastSignalCount; i < bytesWrittenSpy.count(); i++) {
            written += bytesWrittenSpy.at(i).at(0).value<qint64>();
        }
        lastSignalCount = bytesWrittenSpy.count();
        qDebug() << "current signal count = " << lastSignalCount;
        qDebug() << "written payload size = " << written;
        if (written < outPayload.size()) {
            QTRY_VERIFY(bytesWrittenSpy.count() > lastSignalCount);
        }
        else {
            break;
        }
    }
    qDebug() << "Overall bytesWritten = " << written;
    qDebug() << "Overall block size = " << outPayload.size();
    QVERIFY(written == outPayload.size());

    // STEP 4: Start read payload from server
    QString messageBox2("Wait remote send buffer");
    QNfcTestUtil::ShowAutoMsg(messageBox2, &readyReadSpy);

    QByteArray inPayload;
    while(localSocket.hasPendingDatagrams()) {
        QByteArray tempBuffer;
        tempBuffer.resize(localSocket.pendingDatagramSize());
        quint8 remotePort = 0;
        QSignalSpy readyRead(&localSocket, SIGNAL(readyRead()));
        qint64 readSize = localSocket.readDatagram(tempBuffer.data()
                                                   , tempBuffer.size()
                                                   , &m_target, &remotePort);
        QVERIFY(readSize != -1);
        QVERIFY(remotePort > 0);
        inPayload.append(tempBuffer);
        qDebug() << "Client-- read inPayload size=" << inPayload.size();
        qDebug() << "Client-- read remotePort=" << remotePort;
        if (inPayload.size() >= (int)sizeof(quint16)) {
            break;
        }
        else
        {
            qDebug() << "Client-- not enough header";
            QTRY_VERIFY(!readyRead.isEmpty());
        }
    }

    QDataStream in(inPayload);
    in.setVersion(QDataStream::Qt_4_6);
    quint16 headerSize = 0; // size of real echo payload
    in >> headerSize;
    qDebug() << "Client-- read headerSize=" << headerSize;
    while (inPayload.size() < headerSize + (int)sizeof(quint16)){
        QSignalSpy readyRead(&localSocket, SIGNAL(readyRead()));
        QTRY_VERIFY(!readyRead.isEmpty());
        if(localSocket.hasPendingDatagrams()) {
            QByteArray tempBuffer;
            tempBuffer.resize(localSocket.pendingDatagramSize());
            quint8 remotePort = 0;
            qint64 readSize = localSocket.readDatagram(tempBuffer.data()
                                                       , tempBuffer.size()
                                                       , &m_target, &remotePort);
            QVERIFY(readSize != -1);
            QVERIFY(remotePort > 0);
            inPayload.append(tempBuffer);
            qDebug() << "Client-- read long blocksize=" << inPayload.size();
        }
    }

    QDataStream in2(inPayload);
    in2.setVersion(QDataStream::Qt_4_6);
    in2 >> headerSize;
    QString inEchoPayload;
    in2 >> inEchoPayload;
    qDebug() << "Client-- in echo string len:" << inEchoPayload.length();
    qDebug("Client-- in echo string = %s", qPrintable(inEchoPayload));
    //test the echoed string is same as the original string
    QVERIFY(echoPayload == inEchoPayload);
    // make sure the no error signal emitted
    QVERIFY(errorSpy.isEmpty());

    }

}
void tst_qllcpsocketlocal::cleanupTest()
{
}

QTEST_MAIN(tst_qllcpsocketlocal);

#include "tst_qllcpsocketlocal.moc"

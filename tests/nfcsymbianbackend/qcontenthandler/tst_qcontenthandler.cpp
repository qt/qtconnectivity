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

#include "qnfctestcommon.h"
#include "qnfctestutil.h"

#include <qnearfieldmanager.h>
#include <qnearfieldtarget.h>
#include <qnearfieldtagtype1.h>
#include <qndefnfctextrecord.h>
#include <qndefnfcurirecord.h>
#include <qndefmessage.h>
#include <qndefrecord.h>

QTNFC_USE_NAMESPACE

Q_DECLARE_METATYPE(QNearFieldTarget*)
Q_DECLARE_METATYPE(QNearFieldTarget::Type)
Q_DECLARE_METATYPE(QNdefFilter)


static const QString& logFileName = "E:\\testserviceprovider.dat";
static const QString& logDirName = "E:\\";

static const QString& logFileName_negative = "E:\\testserviceprovider2.dat";
static const QString& expectedLog = "register handle return -1";


class MessageListener : public QObject
{
    Q_OBJECT

signals:
    void matchedNdefMessage(const QNdefMessage &message, QNearFieldTarget *target);
};

class tst_QContentHandler : public QObject
{
    Q_OBJECT

public:
    tst_QContentHandler();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testcase_01();
    void testcase_02();
    void testcase_03_negative();
    
    
private:
    QNearFieldManager m_manager;
    QNdefMessage m_message;
};

tst_QContentHandler::tst_QContentHandler()
{
    qRegisterMetaType<QNdefMessage>("QNdefMessage");
    qRegisterMetaType<QNearFieldTarget *>("QNearFieldTarget*");
}

void tst_QContentHandler::initTestCase()
{
  
}

void tst_QContentHandler::cleanupTestCase()
{
}

/*!
 Description: Unit test for NFC content handler

 TestScenario:  Write a Ndef Message into tag type 1, read the tag again to ensure corresponding service provider 
                being invoked
 Note: This case need nfctestserviceprovider sis to be installed

*/
void tst_QContentHandler::testcase_01()
{
    //first step, delete the log file generated by nfctestserviceprovider if any
    QFile m_file(logFileName);
    if (m_file.exists())
        {
        QDir(logDirName).remove(logFileName);
        }
    QVERIFY(m_file.exists() == false);
      
    //second step, write the ndef message into the tag
    QNearFieldTagType1* target;
    QSignalSpy targetDetectedSpy(&m_manager, SIGNAL(targetDetected(QNearFieldTarget*)));
    
    m_manager.startTargetDetection(QNearFieldTarget::NfcTagType1);

    QString hint("please touch a writable tag of type 1");
    QNfcTestUtil::ShowAutoMsg(hint, &targetDetectedSpy);

    QTRY_VERIFY(!targetDetectedSpy.isEmpty());
    
    target = qobject_cast<QNearFieldTagType1 *>(targetDetectedSpy.at(targetDetectedSpy.count()-1).at(0).value<QNearFieldTarget *>());
    // make sure target can be detected
    QVERIFY(target);

    // make sure target uid is not empty
    QVERIFY(!target->uid().isEmpty());
   
    QNdefRecord record;
    record.setTypeNameFormat(QNdefRecord::ExternalRtd);
    record.setType("R");
    record.setPayload(QByteArray(2, quint8(0x55)));
    m_message.append(record);

    QList<QNdefMessage> messages;
    messages.append(m_message);

    QSignalSpy ndefMessageReadSpy(target, SIGNAL(ndefMessageRead(QNdefMessage)));
    QSignalSpy ndefMessageWriteSpy(target, SIGNAL(ndefMessagesWritten()));

    target->writeNdefMessages(messages);
    QTRY_VERIFY(!ndefMessageWriteSpy.isEmpty());

    target->readNdefMessages();
    QTRY_VERIFY(!ndefMessageReadSpy.isEmpty());
    
    const QNdefMessage& ndefMessage_new(ndefMessageReadSpy.first().at(0).value<QNdefMessage>());
    QVERIFY(messages.at(0) == ndefMessage_new);
    
    QSignalSpy targetLostSpy(&m_manager, SIGNAL(targetLost(QNearFieldTarget*)));
    QNfcTestUtil::ShowAutoMsg("please remove the tag", &targetLostSpy);
    QTRY_VERIFY(!targetLostSpy.isEmpty());
    
    delete target;
    
    //third step: stop target detection(so that the ndef message can be routed to service provider) 
    //            and touch the tag again to invoke service provider, if the service provider being
    //            invoked successfully, the log file contains the message content will be generated
    //            compare the message in the log file with the message writen to the tag
    m_manager.stopTargetDetection();
        
    hint = "please touch the tag just be writen successfully";
    QNfcTestUtil::ShowMessage(hint);
    
    QVERIFY(m_file.exists());
    m_file.open(QIODevice::ReadOnly);
    QDataStream fileInData (&m_file);
    QByteArray msgByteArray;
    fileInData >> msgByteArray;   
    QNdefMessage msgFromLog = QNdefMessage::fromByteArray(msgByteArray);
    
    QVERIFY(m_message == msgFromLog);
    
    hint = "please remove tag";
    QNfcTestUtil::ShowMessage(hint);
}

/*!
 Description: Unit test for NFC content handler

 TestScenario:  Read the tag again to ensure if an application registered for the ndef message,
                the ndef message can't be routed to service provider
 Note: This case need nfctestserviceprovider sis to be installed

*/
void tst_QContentHandler::testcase_02()
{
    //first step, delete the log file generated by nfctestserviceprovider if any
    QFile m_file(logFileName);
    if (m_file.exists())
        {
        QDir(logDirName).remove(logFileName);
        }
    QVERIFY(m_file.exists() == false);
  
    //second step, register for the ndef message, so that the test case itself 
    // is the application registered for the ndef message
    QNdefFilter filter;
    filter.appendRecord(QNdefRecord::ExternalRtd, "R");
    MessageListener listener;
    QSignalSpy messageSpy(&listener, SIGNAL(matchedNdefMessage(QNdefMessage,QNearFieldTarget*)));
    int id = m_manager.registerNdefMessageHandler(filter, &listener,
                                                       SIGNAL(matchedNdefMessage(QNdefMessage,QNearFieldTarget*)));
    QVERIFY(id != -1);
    
    //third step, touch the tag with the ndef message, to verify the log file can't
    // be generated because the service provider can't be invoked
    QString hint("please touch again the tag just be writen successfully");
    QNfcTestUtil::ShowAutoMsg(hint, &messageSpy, 1);
       
    QVERIFY(m_file.exists() == false);
    
    m_manager.unregisterNdefMessageHandler(id);
    
    hint = "please remove tag";
    QNfcTestUtil::ShowMessage(hint);

}

/*!
 Description: Unit test for NFC content handler

 TestScenario:  Write a Ndef Message into tag type 1, read the tag again to ensure corresponding service provider 
                being invoked but the corresponding interface can't be invoked because user write xml file wrongly
 Note: This case need nfctestserviceprovider2 sis to be installed

*/
void tst_QContentHandler::testcase_03_negative()
{
    //first step, delete the log file generated by nfctestserviceprovider2 if any
    QFile m_file(logFileName_negative);
    if (m_file.exists())
        {
        QDir(logDirName).remove(logFileName_negative);
        }
    QVERIFY(!m_file.exists());
      
    //second step, write the ndef message into the tag
    QNearFieldTagType1* target;
    QSignalSpy targetDetectedSpy(&m_manager, SIGNAL(targetDetected(QNearFieldTarget*)));
    
    m_manager.startTargetDetection(QNearFieldTarget::NfcTagType1);

    QString hint("please touch a writable tag of type 1");
    QNfcTestUtil::ShowAutoMsg(hint, &targetDetectedSpy);

    QTRY_VERIFY(!targetDetectedSpy.isEmpty());
    
    target = qobject_cast<QNearFieldTagType1 *>(targetDetectedSpy.at(targetDetectedSpy.count()-1).at(0).value<QNearFieldTarget *>());
    // make sure target can be detected
    QVERIFY(target);

    // make sure target uid is not empty
    QVERIFY(!target->uid().isEmpty());
   
    QNdefRecord record;
    record.setTypeNameFormat(QNdefRecord::ExternalRtd);
    record.setType("S");
    record.setPayload(QByteArray(2, quint8(0x55)));
    m_message.clear();
    m_message.append(record);

    QList<QNdefMessage> messages;
    messages.append(m_message);

    QSignalSpy ndefMessageReadSpy(target, SIGNAL(ndefMessageRead(QNdefMessage)));
    QSignalSpy ndefMessageWriteSpy(target, SIGNAL(ndefMessagesWritten()));

    target->writeNdefMessages(messages);
    QTRY_VERIFY(!ndefMessageWriteSpy.isEmpty());

    target->readNdefMessages();
    QTRY_VERIFY(!ndefMessageReadSpy.isEmpty());
    
    const QNdefMessage& ndefMessage_new(ndefMessageReadSpy.first().at(0).value<QNdefMessage>());
    QVERIFY(messages.at(0) == ndefMessage_new);
    
    QSignalSpy targetLostSpy(&m_manager, SIGNAL(targetLost(QNearFieldTarget*)));
    QNfcTestUtil::ShowAutoMsg("please remove the tag", &targetLostSpy);
    QTRY_VERIFY(!targetLostSpy.isEmpty());
    
    delete target;
    m_manager.stopTargetDetection();
    
    //third step: stop target detection(so that the ndef message can be routed to service provider) 
    //            and touch the tag again to invoke service provider, if the service provider being
    //            invoked successfully, the log file contains the register fail info will be generated
    //            compare the message in the log file with the expected info
      
    hint = "please touch the tag just be writen successfully";
    QNfcTestUtil::ShowMessage(hint);    
 
    QVERIFY(m_file.exists());
    
    m_file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream fileInData(&m_file);
    QString logMsg = fileInData.readAll() ;
    
    qDebug() << "log message: " << logMsg;
    
    QVERIFY(expectedLog == logMsg);
    
    hint = "please remove tag";
    QNfcTestUtil::ShowMessage(hint);
}




QTEST_MAIN(tst_QContentHandler);

#include "tst_qcontenthandler.moc"

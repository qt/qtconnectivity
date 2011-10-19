/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
#include <qndefnfctextrecord.h>
#include <qndefnfcurirecord.h>
#include <qndefmessage.h>
#include <qndefrecord.h>

QTNFC_USE_NAMESPACE

Q_DECLARE_METATYPE(QNearFieldTarget*)
Q_DECLARE_METATYPE(QNearFieldTarget::Type)
Q_DECLARE_METATYPE(QNdefFilter)

class tst_QNearFieldManager : public QObject
{
    Q_OBJECT

public:
    tst_QNearFieldManager();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void targetDetected();
    void targetDetected_data();
    void unregisterNdefMessageHandler();
    void registerNdefMessageHandler();
    void registerNdefMessageHandler_filter_data();
    void registerNdefMessageHandler_filter();
    void registerNdefMessageHandler_filter_negtive();
    void registerNdefMessageHandler_filter_negtive_data();
};

tst_QNearFieldManager::tst_QNearFieldManager()
{
    qRegisterMetaType<QNdefMessage>("QNdefMessage");
    qRegisterMetaType<QNearFieldTarget *>("QNearFieldTarget*");
}

void tst_QNearFieldManager::initTestCase()
{
    qRegisterMetaType<QNearFieldTarget *>("QNearFieldTarget *");
}

void tst_QNearFieldManager::cleanupTestCase()
{
}

/*!
 Description: Unit test for NFC target detected and lost

 TestScenario: 1. Touch and remove llcp device/Type1/Type2/Type3/Type4 tag one by one

 TestExpectedResults: 1. llcp device/Type1/Type2/Type3/Type4 tag detected/lost signal can be received
*/

void tst_QNearFieldManager::targetDetected()
{
    QFETCH(QNearFieldTarget::Type, type);
    QFETCH(QString, hint);

    QNearFieldManager nfcManager;

    QSignalSpy targetDetectedSpy(&nfcManager, SIGNAL(targetDetected(QNearFieldTarget*)));
    QSignalSpy targetLostSpy(&nfcManager, SIGNAL(targetLost(QNearFieldTarget*)));

    nfcManager.startTargetDetection(type);

    QNfcTestUtil::ShowAutoMsg(hint, &targetDetectedSpy, 1);
    QTRY_VERIFY(!targetDetectedSpy.isEmpty());

    QNearFieldTarget *target = targetDetectedSpy.at(targetDetectedSpy.count()-1).at(0).value<QNearFieldTarget *>();

    QSignalSpy disconnectedSpy(target, SIGNAL(disconnected()));
    QVERIFY(target);

    if (type != QNearFieldTarget::NfcForumDevice)
    {
        QVERIFY(!target->uid().isEmpty());
        QCOMPARE(target->type(), type);
    }

    QNfcTestUtil::ShowAutoMsg("please remove the target", &disconnectedSpy, 1);
    QTRY_VERIFY(!targetLostSpy.isEmpty());

    QNearFieldTarget *lostTarget = targetLostSpy.first().at(0).value<QNearFieldTarget *>();

    QCOMPARE(target, lostTarget);

    QVERIFY(!disconnectedSpy.isEmpty());

    nfcManager.stopTargetDetection();
}

void tst_QNearFieldManager::targetDetected_data()
{
    QTest::addColumn<QNearFieldTarget::Type>("type");
    QTest::addColumn<QString>("hint");
    QTest::newRow("llcp device") << QNearFieldTarget::NfcForumDevice << "Please touch llcp device";
    QTest::newRow("NfcTagType1") << QNearFieldTarget::NfcTagType1 << "Please touch tag type1";
    QTest::newRow("NfcTagType2") << QNearFieldTarget::NfcTagType2 << "Please touch tag type2";
    QTest::newRow("NfcTagType3") << QNearFieldTarget::NfcTagType3 << "Please touch tag type3";
    QTest::newRow("NfcTagType4") << QNearFieldTarget::NfcTagType4 << "Please touch tag type4";
}

/*!
 Description: Unit test for NFC unregisterNdefMessageHandler function

 TestScenario: 1.

 TestExpectedResults: 1. return false
*/

void tst_QNearFieldManager::unregisterNdefMessageHandler()
{
    QNearFieldManager manager;

    QVERIFY(!manager.unregisterNdefMessageHandler(-1));
    QVERIFY(!manager.unregisterNdefMessageHandler(0));
}

class MessageListener : public QObject
{
    Q_OBJECT

signals:
    void matchedNdefMessage(const QNdefMessage &message, QNearFieldTarget *target);
};

/*!
 Description: Unit test for NFC registerNdefMessageHandler function

 TestScenario: 1. Symbian backend does not support registerNdefMessageHandler without a QNdefFilter

 TestExpectedResults: 1. return -1
*/
void tst_QNearFieldManager::registerNdefMessageHandler()
{
    QNearFieldManager manager;

    MessageListener listener;
    QSignalSpy messageSpy(&listener, SIGNAL(matchedNdefMessage(QNdefMessage,QNearFieldTarget*)));

    int id = manager.registerNdefMessageHandler(&listener,
                                                SIGNAL(matchedNdefMessage(QNdefMessage,QNearFieldTarget*)));

    QVERIFY(id == -1);//symbian backend does not support registerNdefMessageHandler without QNdefFilter
}

void tst_QNearFieldManager::registerNdefMessageHandler_filter_data()
{
    QTest::addColumn<QNdefFilter>("filter");
    QTest::addColumn<QString>("hint");

    QNdefFilter filter;

    filter.appendRecord(QNdefRecord::NfcRtd, "Sp");
    QTest::newRow("SP") << filter << "Please touch a tag with 'SP' NDef message";

    filter.clear();
    filter.setOrderMatch(true);
    filter.appendRecord<QNdefNfcTextRecord>(1, 1);
    filter.appendRecord<QNdefNfcUriRecord>(1, 1);
    QTest::newRow("Text + URI") << filter << "Please touch a tag with 'Text + URI' NDef message";

    filter.clear();
    filter.appendRecord<QNdefNfcUriRecord>(1, 1);
    QTest::newRow("URI") << filter << "Please touch a tag with 'URI' NDef message";

    filter.clear();
    filter.setOrderMatch(true);
    filter.appendRecord<QNdefNfcUriRecord>(1, 1);
    QTest::newRow("URI") << filter << "Please touch a tag with only one 'URI' NDef record";

}

/*!
 Description: Unit test for NFC registerNdefMessageHandler with a NDef filter

 TestScenario: 1. Touch a tag with random NDef message/with 'Image + Multiple Text + URI' NDef message/with 'Text + URI' NDef message

 TestExpectedResults: 1. matchedNdefMessage signal will be emitted
*/
void tst_QNearFieldManager::registerNdefMessageHandler_filter()
{
    QFETCH(QNdefFilter, filter);
    QFETCH(QString, hint);

    QNearFieldManager manager;

    MessageListener listener;
    QSignalSpy messageSpy(&listener, SIGNAL(matchedNdefMessage(QNdefMessage,QNearFieldTarget*)));

    int id = manager.registerNdefMessageHandler(filter, &listener,
                                                SIGNAL(matchedNdefMessage(QNdefMessage,QNearFieldTarget*)));

    QVERIFY(id != -1);

    QNfcTestUtil::ShowAutoMsg(hint, &messageSpy, 1);

    QTRY_VERIFY(!messageSpy.isEmpty());

    const QNdefMessage message = messageSpy.first().at(0).value<QNdefMessage>();

    QNearFieldTarget *target = messageSpy.first().at(1).value<QNearFieldTarget *>();

    QVERIFY(target == NULL);//symbain backend always return NULL target
    QCOMPARE(filter.recordCount(), message.count());
    qDebug()<<"message.count()="<<message.count();
    for (int i = 0; i < filter.recordCount(); ++i)
        {
        if (filter.orderMatch())
            {
            QCOMPARE(filter.recordAt(i).typeNameFormat, message.at(i).typeNameFormat());
            QCOMPARE(filter.recordAt(i).type, message.at(i).type());
            }
        else
            {
            bool matched = false;
            for (int j = 0; j < filter.recordCount(); ++j)
                {
                if (message.at(i).typeNameFormat() == filter.recordAt(i).typeNameFormat && message.at(i).type() == filter.recordAt(i).type)
                    {
                    matched = true;
                    break;
                    }
                }
            QVERIFY(matched);
            }
        }

    QVERIFY(manager.unregisterNdefMessageHandler(id));
}

void tst_QNearFieldManager::registerNdefMessageHandler_filter_negtive_data()
{
    QTest::addColumn<QNdefFilter>("filter");
    QTest::addColumn<QString>("hint");
    //negtive test
    QNdefFilter filter;

    QTest::newRow("Empty") << filter << "Please touch a tag without NDef message";

    filter.clear();
    filter.appendRecord<QNdefNfcUriRecord>(1, 1);
    QTest::newRow("URI") << filter << "Please touch a tag without 'URI' NDef message";

}

/*!
 Description: Unit test for NFC registerNdefMessageHandler with a NDef filter

 TestScenario: 1. Touch a tag without filter expected NDef message

 TestExpectedResults: 1. matchedNdefMessage signal will NOT be emitted
*/
void tst_QNearFieldManager::registerNdefMessageHandler_filter_negtive()
{
    QFETCH(QNdefFilter, filter);
    QFETCH(QString, hint);

    QNearFieldManager manager;

    MessageListener listener;
    QSignalSpy messageSpy(&listener, SIGNAL(matchedNdefMessage(QNdefMessage,QNearFieldTarget*)));

    int id = manager.registerNdefMessageHandler(filter, &listener,
                                                SIGNAL(matchedNdefMessage(QNdefMessage,QNearFieldTarget*)));

    QVERIFY(id != -1);

    QNfcTestUtil::ShowAutoMsg(hint);

    QTRY_VERIFY(messageSpy.isEmpty());
    QVERIFY(manager.unregisterNdefMessageHandler(id));
}

QTEST_MAIN(tst_QNearFieldManager);

#include "tst_qnearfieldmanager.moc"

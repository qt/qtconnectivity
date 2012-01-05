/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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
#include <QVariant>
#include <QVariantList>
#include "qnfctagtestcommon.h"
#include <qnearfieldtagtype3.h>
#include "qnfctestcommon.h"

QTNFC_USE_NAMESPACE

typedef QMap<quint16,QByteArray> checkResponseType;
Q_DECLARE_METATYPE(checkResponseType)

class NfcTagRawCommandOperationType3: public NfcTagRawCommandOperationCommon
{
public:
    NfcTagRawCommandOperationType3(QNearFieldTarget * tag);

    void run()
    {
        if (check)
        {
            mId = (tagType3->*check)(mSerivceBlockList);
        }

        if (update)
        {
            mId = (tagType3->*update)(mSerivceBlockList, mDataArray);
        }

        checkInvalidId();
        waitRequest();
    }

    void setCheck(const QMap<quint16, QList<quint16> > &serviceBlockList)
    {
        check = &QNearFieldTagType3::check;
        mSerivceBlockList = serviceBlockList;
    }

    void setUpdate(const QMap<quint16, QList<quint16> > &serviceBlockList,
                   const QByteArray &data)
    {
        update = &QNearFieldTagType3::update;
        mSerivceBlockList = serviceBlockList;
        mDataArray = data;
    }

    void checkResponse()
    {
        qDebug()<<"checkResponse begin";
        if (check)
        {
            // response is QMap<quint16, QByteArray>
            checkResponseType e = mExpectedResult.value<checkResponseType>();
            checkResponseType r = mTarget->requestResponse(mId).value<checkResponseType>();
            QCOMPARE(e, r);
        }
        else
        {
            QTRY_COMPARE(mTarget->requestResponse(mId), mExpectedResult);
        }
        qDebug()<<"checkResponse end";
    }

protected:
    QNearFieldTagType3 * tagType3;
    QNearFieldTarget::RequestId (QNearFieldTagType3::*check)(const QMap<quint16, QList<quint16> > &serviceBlockList);
    QNearFieldTarget::RequestId (QNearFieldTagType3::*update)(const QMap<quint16, QList<quint16> > &serviceBlockList,
                                                             const QByteArray &data);
    QMap<quint16, QList<quint16> > mSerivceBlockList;
    QByteArray mDataArray;
};

NfcTagRawCommandOperationType3::NfcTagRawCommandOperationType3(QNearFieldTarget * tag):NfcTagRawCommandOperationCommon(tag)
{
    tagType3 = qobject_cast<QNearFieldTagType3 *>(mTarget);
    QVERIFY(tagType3);
    check = 0;
    update = 0;
}

class tst_qnearfieldtagtype3 : public QObject
{
    Q_OBJECT

public:
    tst_qnearfieldtagtype3();
    void testRawAccessAndNdefAccess(const QList<QNdefMessage> &messages);

private Q_SLOTS:
    void initTestCase();
    void testRawAndNdefAccess();
    void cleanupTestCase(){}
private:
    QNfcTagTestCommon<QNearFieldTagType3> tester;
};


tst_qnearfieldtagtype3::tst_qnearfieldtagtype3()
{
}

void tst_qnearfieldtagtype3::initTestCase()
{

}

void tst_qnearfieldtagtype3::testRawAccessAndNdefAccess(const QList<QNdefMessage> &messages)
{
    QSignalSpy okSpy(tester.target, SIGNAL(requestCompleted(const QNearFieldTarget::RequestId&)));
    QSignalSpy errSpy(tester.target, SIGNAL(error(QNearFieldTarget::Error, const QNearFieldTarget::RequestId&)));
    QSignalSpy ndefMessageReadSpy(tester.target, SIGNAL(ndefMessageRead(QNdefMessage)));
    QSignalSpy ndefMessageWriteSpy(tester.target, SIGNAL(ndefMessagesWritten()));

    int okCount = 0;
    int errCount = 0;
    int ndefReadCount = 0;
    int ndefWriteCount = 0;

    // write ndef first
    tester.target->writeNdefMessages(messages);
    ++ndefWriteCount;
    QTRY_COMPARE(ndefMessageWriteSpy.count(), ndefWriteCount);

    QList<quint16> blockList;
    // first block
    blockList.append(0);
    // NDEF service
    quint16 serviceCode = 0x0B;

    QMap<quint16, QList<quint16> > serviceBlockList;
    serviceBlockList.insert(serviceCode, blockList);

    QNearFieldTarget::RequestId id = tester.target->check(serviceBlockList);

    QVERIFY(tester.target->waitForRequestCompleted(id));
    ++okCount;
    QCOMPARE(okSpy.count(), okCount);

    QMap<quint16, QByteArray> result = tester.target->requestResponse(id).value<QMap<quint16, QByteArray> >();
    QVERIFY(result.contains(serviceCode));

    const QByteArray& attribInfo = result.value(serviceCode);
    QVERIFY(!attribInfo.isEmpty());

    quint32 len = attribInfo.at(11);
    len<<=8;
    len |= attribInfo.at(12);
    len<<=8;
    len |= attribInfo.at(13);

    QCOMPARE(len, (quint32)(messages.at(0).toByteArray().count()));

    // read NDEF with RAW command
    int Nbc = (0 == len%16)?(len/16):(len/16+1);
    qDebug()<<"Nbc = "<<Nbc;

    serviceBlockList.clear();
    blockList.clear();
    for(int i = 1; i <= Nbc; ++i)
    {
        blockList.append(i);
    }
    serviceBlockList.insert(serviceCode, blockList);

    QNearFieldTarget::RequestId id1 = tester.target->check(serviceBlockList);

    QVERIFY(tester.target->waitForRequestCompleted(id1));
    ++okCount;
    QCOMPARE(okSpy.count(), okCount);

    QMap<quint16, QByteArray> ndefContent = tester.target->requestResponse(id1).value<QMap<quint16, QByteArray> >();

    QVERIFY(ndefContent.contains(serviceCode));
    QCOMPARE(ndefContent.value(serviceCode).left(len), messages.at(0).toByteArray());

    // update the ndef meesage with raw command
    QNdefMessage message;
    QNdefNfcTextRecord textRecord;
    textRecord.setText("nfc");
    message.append(textRecord);

    QByteArray expectedNdefContent = message.toByteArray();
    len = expectedNdefContent.count();
    serviceCode = 0x0009;

    // use previous attribute information block data, just update the len
    QByteArray newAttribInfo = attribInfo;
    newAttribInfo[13] = len&0xFF;
    newAttribInfo[12] = (len>>8)&0xFF;
    newAttribInfo[11] = (len>>16)&0xFF;
    newAttribInfo[9] = 0x0F;

    QByteArray ndefData;
    ndefData.append(newAttribInfo);
    ndefData.append(expectedNdefContent);

    qDebug()<<"updated ndef len = "<<len;

    for (int i = 0; i < 16 - ndefData.count()%16; ++i)
    {
        // appending padding data
        ndefData.append((char)0);
    }
    qDebug()<<"raw ndefData len = "<<ndefData.count();

    QList<quint16> updatedBlockList;

    int blockNumber = (len%16 == 0)?len/16:(len/16+1);
    qDebug()<<"updated block number = "<<blockNumber;

    for(int i = 0; i <= blockNumber; ++i)
    {
        updatedBlockList.append((char)i);
    }

    qDebug()<<"updatedBlockList len = "<<updatedBlockList.count();
    serviceBlockList.clear();
    serviceBlockList.insert(serviceCode, updatedBlockList);
    QNearFieldTarget::RequestId id2 = tester.target->update(serviceBlockList, ndefData);
    QVERIFY(tester.target->waitForRequestCompleted(id2));
    QVERIFY(tester.target->requestResponse(id2).toBool());
    ++okCount;
    QCOMPARE(okSpy.count(), okCount);

    // read ndef with ndef access
    tester.target->readNdefMessages();
    ++ndefReadCount;
    QTRY_COMPARE(ndefMessageReadSpy.count(), ndefReadCount);

    const QNdefMessage& ndefMessage_new(ndefMessageReadSpy.first().at(0).value<QNdefMessage>());

    QCOMPARE(expectedNdefContent, ndefMessage_new.toByteArray());
    QCOMPARE(errSpy.count(), errCount);
}


void tst_qnearfieldtagtype3::testRawAndNdefAccess()
{
    tester.touchTarget();
    QNdefMessage message;
    QNdefNfcUriRecord uriRecord;
    uriRecord.setUri(QUrl("http://qt.nokia.com"));
    message.append(uriRecord);

    QList<QNdefMessage> messages;
    messages.append(message);

    testRawAccessAndNdefAccess(messages);
    tester.removeTarget();
}
QTEST_MAIN(tst_qnearfieldtagtype3);

#include "tst_qnearfieldtagtype3.moc"

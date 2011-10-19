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
#include <QVariant>
#include <QVariantList>
#include <QtEndian>
#include <qnearfieldtagtype4.h>
#include "qnfctagtestcommon.h"
#include "qnfctestcommon.h"

QTNFC_USE_NAMESPACE

class NfcTagRawCommandOperationType4: public NfcTagRawCommandOperationCommon
{
public:
    NfcTagRawCommandOperationType4(QNearFieldTarget * tag);

    void run()
    {
        if (selectByName)
        {
            mId = (mTagType4->*selectByName)(mName);
        }

        if (selectById)
        {
            mId = (mTagType4->*selectById)(mFileId);
        }

        if (read)
        {
            mId = (mTagType4->*read)(mLength, mStartOffset);
        }

        if (write)
        {
            mId = (mTagType4->*write)(mDataArray, mStartOffset);
        }
        checkInvalidId();
        waitRequest();
    }

    void setRead(quint16 length, quint16 startOffset)
    {
        mLength = length;
        mStartOffset = startOffset;
        read = &QNearFieldTagType4::read;
    }

    void setWrite(const QByteArray &data, quint16 startOffset)
    {
        mDataArray = data;
        mStartOffset = startOffset;
        write = &QNearFieldTagType4::write;
    }

    void setSelectByName(const QByteArray &name)
    {
        mName = name;
        selectByName = &QNearFieldTagType4::select;
    }

    void setSelectById(quint16 fileId)
    {
        mFileId = fileId;
        selectById = &QNearFieldTagType4::select;
    }

protected:
    QNearFieldTagType4 * mTagType4;
    QNearFieldTarget::RequestId (QNearFieldTagType4::*selectByName)(const QByteArray &name);
    QNearFieldTarget::RequestId (QNearFieldTagType4::*selectById)(quint16 fileIdentifier);
    QNearFieldTarget::RequestId (QNearFieldTagType4::*write)(const QByteArray &data, quint16 startOffset);
    QNearFieldTarget::RequestId (QNearFieldTagType4::*read)(quint16 length, quint16 startOffset);

    QByteArray mDataArray;
    QByteArray mName;
    quint16 mLength;
    quint16 mFileId;
    quint16 mStartOffset;
};

NfcTagRawCommandOperationType4::NfcTagRawCommandOperationType4(QNearFieldTarget * tag):NfcTagRawCommandOperationCommon(tag)
{
    mTagType4 = qobject_cast<QNearFieldTagType4 *>(mTarget);
    QVERIFY(mTagType4);
    selectByName = 0;
    selectById = 0;
    write = 0;
    read = 0;
}


class tst_qnearfieldtagtype4 : public QObject
{
    Q_OBJECT

public:
    tst_qnearfieldtagtype4();
    void testRawAccessAndNdefAccess(const QList<QNdefMessage> &messages);

private Q_SLOTS:
    void initTestCase();
    void testRawAndNdefAccess();
    void cleanupTestCase(){}
private:
    QNfcTagTestCommon<QNearFieldTagType4> mTester;
};


tst_qnearfieldtagtype4::tst_qnearfieldtagtype4()
{
}

void tst_qnearfieldtagtype4::initTestCase()
{
}

void tst_qnearfieldtagtype4::testRawAccessAndNdefAccess(const QList<QNdefMessage> &messages)
{
    QSignalSpy okSpy(mTester.target, SIGNAL(requestCompleted(const QNearFieldTarget::RequestId&)));
    QSignalSpy errSpy(mTester.target, SIGNAL(error(QNearFieldTarget::Error, const QNearFieldTarget::RequestId&)));
    QSignalSpy ndefMessageReadSpy(mTester.target, SIGNAL(ndefMessageRead(QNdefMessage)));
    QSignalSpy ndefMessageWriteSpy(mTester.target, SIGNAL(ndefMessagesWritten()));

    int okCount = 0;
    int errCount = 0;
    int ndefReadCount = 0;
    int ndefWriteCount = 0;

    // write ndef first
    mTester.target->writeNdefMessages(messages);
    ++ndefWriteCount;
    QTRY_COMPARE(ndefMessageWriteSpy.count(), ndefWriteCount);

    // has Ndef message check
    QVERIFY(mTester.target->hasNdefMessage());

    // read NDEF with RAW command
    QByteArray resp; // correct response
    resp.append(char(0x90));
    resp.append(char(0x00));

    qDebug()<<"select NDEF application";
    QByteArray command;
    command.append(char(0xD2));
    command.append(char(0x76));
    command.append(char(0x00));
    command.append(char(0x00));
    command.append(char(0x85));
    command.append(char(0x01));
    command.append(char(0x00));
    QNearFieldTarget::RequestId id = mTester.target->select(command);
    QVERIFY(mTester.target->waitForRequestCompleted(id));
    ++okCount;
    QCOMPARE(okSpy.count(), okCount);
    QVERIFY(mTester.target->requestResponse(id).toBool());

    qDebug()<<"select CC";
    QNearFieldTarget::RequestId id1 = mTester.target->select(0xe103);
    QVERIFY(mTester.target->waitForRequestCompleted(id1));
    ++okCount;
    QCOMPARE(okSpy.count(), okCount);
    QVERIFY(mTester.target->requestResponse(id1).toBool());

    qDebug()<<"read CC";
    QNearFieldTarget::RequestId id2 = mTester.target->read(0x000F,0x0000);
    QVERIFY(mTester.target->waitForRequestCompleted(id2));
    ++okCount;
    QCOMPARE(okSpy.count(), okCount);
    QByteArray ccContent = mTester.target->requestResponse(id2).toByteArray();
    QCOMPARE(ccContent.right(2), resp);
    QCOMPARE(ccContent.count(), int(17));
    QCOMPARE(ccContent.at(1), char(0x0F));

    quint16 ndefFileId = 0;
    quint8 temp = ccContent.at(9);
    ndefFileId |= temp;
    ndefFileId <<= 8;

    temp = ccContent.at(10);
    ndefFileId |= temp;

    quint16 maxNdefLen = 0;
    temp = ccContent.at(11);
    maxNdefLen |= temp;
    maxNdefLen <<= 8;

    temp = ccContent.at(12);
    maxNdefLen |= temp;

    qDebug()<<"ndefFileId is "<<ndefFileId;
    qDebug()<<"maxNdefLen is "<<maxNdefLen;

    qDebug()<<"select NDEF";
    QNearFieldTarget::RequestId id3 = mTester.target->select(ndefFileId);
    QVERIFY(mTester.target->waitForRequestCompleted(id3));
    ++okCount;
    QCOMPARE(okSpy.count(), okCount);
    QVERIFY(mTester.target->requestResponse(id3).toBool());

    qDebug()<<"read NDEF message length";
    QNearFieldTarget::RequestId id4 = mTester.target->read(0x0002, 0x0000);
    QVERIFY(mTester.target->waitForRequestCompleted(id4));
    ++okCount;
    QCOMPARE(okSpy.count(), okCount);
    QByteArray ndefLenResult = mTester.target->requestResponse(id4).toByteArray();
    QCOMPARE(ndefLenResult.right(2), resp);

    temp = ndefLenResult.at(0);
    quint16 nLen = 0;
    nLen |= temp;
    nLen <<= 8;

    temp = ndefLenResult.at(1);
    nLen |= temp;

    qDebug()<<"ndef length is "<<nLen;
    QVERIFY( nLen > 0 );
    QVERIFY( nLen < maxNdefLen - 2 );


    qDebug()<<"read NDEF message";
    QNearFieldTarget::RequestId id5 = mTester.target->read(nLen + 2, 0x0000);
    QVERIFY(mTester.target->waitForRequestCompleted(id5));
    ++okCount;
    QCOMPARE(okSpy.count(), okCount);
    QByteArray ndefContent = mTester.target->requestResponse(id5).toByteArray();
    QCOMPARE(ndefContent.right(2), resp);


    QByteArray ndefMessageContent = ndefContent.mid(2, nLen);
    QByteArray inputNdefMessageContent = messages.at(0).toByteArray();
    QCOMPARE(ndefMessageContent, inputNdefMessageContent);

    QCOMPARE(errSpy.count(), errCount);

    // Write NDEF with raw command
    QNdefMessage message;
    QNdefNfcTextRecord textRecord;
    textRecord.setText("nfc");

    message.append(textRecord);

    QByteArray newNdefMessageContent = message.toByteArray();
    quint16 ndefMessageContentLen = newNdefMessageContent.count();
    temp = ndefMessageContentLen & 0x00FF;
    newNdefMessageContent.push_front((char)temp);
    temp = (ndefMessageContentLen >> 8) & 0x00FF;
    newNdefMessageContent.push_front((char)temp);

    // ndef file is selected
    QNearFieldTarget::RequestId id6 = mTester.target->write(newNdefMessageContent, 0);
    QVERIFY(mTester.target->waitForRequestCompleted(id6));
    ++okCount;
    QCOMPARE(okSpy.count(), okCount);
    QVERIFY(mTester.target->requestResponse(id6).toBool());

    // read ndef with ndef access
    mTester.target->readNdefMessages();
    ++ndefReadCount;
    QTRY_COMPARE(ndefMessageReadSpy.count(), ndefReadCount);

    const QNdefMessage& ndefMessage_new(ndefMessageReadSpy.first().at(0).value<QNdefMessage>());

    QCOMPARE(newNdefMessageContent.right(newNdefMessageContent.count() - 2), ndefMessage_new.toByteArray());
    QCOMPARE(errSpy.count(), errCount);
}

void tst_qnearfieldtagtype4::testRawAndNdefAccess()
{
    mTester.touchTarget();
    QNdefMessage message;
    QNdefNfcUriRecord uriRecord;
    uriRecord.setUri(QUrl("http://qt.nokia.com"));
    message.append(uriRecord);

    QList<QNdefMessage> messages;
    messages.append(message);

    testRawAccessAndNdefAccess(messages);
    mTester.removeTarget();
}

QTEST_MAIN(tst_qnearfieldtagtype4);

#include "tst_qnearfieldtagtype4.moc"

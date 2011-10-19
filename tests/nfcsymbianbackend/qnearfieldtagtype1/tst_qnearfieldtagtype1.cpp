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
#include "qnfctagtestcommon.h"
#include <qnearfieldtagtype1.h>
#include "qnfctestcommon.h"

QTCONNECTIVITY_USE_NAMESPACE

class NfcTagRawCommandOperationType1 : public NfcTagRawCommandOperationCommon
{
public:
    NfcTagRawCommandOperationType1(QNearFieldTarget * tag);

    void setReadAllOp() { readAll = &QNearFieldTagType1::readAll; }
    void setReadIdOp() { readIdentification = &QNearFieldTagType1::readIdentification; }
    void setReadByteOp(quint8 addr) { mAddr = addr; readByte = &QNearFieldTagType1::readByte; }
    void setWriteByteOp(quint8 addr, quint8 data, QNearFieldTagType1::WriteMode mode)
    {
        mAddr = addr;
        mData = data;
        mMode = mode;
        writeByte = &QNearFieldTagType1::writeByte;
    }
    void setReadSegment(quint8 addr) { mAddr = addr; readSegment = &QNearFieldTagType1::readSegment; }
    void setReadBlock(quint8 addr) { mAddr = addr; readBlock = &QNearFieldTagType1::readBlock; }
    void setWriteBlock(quint8 addr, const QByteArray & data, QNearFieldTagType1::WriteMode mode)
    {
        mAddr = addr;
        mDataArray = data;
        mMode = mode;
        writeBlock = &QNearFieldTagType1::writeBlock;
    }

    void run()
    {
        if (readAll)
        {
            mId = (tagType1->*readAll)();
        }

        if (readIdentification)
        {
            mId = (tagType1->*readIdentification)();
        }

        if (readByte)
        {
            mId = (tagType1->*readByte)(mAddr);
        }

        if (writeByte)
        {
            mId = (tagType1->*writeByte)(mAddr, mData, mMode);
        }

        if (readBlock)
        {
            mId = (tagType1->*readBlock)(mAddr);
        }

        if (writeBlock)
        {
            mId = (tagType1->*writeBlock)(mAddr, mDataArray, mMode);
        }

        if (readSegment)
        {
            mId = (tagType1->*readSegment)(mAddr);
        }
        checkInvalidId();
        waitRequest();
    }

protected:
    QNearFieldTagType1 * tagType1;
    QNearFieldTarget::RequestId (QNearFieldTagType1::*readAll)();
    QNearFieldTarget::RequestId (QNearFieldTagType1::*readIdentification)();

    QNearFieldTarget::RequestId (QNearFieldTagType1::*readByte)(quint8 address);
    QNearFieldTarget::RequestId (QNearFieldTagType1::*writeByte)(quint8 address, quint8 data, QNearFieldTagType1::WriteMode mode);

    // dynamic memory functions
    QNearFieldTarget::RequestId (QNearFieldTagType1::*readSegment)(quint8 segmentAddress);
    QNearFieldTarget::RequestId (QNearFieldTagType1::*readBlock)(quint8 blockAddress);
    QNearFieldTarget::RequestId (QNearFieldTagType1::*writeBlock)(quint8 blockAddress, const QByteArray &data,
                                 QNearFieldTagType1::WriteMode mode);
    quint8 mAddr;
    quint8 mData;
    QByteArray mDataArray;
    QNearFieldTagType1::WriteMode mMode;
};

NfcTagRawCommandOperationType1::NfcTagRawCommandOperationType1(QNearFieldTarget * tag):NfcTagRawCommandOperationCommon(tag)
{
    tagType1 = qobject_cast<QNearFieldTagType1 *>(mTarget);
    QVERIFY(tagType1);
    readAll = 0;
    readIdentification = 0;
    readByte = 0;
    writeByte = 0;
    readSegment = 0;
    readBlock = 0;
    writeBlock = 0;
}

class tst_qnearfieldtagtype1 : public QObject
{
    Q_OBJECT

public:
    tst_qnearfieldtagtype1();
    void _testRawAccessAndNdefAccess(const QList<QNdefMessage> &messages);

private Q_SLOTS:
    void initTestCase();
    void testRawAndNdefAccess();
    void testSequence();
    void testWaitInSlot();
    void testDeleteOperationBeforeAsyncRequestComplete();
    void testRemoveTagBeforeAsyncRequestComplete();
    void testCancelNdefOperation();
    void cleanupTestCase();
private:
    QNfcTagTestCommon<QNearFieldTagType1> tester;
};


tst_qnearfieldtagtype1::tst_qnearfieldtagtype1()
{
}

void tst_qnearfieldtagtype1::initTestCase()
{
}

void tst_qnearfieldtagtype1::cleanupTestCase()
{
}

void tst_qnearfieldtagtype1::testSequence()
{
    tester.touchTarget(" dynamic ");
    QByteArray uid = tester.target->uid();
    QVERIFY(!uid.isEmpty());

////////////////////////////////////////////////////////////////////////////////////////////////
    qDebug()<<"========== Test readSeg, writeBlock, async and sync mix ==========";
    OperationList rawCommandList;
    const char data[] = {0,1,2,3,4,5,6,7};
    QByteArray blockData;
    blockData.append(data, sizeof(data));
    QByteArray segmentData;

    for (int i = 0x10; i <= 0x1F; ++i)
    {
        NfcTagRawCommandOperationType1 * op1 = new NfcTagRawCommandOperationType1(tester.target);
        op1->setWriteBlock(i, blockData, QNearFieldTagType1::EraseAndWrite);
        op1->setExpectedOkSignal();
        op1->setExpectedResponse(QVariant(true));

        if (i == 0x15)
        {
            op1->setWaitOperation(NfcTagRawCommandOperationCommon::EWaitTrue);
        }
        rawCommandList.append(op1);
        segmentData.append(blockData);
    }

    NfcTagRawCommandOperationType1 * op2 = new NfcTagRawCommandOperationType1(tester.target);
    op2->setReadSegment(1);
    op2->setExpectedOkSignal();
    op2->setWaitOperation(NfcTagRawCommandOperationCommon::EWaitTrue);
    op2->setExpectedResponse(segmentData);
    rawCommandList.append(op2);

    tester.testSequence(rawCommandList);
    qDeleteAll(rawCommandList);

//////////////////////////////////////////////////////////////////////////////////////////////
    qDebug()<<"========== Test invalid readSeg, writeNE, writeBlockNE ==========";
    rawCommandList.clear();
    NfcTagRawCommandOperationType1 * op6 = new NfcTagRawCommandOperationType1(tester.target);
    op6->setReadSegment(0xf1);
    op6->setIfExpectInvalidId();
    op6->setExpectedResponse(QVariant());
    rawCommandList.append(op6);

    NfcTagRawCommandOperationType1 * op3 = new NfcTagRawCommandOperationType1(tester.target);
    op3->setWriteBlock(0x10, blockData, QNearFieldTagType1::WriteOnly);
    op3->setExpectedOkSignal();
    op3->setExpectedResponse(QVariant(true));
    rawCommandList.append(op3);

    NfcTagRawCommandOperationType1 * op4 = new NfcTagRawCommandOperationType1(tester.target);
    op4->setWriteByteOp(0x10, 0xff, QNearFieldTagType1::WriteOnly);
    op4->setExpectedOkSignal();
    op4->setExpectedResponse(QVariant(true));
    rawCommandList.append(op4);

    QList<QByteArray> cmdList;
    QByteArray command;
    command.append(char(0x1a));   // WRITE-NE
    command.append(char(0x10));  // Address
    command.append(char(0x00));  // Data
    command.append(uid.left(4)); // 4 bytes of UID
    cmdList.append(command);

    command.clear();
    command.append(char(0xff)); // Invalid command
    command.append(char(0xff));
    command.append(char(0xff));
    command.append(char(0xff));
    command.append(char(0xff));
    command.append(char(0xff));
    cmdList.append(command);

    QVariantList expectRsp;
    expectRsp.push_back(QVariant(true));
    expectRsp.push_back(QVariant());

    NfcTagSendCommandsCommon * op7 = new NfcTagSendCommandsCommon(tester.target);
    op7->SetCommandLists(cmdList);
    op7->setExpectedErrorSignal(QNearFieldTarget::InvalidParametersError);
    op7->SetExpectedResponse(expectRsp);
    op7->setWaitOperation(NfcTagRawCommandOperationCommon::EWaitFalse);
    rawCommandList.append(op7);

    cmdList.clear();
    QByteArray command1;
    command1.append(char(0x1a));   // WRITE-NE
    command1.append(char(0x10));  // Address
    command1.append(char(0x00));  // Data
    command1.append(uid.left(4)); // 4 bytes of UID
    cmdList.append(command1);
    cmdList.append(command1);

    expectRsp.clear();
    expectRsp.push_back(QVariant(true));
    expectRsp.push_back(QVariant(true));

    NfcTagSendCommandsCommon * op8 = new NfcTagSendCommandsCommon(tester.target);
    op8->SetCommandLists(cmdList);
    op8->setExpectedOkSignal();
    op8->SetExpectedResponse(expectRsp);
    rawCommandList.append(op8);

    tester.testSequence(rawCommandList);
    qDeleteAll(rawCommandList);

    tester.removeTarget();
}

void tst_qnearfieldtagtype1::testWaitInSlot()
{
    tester.touchTarget(" dynamic ");

    const char data[] = {0,1,2,3,4,5,6,7};
    QByteArray blockData;
    blockData.append(data, sizeof(data));

    NfcTagRawCommandOperationType1 * op1 = new NfcTagRawCommandOperationType1(tester.target);
    op1->setWriteBlock(0x10, blockData, QNearFieldTagType1::EraseAndWrite);
    op1->setExpectedOkSignal();
    op1->setExpectedResponse(QVariant(true));

    NfcTagRawCommandOperationType1 * op2Wait = new NfcTagRawCommandOperationType1(tester.target);
    op2Wait->setWriteBlock(0x11, blockData, QNearFieldTagType1::EraseAndWrite);
    op2Wait->setExpectedOkSignal();
    op2Wait->setExpectedResponse(QVariant(true));
    op2Wait->setWaitOperation(NfcTagRawCommandOperationCommon::EWaitTrue);

    NfcTagRawCommandOperationType1 * op3 = new NfcTagRawCommandOperationType1(tester.target);
    op3->setWriteBlock(0x12, blockData, QNearFieldTagType1::EraseAndWrite);
    op3->setExpectedOkSignal();
    op3->setExpectedResponse(QVariant(true));

    NfcTagRawCommandOperationType1 * op4WaitInSlot = new NfcTagRawCommandOperationType1(tester.target);
    op4WaitInSlot->setWriteBlock(0x13, blockData, QNearFieldTagType1::EraseAndWrite);
    op4WaitInSlot->setExpectedOkSignal();
    op4WaitInSlot->setExpectedResponse(QVariant(true));

    tester.testWaitInSlot(op1, op2Wait, op3, op4WaitInSlot);

    delete op1;
    delete op2Wait;
    delete op3;
    delete op4WaitInSlot;

    tester.removeTarget();
}

void tst_qnearfieldtagtype1::testDeleteOperationBeforeAsyncRequestComplete()
{
    tester.touchTarget(" dynamic ");
    OperationList rawCommandList;
    const char data[] = {0,1,2,3,4,5,6,7};
    QByteArray blockData;
    blockData.append(data, sizeof(data));
    QByteArray segmentData;

    for (int i = 0x10; i <= 0x1F; ++i)
    {
       NfcTagRawCommandOperationType1 * op1 = new NfcTagRawCommandOperationType1(tester.target);
       op1->setWriteBlock(i, blockData, QNearFieldTagType1::EraseAndWrite);
       op1->setExpectedOkSignal();
       op1->setExpectedResponse(QVariant(true));

       if (i == 0x15)
       {
           op1->setWaitOperation(NfcTagRawCommandOperationCommon::EWaitTrue);
       }
       rawCommandList.append(op1);
       segmentData.append(blockData);
    }

    NfcTagRawCommandOperationType1 * op2 = new NfcTagRawCommandOperationType1(tester.target);
    op2->setReadSegment(1);
    op2->setExpectedOkSignal();
    op2->setWaitOperation(NfcTagRawCommandOperationCommon::EWaitTrue);
    op2->setExpectedResponse(segmentData);
    rawCommandList.append(op2);
    tester.testDeleteOperationBeforeAsyncRequestComplete(rawCommandList);

    qDeleteAll(rawCommandList);
}

void tst_qnearfieldtagtype1::testRemoveTagBeforeAsyncRequestComplete()
{
    tester.touchTarget(" dynamic ");
    OperationList rawCommandList;
    const char data[] = {0,1,2,3,4,5,6,7};
    QByteArray blockData;
    blockData.append(data, sizeof(data));
    QByteArray segmentData;

    for (int i = 0x10; i <= 0x1F; ++i)
    {
       NfcTagRawCommandOperationType1 * op1 = new NfcTagRawCommandOperationType1(tester.target);
       op1->setWriteBlock(i, blockData, QNearFieldTagType1::EraseAndWrite);
       op1->setExpectedOkSignal();
       op1->setExpectedResponse(QVariant(true));

       rawCommandList.append(op1);
       segmentData.append(blockData);
    }

    NfcTagRawCommandOperationType1 * op2 = new NfcTagRawCommandOperationType1(tester.target);
    op2->setReadSegment(1);
    op2->setExpectedOkSignal();
    op2->setExpectedResponse(segmentData);
    rawCommandList.append(op2);

    OperationList rawCommandList1;

    segmentData.clear();

    for (int i = 0x10; i <= 0x1F; ++i)
    {
       NfcTagRawCommandOperationType1 * op1 = new NfcTagRawCommandOperationType1(tester.target);
       op1->setWriteBlock(i, blockData, QNearFieldTagType1::EraseAndWrite);
       op1->setExpectedOkSignal();
       op1->setExpectedResponse(QVariant(true));

       rawCommandList1.append(op1);
       segmentData.append(blockData);
    }

    NfcTagRawCommandOperationType1 * op3 = new NfcTagRawCommandOperationType1(tester.target);
    op3->setReadSegment(1);
    op3->setExpectedOkSignal();
    op3->setWaitOperation(NfcTagRawCommandOperationCommon::EWaitFalse);
    op3->setExpectedResponse(segmentData);
    rawCommandList1.append(op3);

    tester.testRemoveTagBeforeAsyncRequestComplete(rawCommandList, rawCommandList1);
    qDeleteAll(rawCommandList);
    qDeleteAll(rawCommandList1);
}

void tst_qnearfieldtagtype1::testCancelNdefOperation()
{
    tester.touchTarget(" static ");
    tester.testCancelNdefOperation();
}

void tst_qnearfieldtagtype1::_testRawAccessAndNdefAccess(const QList<QNdefMessage> &messages)
{
    QSignalSpy okSpy(tester.target, SIGNAL(requestCompleted(const QNearFieldTarget::RequestId&)));
    QSignalSpy errSpy(tester.target, SIGNAL(error(QNearFieldTarget::Error, const QNearFieldTarget::RequestId&)));
    QSignalSpy ndefMessageReadSpy(tester.target, SIGNAL(ndefMessageRead(QNdefMessage)));
    QSignalSpy ndefMessageWriteSpy(tester.target, SIGNAL(ndefMessagesWritten()));

    int okCount = 0;
    int errCount = 0;
    int ndefReadCount = 0;
    int ndefWriteCount = 0;
    qDebug()<<"okSpy.count()"<<okSpy.count();

    // write ndef first
    tester.target->writeNdefMessages(messages);
    ++ndefWriteCount;
    QTRY_COMPARE(ndefMessageWriteSpy.count(), ndefWriteCount);
    qDebug()<<"okSpy.count()"<<okSpy.count();

    // has Ndef message check
    QVERIFY(tester.target->hasNdefMessage());
    qDebug()<<"okSpy.count()"<<okSpy.count();

    QNearFieldTarget::RequestId id = tester.target->readAll();
    QVERIFY(tester.target->waitForRequestCompleted(id));
    QByteArray allBlocks = tester.target->requestResponse(id).toByteArray();
    ++okCount;
    QCOMPARE(okSpy.count(), okCount);

    // Verify readByte
    QNearFieldTarget::RequestId id1 = tester.target->readByte(0x08);
    QVERIFY(tester.target->waitForRequestCompleted(id1));
    quint8 cc0 = tester.target->requestResponse(id1).value<quint8>();
    QCOMPARE(cc0, quint8(0xE1));
    ++okCount;
    QCOMPARE(okSpy.count(), okCount);

    QNearFieldTarget::RequestId id2 = tester.target->readByte(0x0d);
    QVERIFY(tester.target->waitForRequestCompleted(id2));
    quint8 len = tester.target->requestResponse(id2).value<quint8>();
    QCOMPARE(int(len), messages.at(0).toByteArray().count());
    ++okCount;
    QCOMPARE(okSpy.count(), okCount);

    QCOMPARE(allBlocks.mid(16, len), messages.at(0).toByteArray());

    // Write NDEF with raw command
    QNdefMessage message;
    QNdefNfcTextRecord textRecord;
    textRecord.setText("nfc");

    message.append(textRecord);

    QByteArray newNdefMessageContent = message.toByteArray();
    quint8 ndefMessageContentLen = newNdefMessageContent.count();

    QNearFieldTarget::RequestId id3 = tester.target->writeByte(0x0d, ndefMessageContentLen);
    QVERIFY(tester.target->waitForRequestCompleted(id3));
    QVERIFY(tester.target->requestResponse(id3).toBool());
    ++okCount;
    QCOMPARE(okSpy.count(), okCount);

    quint8 block = 1;
    quint8 byte = 6;
    for (int i = 0; i < ndefMessageContentLen; ++i)
    {
        quint8 addr = (block & 0x0F);
        addr <<= 3;
        addr |= (byte & 0x07);
        QNearFieldTarget::RequestId tempId = tester.target->writeByte(addr, newNdefMessageContent.at(i));
        QVERIFY(tester.target->waitForRequestCompleted(tempId));
        QVERIFY(tester.target->requestResponse(tempId).toBool());
        byte = (7 == byte) ? 0 : (byte+1);
        block = (0 == byte) ? (block+1) : block;
        ++okCount;
        QCOMPARE(okSpy.count(), okCount);
    }

    // read ndef with ndef access
    tester.target->readNdefMessages();
    ++ndefReadCount;
    QTRY_COMPARE(ndefMessageReadSpy.count(), ndefReadCount);

    const QNdefMessage& ndefMessage_new(ndefMessageReadSpy.first().at(0).value<QNdefMessage>());

    QCOMPARE(newNdefMessageContent, ndefMessage_new.toByteArray());
    QCOMPARE(errSpy.count(), errCount);
}

void tst_qnearfieldtagtype1::testRawAndNdefAccess()
{
    tester.touchTarget(" static ");
    QByteArray uid = tester.target->uid();
    QVERIFY(!uid.isEmpty());

    QNdefMessage message;
    QNdefNfcUriRecord uriRecord;
    uriRecord.setUri(QUrl("http://qt.nokia.com"));
    message.append(uriRecord);

    QList<QNdefMessage> messages;
    messages.append(message);

    _testRawAccessAndNdefAccess(messages);
    tester.removeTarget();
}

QTEST_MAIN(tst_qnearfieldtagtype1);

#include "tst_qnearfieldtagtype1.moc"

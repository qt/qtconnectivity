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
#include <qnearfieldtagtype2.h>
#include "qnfctestcommon.h"

QTCONNECTIVITY_USE_NAMESPACE

class NfcTagRawCommandOperationType2: public NfcTagRawCommandOperationCommon
{
public:
    NfcTagRawCommandOperationType2(QNearFieldTarget * tag);

    void run()
    {
        if (readBlock)
        {
            mId = (tagType2->*readBlock)(mAddr);
        }

        if (writeBlock)
        {
            mId = (tagType2->*writeBlock)(mAddr, mDataArray);
        }

        if (selectSector)
        {
            mId = (tagType2->*selectSector)(mSector);
        }
        checkInvalidId();
        waitRequest();
    }

    void setReadBlock(quint8 blockAddress)
    {
        mAddr = blockAddress;
        readBlock = &QNearFieldTagType2::readBlock;
    }

    void setWriteBlock(quint8 blockAddress, const QByteArray &data)
    {
        mAddr = blockAddress;
        mDataArray = data;
        writeBlock = &QNearFieldTagType2::writeBlock;
    }

    void setSelectSector(quint8 sector)
    {
        mSector = sector;
        selectSector = &QNearFieldTagType2::selectSector;
    }

protected:
    QNearFieldTagType2 * tagType2;
    QNearFieldTarget::RequestId (QNearFieldTagType2::*readBlock)(quint8 blockAddress);
    QNearFieldTarget::RequestId (QNearFieldTagType2::*writeBlock)(quint8 blockAddress, const QByteArray &data);
    QNearFieldTarget::RequestId (QNearFieldTagType2::*selectSector)(quint8 sector);

    quint8 mAddr;
    quint8 mSector;
    QByteArray mDataArray;
};

NfcTagRawCommandOperationType2::NfcTagRawCommandOperationType2(QNearFieldTarget * tag):NfcTagRawCommandOperationCommon(tag)
{
    tagType2 = qobject_cast<QNearFieldTagType2 *>(mTarget);
    QVERIFY(tagType2);
    readBlock = 0;
    writeBlock = 0;
    selectSector = 0;
}

class tst_qnearfieldtagtype2: public QObject
{
    Q_OBJECT

public:
    tst_qnearfieldtagtype2();

    void testRawAccessAndNdefAccess(const QList<QNdefMessage> &messages);

private Q_SLOTS:
    void initTestCase();
    void testRawAndNdefAccess();
    void testSequence();
    void cleanupTestCase(){}
private:
    QNfcTagTestCommon<QNearFieldTagType2> tester;
};


tst_qnearfieldtagtype2::tst_qnearfieldtagtype2()
{
}

void tst_qnearfieldtagtype2::initTestCase()
{
}

void tst_qnearfieldtagtype2::testSequence()
{
    tester.touchTarget();
    QByteArray uid = tester.target->uid();
    QVERIFY(!uid.isEmpty());

    OperationList rawCommandList;
    const char data[] = {0,1,2,3};
    QByteArray blockData;
    blockData.append(data, sizeof(data));

    for (int i = 4; i < 8; ++i)
    {
        NfcTagRawCommandOperationType2 * op1 = new NfcTagRawCommandOperationType2(tester.target);
        op1->setWriteBlock(i, blockData);
        op1->setExpectedOkSignal();
        op1->setExpectedResponse(QVariant(true));

        if (i == 6)
        {
            op1->setWaitOperation(NfcTagRawCommandOperationCommon::EWaitTrue);
        }
        rawCommandList.append(op1);
    }

    QList<QByteArray> cmdList;
    QByteArray command;
    command.append(char(0xff)); // Invalid command
    command.append(char(0xff));
    command.append(char(0xff));
    command.append(char(0xff));
    command.append(char(0xff));
    command.append(char(0xff));
    cmdList.append(command);
    cmdList.append(command);

    QVariantList expectRsp;
    expectRsp.push_back(QVariant());
    expectRsp.push_back(QVariant());

    NfcTagSendCommandsCommon * op7 = new NfcTagSendCommandsCommon(tester.target);
    op7->SetCommandLists(cmdList);
    op7->setExpectedErrorSignal(QNearFieldTarget::InvalidParametersError);
    op7->SetExpectedResponse(expectRsp);
    op7->setWaitOperation(NfcTagRawCommandOperationCommon::EWaitFalse);
    rawCommandList.append(op7);

    tester.testSequence(rawCommandList);
    qDeleteAll(rawCommandList);

    tester.removeTarget();
}

void tst_qnearfieldtagtype2::testRawAccessAndNdefAccess(const QList<QNdefMessage> &messages)
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

    QNearFieldTarget::RequestId id = tester.target->readBlock(3);
    QVERIFY(tester.target->waitForRequestCompleted(id, 50000));
    ++okCount;
    QCOMPARE(okSpy.count(), okCount);
    // check if NDEF existed
    QByteArray cc = tester.target->requestResponse(id).toByteArray();
    QCOMPARE((char)(cc.at(0)), (char)0xE1);

    // try to find NDEF tlv
    QByteArray blocks;
    int NdefLen = -1;
    for(int i = 4; i < 15; i+=4)
    {
        QNearFieldTarget::RequestId id1 = tester.target->readBlock(i);
        QVERIFY(tester.target->waitForRequestCompleted(id1, 50000));
        QByteArray tlv = tester.target->requestResponse(id1).toByteArray();
        blocks.append(tlv);
        ++okCount;
        QCOMPARE(okSpy.count(), okCount);
    }

    QByteArray ndefContent;
    for (int i = 0; i < blocks.count(); ++i)
    {
        if ((blocks.at(i) == 0x03) && (i < blocks.count() - 1))
        {
            // find ndef tlv
            NdefLen = blocks.at(i+1);
            qDebug()<<"NDefLen is "<<NdefLen;
            ndefContent = blocks.mid(i+2, NdefLen);
            break;
        }
    }

    QCOMPARE(ndefContent, messages.at(0).toByteArray());
    // update the ndef meesage with raw command
    QNdefMessage message;
    QNdefNfcTextRecord textRecord;
    textRecord.setText("nfc");
    message.append(textRecord);

    QByteArray newNdefMessage = message.toByteArray();
    NdefLen = newNdefMessage.count();
    qDebug()<<"NDefLen is "<<NdefLen;
    qDebug()<<"new Ndef len is "<<NdefLen;
    newNdefMessage.push_front((char)NdefLen);
    newNdefMessage.push_front((char)0x03);

    for(int i = 0; i < 16 - NdefLen; ++i)
    {
        // append padding
        newNdefMessage.append((char)0);
    }

    for(int i = 4; i < 8; ++i)
    {
        QNearFieldTarget::RequestId id2 = tester.target->writeBlock(i, newNdefMessage.left(4));
        QVERIFY(tester.target->waitForRequestCompleted(id2));
        ++okCount;
        QCOMPARE(okSpy.count(), okCount);
        newNdefMessage.remove(0,4);
    }

    // read ndef with ndef access
    tester.target->readNdefMessages();
    ++ndefReadCount;
    QTRY_COMPARE(ndefMessageReadSpy.count(), ndefReadCount);

    const QNdefMessage& ndefMessage_new(ndefMessageReadSpy.first().at(0).value<QNdefMessage>());

    QCOMPARE(message.toByteArray(), ndefMessage_new.toByteArray());
    QCOMPARE(errSpy.count(), errCount);
}

void tst_qnearfieldtagtype2::testRawAndNdefAccess()
{
    tester.touchTarget();
    QNdefMessage message;
    QNdefNfcUriRecord uriRecord;
    uriRecord.setUri(QUrl("http://qt"));
    message.append(uriRecord);

    QList<QNdefMessage> messages;
    messages.append(message);

    testRawAccessAndNdefAccess(messages);
    tester.removeTarget();
}

QTEST_MAIN(tst_qnearfieldtagtype2);

#include "tst_qnearfieldtagtype2.moc"

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

#ifndef QNFCTAGTESTCOMMON_H
#define QNFCTAGTESTCOMMON_H

#include <qnearfieldmanager.h>
#include <qnearfieldtarget.h>
#include <QtTest/QtTest>
#include <qndefmessage.h>
#include <qndefnfctextrecord.h>
#include <qndefnfcurirecord.h>
#include <qnearfieldtagtype1.h>
#include <qnearfieldtagtype2.h>
#include <qnearfieldtagtype3.h>
#include <qnearfieldtagtype4.h>
#include "qdummyslot.h"
#include "qnfctestcommon.h"
#include "qnfctestutil.h"

template<typename TAG>
struct TagTrait
{
    static QNearFieldTarget::Type type() { return QNearFieldTarget::ProprietaryTag; }
    static QString info(){ return QString("unknow tag"); }
};

template<>
struct TagTrait<QNearFieldTagType1>
{
    static QNearFieldTarget::Type type() { return QNearFieldTarget::NfcTagType1; }
    static QString info(){ return QString("tag type 1"); }
};

template<>
struct TagTrait<QNearFieldTagType2>
{
    static QNearFieldTarget::Type type() { return QNearFieldTarget::NfcTagType2; }
    static QString info(){ return QString("tag type 2"); }
};

template<>
struct TagTrait<QNearFieldTagType3>
{
    static QNearFieldTarget::Type type() { return QNearFieldTarget::NfcTagType3; }
    static QString info(){ return QString("tag type 3"); }
};

template<>
struct TagTrait<QNearFieldTagType4>
{
    static QNearFieldTarget::Type type() { return QNearFieldTarget::NfcTagType4; }
    static QString info(){ return QString("tag type 4"); }
};



class MNfcTagOperation
{
public:
    MNfcTagOperation(QNearFieldTarget * tag)
    {
        mTarget = tag;
    }
    virtual void run() = 0;
    virtual void checkSignal() = 0;
    virtual void checkResponse() = 0;
    virtual ~MNfcTagOperation(){};
protected:
    // Not own
    QNearFieldTarget * mTarget;
};

class NfcTagRawCommandOperationCommon : public MNfcTagOperation
{
public:
    enum Wait
    {
        ENoWait,
        EWaitTrue,
        EWaitFalse
    };

    enum Signal
    {
        ENoSignal,
        EErrorSignal,
        ECompleteSignal
    };
public:
    inline NfcTagRawCommandOperationCommon(QNearFieldTarget * tag);

    ~NfcTagRawCommandOperationCommon()
    {
        delete okSpy;
        delete errSpy;
    }

    void checkSignal()
    {
        qDebug()<<"checkSignal begin";
        qDebug()<<"checking signal type "<<mSignalType;
        // check signal
        switch (mSignalType)
        {
            case EErrorSignal:
            {
                int findRequest = 0;
                for (int i = 0; i < errSpy->count(); ++i)
                {
                    if (mId == errSpy->at(i).at(1).value<QNearFieldTarget::RequestId>())
                    {
                        qDebug()<<"real err code "<<(int)(errSpy->at(i).at(0).value<QNearFieldTarget::Error>());
                        qDebug()<<"expect err code "<<(int)(mExpectedErrCode);
                        QCOMPARE(errSpy->at(i).at(0).value<QNearFieldTarget::Error>(), mExpectedErrCode);
                        ++findRequest;
                    }
                }
                qDebug()<<"find "<<findRequest<<" errSignal";
                QCOMPARE(findRequest, 1);

                findRequest = 0;
                for (int i = 0; i < okSpy->count(); ++i)
                {
                    if (mId == okSpy->at(i).at(0).value<QNearFieldTarget::RequestId>())
                    {
                        ++findRequest;
                    }
                }
                qDebug()<<"find "<<findRequest<<" okSignal";
                QCOMPARE(findRequest, 0);
                break;
            }

            case ECompleteSignal:
            {
                int findResult = 0;
                for (int i = 0; i < okSpy->count(); ++i)
                {
                    if (mId == okSpy->at(i).at(0).value<QNearFieldTarget::RequestId>())
                    {
                        ++findResult;
                    }
                }
                QCOMPARE(findResult, 1);

                findResult = 0;
                for (int i = 0; i < errSpy->count(); ++i)
                {
                    if (mId == errSpy->at(i).at(1).value<QNearFieldTarget::RequestId>())
                    {
                        ++findResult;
                    }
                }
                QTRY_COMPARE(findResult, 0);
                break;
            }

            default:
            {
                int findRequest = 0;
                for (int i = 0; i < errSpy->count(); ++i)
                {
                    if (mId == errSpy->at(i).at(1).value<QNearFieldTarget::RequestId>())
                    {
                        ++findRequest;
                    }
                }
                QTRY_COMPARE(findRequest, 0);

                findRequest = 0;
                for (int i = 0; i < okSpy->count(); ++i)
                {
                    if (mId == okSpy->at(i).at(0).value<QNearFieldTarget::RequestId>())
                    {
                        ++findRequest;
                    }
                }
                QTRY_COMPARE(findRequest, 0);
            }
        }
        qDebug()<<"checkSignal end";
    }

    void checkResponse()
    {
        qDebug()<<"checkResponse begin";
        QTRY_COMPARE(mTarget->requestResponse(mId), mExpectedResult);
        qDebug()<<"checkResponse end";
    }

    void setWaitOperation(NfcTagRawCommandOperationCommon::Wait waitOp) { mWaitOp = waitOp; }
    void setExpectedErrorSignal(QNearFieldTarget::Error error) { mExpectedErrCode = error; mSignalType = EErrorSignal; }
    void setExpectedOkSignal() { mSignalType = ECompleteSignal; }
    void setExpectedResponse(QVariant expected){ mExpectedResult = expected; }
    void setIfExpectInvalidId() { mInvalidId = true; }
    void checkInvalidId() { if (mInvalidId) QVERIFY(!mId.isValid()); }
    void waitRequest()
    {
        if (mWaitOp == NfcTagRawCommandOperationCommon::EWaitFalse)
        {
            QVERIFY(!mTarget->waitForRequestCompleted(mId));
        }
        else if (mWaitOp == NfcTagRawCommandOperationCommon::EWaitTrue)
        {
            QVERIFY(mTarget->waitForRequestCompleted(mId));
        }
    }

    QNearFieldTarget::RequestId mId;
protected:
    QSignalSpy * okSpy;
    QSignalSpy * errSpy;
    Wait mWaitOp;
    Signal mSignalType;
    QNearFieldTarget::Error mExpectedErrCode;
    QVariant mExpectedResult;
    bool mInvalidId;
};

NfcTagRawCommandOperationCommon::NfcTagRawCommandOperationCommon(QNearFieldTarget * tag) : MNfcTagOperation(tag)
   {
       mWaitOp = ENoWait;
       mSignalType = ENoSignal;
       mInvalidId = false;
       okSpy = new QSignalSpy(mTarget, SIGNAL(requestCompleted(const QNearFieldTarget::RequestId&)));
       errSpy = new QSignalSpy(mTarget, SIGNAL(error(QNearFieldTarget::Error, const QNearFieldTarget::RequestId&)));
   }

// Ndef signal count check has limit. Use Ndef operation only once so far.
class NfcTagNdefOperationCommon : public MNfcTagOperation
{
public:
    enum Signal
    {
        ENoSignal,
        EErrorSignal,
        EReadSignal,
        EWriteSignal
    };
public:
    inline NfcTagNdefOperationCommon(QNearFieldTarget * tag);

    ~NfcTagNdefOperationCommon()
    {
        delete errSpy;
        delete ndefMessageReadSpy;
        delete ndefMessagesWrittenSpy;
    }

    void run()
    {
        if (readNdefMessages)
        {
            (mTarget->*readNdefMessages)();
        }

        if (writeNdefMessages)
        {
            (mTarget->*writeNdefMessages)(mInputMessages);
        }
    }

    void checkSignal()
    {
        // check signal
        switch (mSignalType)
        {
            case NfcTagNdefOperationCommon::EErrorSignal:
            {
                for (int i = 0; i < errSpy->count(); ++i)
                {
                    if (!(errSpy->at(i).at(0).value<QNearFieldTarget::RequestId>().isValid()))
                    {
                        QTRY_COMPARE(errSpy->at(i).at(1).value<QNearFieldTarget::Error>(), mExpectedErrCode);
                        break;
                    }
                }

                break;
            }

            case NfcTagNdefOperationCommon::EReadSignal:
            {
                QVERIFY(ndefMessageReadSpy->count()>0);
                for (int i = 0; i < ndefMessageReadSpy->count(); ++i)
                {
                    mReceivedMessages.push_back(ndefMessageReadSpy->at(i).at(0).value<QNdefMessage>());
                }
                break;
            }

            case NfcTagNdefOperationCommon::EWriteSignal:
            {
                QTRY_COMPARE(ndefMessagesWrittenSpy->count(), 1);
                break;
            }

            default:
            {
                QTRY_COMPARE(errSpy->count(), 0);
                QTRY_COMPARE(ndefMessageReadSpy->count(), 0);
                QTRY_COMPARE(ndefMessagesWrittenSpy->count(), 0);
            }
        }
    }
    void checkResponse()
    {
        if (!mExpectedMessages.isEmpty())
        {
            QTRY_COMPARE(mReceivedMessages, mExpectedMessages);
        }
    }

    void setExpectedErrorSignal(QNearFieldTarget::Error error) { mExpectedErrCode = error; mSignalType = NfcTagNdefOperationCommon::EErrorSignal; }
    void setExpectedNdefReadSignal() { mSignalType = NfcTagNdefOperationCommon::EReadSignal; }
    void setExpectedNdefWriteSignal() { mSignalType = NfcTagNdefOperationCommon::EWriteSignal; }

    void setReadNdefOperation() { readNdefMessages = &QNearFieldTarget::readNdefMessages; }
    void setWriteNdefOperation(const QList<QNdefMessage> &messages) { mInputMessages = messages; writeNdefMessages = &QNearFieldTarget::writeNdefMessages; }

protected:
    QSignalSpy * errSpy;
    QSignalSpy * ndefMessageReadSpy;
    QSignalSpy * ndefMessagesWrittenSpy;
    NfcTagNdefOperationCommon::Signal mSignalType;
    QNearFieldTarget::Error mExpectedErrCode;
    QList<QNdefMessage> mInputMessages;
    QList<QNdefMessage> mExpectedMessages;
    QList<QNdefMessage> mReceivedMessages;
    QNearFieldTarget::RequestId (QNearFieldTarget::*readNdefMessages)();
    QNearFieldTarget::RequestId (QNearFieldTarget::*writeNdefMessages)(const QList<QNdefMessage> &messages);
};

NfcTagNdefOperationCommon::NfcTagNdefOperationCommon(QNearFieldTarget * tag) : MNfcTagOperation(tag)
{
    QSignalSpy * errSpy = new QSignalSpy(mTarget, SIGNAL(error(QNearFieldTarget::Error, const QNearFieldTarget::RequestId&)));
    QSignalSpy * ndefMessageReadSpy = new QSignalSpy(mTarget, SIGNAL(ndefMessageRead(QNdefMessage)));
    QSignalSpy * ndefMessagesWrittenSpy = new QSignalSpy(mTarget, SIGNAL(ndefMessagesWritten()));
    mSignalType = ENoSignal;
    readNdefMessages = 0;
    writeNdefMessages = 0;
}

class NfcTagSendCommandsCommon : public NfcTagRawCommandOperationCommon
{
public:
    inline NfcTagSendCommandsCommon(QNearFieldTarget * tag);
public:
    void run()
    {
        mId = mTarget->sendCommands(mCommands);
        checkInvalidId();
        waitRequest();
    }
    void checkResponse()
    {
        QTRY_COMPARE(mTarget->requestResponse(mId).value<QVariantList>(), mExpectedResponse);
    }
public:
    void SetCommandLists(QList<QByteArray> commands) { mCommands = commands; }
    void SetExpectedResponse(QVariantList response) { mExpectedResponse = response; }
protected:
    QList<QByteArray> mCommands;
    QVariantList mExpectedResponse;
};

NfcTagSendCommandsCommon::NfcTagSendCommandsCommon(QNearFieldTarget * tag) : NfcTagRawCommandOperationCommon(tag)
{
}

typedef QList<MNfcTagOperation *> OperationList;

Q_DECLARE_METATYPE(QNearFieldTarget*)
Q_DECLARE_METATYPE(QNearFieldTarget::Type)
Q_DECLARE_METATYPE(QNearFieldTarget::AccessMethod)
Q_DECLARE_METATYPE(MNfcTagOperation *)
Q_DECLARE_METATYPE(OperationList)

template <typename TAG>
class QNfcTagTestCommon
{
public:
    QNfcTagTestCommon();
    ~QNfcTagTestCommon();
    void touchTarget(QString info = QString(""));
    void removeTarget();
    void testSequence(OperationList& operations);
    void testWaitInSlot(MNfcTagOperation * op1, NfcTagRawCommandOperationCommon * op2WithWait, MNfcTagOperation * op3, MNfcTagOperation * waitOpInSlot);
    void testDeleteOperationBeforeAsyncRequestComplete(OperationList& operations);
    void testRemoveTagBeforeAsyncRequestComplete(OperationList& operations1, OperationList& operations2);
    void testCancelNdefOperation();
public:
    QNearFieldManager manager;
    TAG* target;
};

template<typename TAG>
QNfcTagTestCommon<TAG>::QNfcTagTestCommon()
{
    target = 0;
    qRegisterMetaType<QNdefMessage>("QNdefMessage");
    qRegisterMetaType<QNearFieldTarget *>("QNearFieldTarget*");
    qRegisterMetaType<QNearFieldTarget::Error>("QNearFieldTarget::Error");
    qRegisterMetaType<QNearFieldTarget::RequestId>("QNearFieldTarget::RequestId");
    qRegisterMetaType<MNfcTagOperation *>("MNfcTagOperation *");
    qRegisterMetaType<OperationList>("OperatoinList");
}


template<typename TAG>
QNfcTagTestCommon<TAG>::~QNfcTagTestCommon()
{
    if (target)
    {
        delete target;
    }
}

template<typename TAG>
void QNfcTagTestCommon<TAG>::touchTarget(QString info)
{
    if (target)
    {
        delete target;
        target = 0;
    }
    QSignalSpy targetDetectedSpy(&manager, SIGNAL(targetDetected(QNearFieldTarget*)));
    QSignalSpy targetLostSpy(&manager, SIGNAL(targetLost(QNearFieldTarget*)));

    qDebug()<<"start detect tag type "<<TagTrait<TAG>::type()<<endl;
    manager.startTargetDetection(TagTrait<TAG>::type());

    QString hint("please touch ");
    hint += TagTrait<TAG>::info();
    hint += info;
    QNfcTestUtil::ShowAutoMsg(hint, &targetDetectedSpy);

    QTRY_VERIFY(!targetDetectedSpy.isEmpty());
    target = qobject_cast<TAG *>(targetDetectedSpy.at(targetDetectedSpy.count()-1).at(0).value<QNearFieldTarget *>());
    // make sure target can be detected
    QVERIFY(target);

    // make sure target uid is not empty
    QVERIFY(!target->uid().isEmpty());

    QCOMPARE(target->type(), TagTrait<TAG>::type());
}

template<typename TAG>
void QNfcTagTestCommon<TAG>::removeTarget()
{
    QVERIFY(target);

    QSignalSpy targetLostSpy(&manager, SIGNAL(targetLost(QNearFieldTarget*)));
    QSignalSpy disconnectedSpy(target, SIGNAL(disconnected()));

    QNfcTestUtil::ShowAutoMsg("please remove the tag", &targetLostSpy);
    QTRY_VERIFY(!targetLostSpy.isEmpty());

    TAG *lostTarget = qobject_cast<TAG*>(targetLostSpy.first().at(0).value<QNearFieldTarget *>());

    QCOMPARE(target, lostTarget);

    QVERIFY(!disconnectedSpy.isEmpty());

    manager.stopTargetDetection();
}

template<typename TAG>
void QNfcTagTestCommon<TAG>::testSequence(OperationList& operations)
{
    qDebug()<<"_testSequence begin";
    for (int i = 0; i < operations.count(); ++i)
    {
        operations.at(i)->run();
    }

    QTest::qWait(10000);
    for (int i = 0; i < operations.count(); ++i)
    {
        operations.at(i)->checkSignal();
        operations.at(i)->checkResponse();
    }

    qDebug()<<"_testSequence end";
}

template <typename TAG>
void QNfcTagTestCommon<TAG>::testWaitInSlot(MNfcTagOperation * op1, NfcTagRawCommandOperationCommon * op2WithWait, MNfcTagOperation * op3, MNfcTagOperation * waitOpInSlot)
{
    QDummySlot waitSlot;
    QObject::connect(target, SIGNAL(requestCompleted(const QNearFieldTarget::RequestId&)),
                     &waitSlot, SLOT(requestCompletedHandling(const QNearFieldTarget::RequestId&)));
    QObject::connect(target, SIGNAL(error(QNearFieldTarget::Error, const QNearFieldTarget::RequestId&)),
                     &waitSlot, SLOT(errorHandling(QNearFieldTarget::Error, const QNearFieldTarget::RequestId&)));

    waitSlot.iOp = waitOpInSlot;

    op1->run();
    op2WithWait->run();
    QVERIFY(!target->waitForRequestCompleted(op2WithWait->mId));
    op3->run();

    QTest::qWait(10000);
    op1->checkSignal();
    op1->checkResponse();

    op2WithWait->checkSignal();
    op2WithWait->checkResponse();

    op3->checkSignal();
    op3->checkResponse();
}

template <typename TAG>
void QNfcTagTestCommon<TAG>::testDeleteOperationBeforeAsyncRequestComplete(OperationList& operations)
{
    for (int i = 0; i < operations.count(); ++i)
    {
        operations.at(i)->run();
    }
    delete target;
    target = 0;

    QNfcTestUtil::ShowMessage("Remove tag and press ok");
}

template <typename TAG>
void QNfcTagTestCommon<TAG>::testRemoveTagBeforeAsyncRequestComplete(OperationList& operations1, OperationList& operations2)
{
    for (int i = 0; i < operations1.count(); ++i)
    {
        operations1.at(i)->run();
    }

    QSignalSpy targetLostSpy(&manager, SIGNAL(targetLost(QNearFieldTarget*)));

    QNfcTestUtil::ShowAutoMsg("please remove the tag", &targetLostSpy);
    QTRY_VERIFY(!targetLostSpy.isEmpty());

    for (int i = 0; i < operations2.count(); ++i)
    {
        operations2.at(i)->run();
    }
    delete target;
    target = 0;
}

template<typename TAG>
void QNfcTagTestCommon<TAG>::testCancelNdefOperation()
{
    target->readNdefMessages();
    delete target;
    target = 0;

    QNfcTestUtil::ShowMessage("please remove tag and press ok");

    touchTarget();

    QNdefMessage message;
    QNdefNfcTextRecord textRecord;
    textRecord.setText(QLatin1String("nfc"));

    message.append(textRecord);

    QList<QNdefMessage> messages;
    messages.append(message);

    target->writeNdefMessages(messages);
    delete target;
    target = 0;

    QNfcTestUtil::ShowMessage("please remove tag and press ok");
}
#endif // QNFCTAGTESTCOMMON_H

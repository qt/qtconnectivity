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

#include "nearfieldtagimplcommon_symbian.h"


bool QNearFieldTagImplCommon::DoReadNdefMessages(MNearFieldNdefOperationCallback * const aCallback)
{
    BEGIN
    int error = KErrGeneral;
    CNearFieldNdefTarget * ndefTarget = mTag->CastToNdefTarget();
    if (ndefTarget)
    {
        LOG("switched to ndef connection");
        ndefTarget->SetNdefOperationCallback(aCallback);
        mMessageList.Reset();
        error = ndefTarget->ndefMessages(mMessageList);
        LOG("error code is"<<error);
    }

    if (error != KErrNone)
    {
        aCallback->ReadComplete(error, 0);
    }
    END
    return (error == KErrNone);
}


bool QNearFieldTagImplCommon::DoSetNdefMessages(const QList<QNdefMessage> &messages, MNearFieldNdefOperationCallback * const aCallback)
{
    BEGIN
    int error = KErrGeneral;
    CNearFieldNdefTarget * ndefTarget = mTag->CastToNdefTarget();

    if (ndefTarget)
    {
        ndefTarget->SetNdefOperationCallback(aCallback);
        if (ndefTarget)
        {
            mInputMessageList.ResetAndDestroy();
            TRAP( error,
                for (int i = 0; i < messages.count(); ++i)
                {
                    mInputMessageList.Append(QNFCNdefUtility::QNdefMsg2CNdefMsgL(messages.at(i)));
                }
            )

            if (error == KErrNone)
            {
                ndefTarget->setNdefMessages(mInputMessageList);
            }
        }
    }
    if (error != KErrNone)
    {
        aCallback->WriteComplete(error);
    }
    END
    return (error == KErrNone);
}


bool QNearFieldTagImplCommon::DoHasNdefMessages()
{
    BEGIN

    LOG("use async request to check ndef message");
    NearFieldTagNdefRequest * readNdefRequest = new NearFieldTagNdefRequest(*this);

    QNearFieldTarget::RequestId requestId;

    if (readNdefRequest)
    {
        readNdefRequest->SetRequestId(requestId);
        readNdefRequest->SetNdefRequestType(NearFieldTagNdefRequest::EReadRequest);

        if (!_isProcessingRequest())
        {
            // issue the request
            LOG("the request will be issued at once");
            mCurrentRequest = readNdefRequest;

            mPendingRequestList.append(readNdefRequest);
            readNdefRequest->IssueRequest();
        }
        else
        {
            mPendingRequestList.append(readNdefRequest);
        }

        readNdefRequest->WaitRequestCompletedNoSignal(5000);
        if (mMessageList.Count() == 0)
        {
            END
            return false;
        }
        else
        {
            mMessageList.Reset();
            END
            return true;
        }
    }
    else
    {
        LOG("unexpect error to create async request");
        END
        return false;
    }
}


bool QNearFieldTagImplCommon::DoSendCommand(const QByteArray& command, MNearFieldTagOperationCallback * const aCallback, bool deferred)
{
    BEGIN
    int error = (mResponse.MaxLength() == 0) ? KErrGeneral : KErrNone;

    if ((KErrNone == error) && (command.count() > 0))
    {
        CNearFieldTag * tag = mTag->CastToTag();

        if (tag)
        {
            tag->SetTagOperationCallback(aCallback);
            TPtrC8 cmd = QNFCNdefUtility::QByteArray2TPtrC8(command);
            mResponse.Zero();
            error = tag->RawModeAccess(cmd, mResponse, mTimeout);
        }
        else
        {
            error = KErrGeneral;
        }
    }
    else
    {
        if (deferred)
        {
            aCallback->CommandComplete(error);
        }
    }
    END
    return (error == KErrNone);
}


bool QNearFieldTagImplCommon::IssueNextRequest(QNearFieldTarget::RequestId aId)
{
    BEGIN
    // find the request after current request
    int index = mPendingRequestList.indexOf(mCurrentRequest);
    LOG("index = "<<index);
    if ((index < 0) || (index == mPendingRequestList.count() - 1))
    {
        // no next request
        mCurrentRequest = 0;
        END
        return false;
    }
    else
    {
        if (aId == mCurrentRequest->RequestID())
        {
            mCurrentRequest = mPendingRequestList.at(index + 1);
            mCurrentRequest->IssueRequest();
        }
        else
        {
            LOG("re-entry happened");
        }

        END
        return true;
    }
}


void QNearFieldTagImplCommon::RemoveRequestFromQueue(QNearFieldTarget::RequestId aId)
{
    BEGIN
    for(int i = 0; i < mPendingRequestList.count(); ++i)
    {
        MNearFieldTagAsyncRequest * request = mPendingRequestList.at(i);
        if (request->RequestID() == aId)
        {
            LOG("remove request id");
            mPendingRequestList.removeAt(i);
            break;
        }
    }
    END
}


QNearFieldTarget::RequestId QNearFieldTagImplCommon::AllocateRequestId()
{
    BEGIN
    QNearFieldTarget::RequestIdPrivate * p = new QNearFieldTarget::RequestIdPrivate;
    QNearFieldTarget::RequestId id(p);
    END
    return id;
}

QNearFieldTagImplCommon::QNearFieldTagImplCommon(CNearFieldNdefTarget *tag) : mTag(tag)
{
    mCurrentRequest = 0;
}


QNearFieldTagImplCommon::~QNearFieldTagImplCommon()
{
    BEGIN
    LOG("pending request count is "<<mPendingRequestList.count());
    for (int i = 0; i < mPendingRequestList.count(); ++i)
    {
        delete mPendingRequestList[i];
    }

    mPendingRequestList.clear();
    mCurrentRequest = 0;

    delete mTag;

    mMessageList.Close();
    mInputMessageList.ResetAndDestroy();
    mInputMessageList.Close();

    mResponse.Close();
    END
}


bool QNearFieldTagImplCommon::_hasNdefMessage()
{
    return DoHasNdefMessages();
}


QNearFieldTarget::RequestId QNearFieldTagImplCommon::_ndefMessages()
{
    BEGIN
    NearFieldTagNdefRequest * readNdefRequest = new NearFieldTagNdefRequest(*this);
    QNearFieldTarget::RequestId requestId = AllocateRequestId();

    if (readNdefRequest)
    {
        LOG("read ndef request created");
        readNdefRequest->SetRequestId(requestId);
        readNdefRequest->SetNdefRequestType(NearFieldTagNdefRequest::EReadRequest);

        if (!_isProcessingRequest())
        {
            LOG("the request will be issued at once");
            // issue the request
            mCurrentRequest = readNdefRequest;
            mPendingRequestList.append(readNdefRequest);
            readNdefRequest->IssueRequest();
        }
        else
        {
            mPendingRequestList.append(readNdefRequest);
        }
    }
    else
    {
        EmitError(KErrNoMemory, requestId);
    }
    END

    return requestId;
}


QNearFieldTarget::RequestId QNearFieldTagImplCommon::_setNdefMessages(const QList<QNdefMessage> &messages)
{
    BEGIN
    NearFieldTagNdefRequest * writeNdefRequest = new NearFieldTagNdefRequest(*this);
    QNearFieldTarget::RequestId requestId = AllocateRequestId();

    if (writeNdefRequest)
    {
        LOG("write ndef request created");
        writeNdefRequest->SetRequestId(requestId);
        writeNdefRequest->SetNdefRequestType(NearFieldTagNdefRequest::EWriteRequest);
        writeNdefRequest->SetInputNdefMessages(messages);

        if (!_isProcessingRequest())
        {
            // issue the request
            LOG("the request will be issued at once");
            mCurrentRequest = writeNdefRequest;
            mPendingRequestList.append(writeNdefRequest);
            writeNdefRequest->IssueRequest();
        }
        else
        {
            mPendingRequestList.append(writeNdefRequest);
        }
    }
    else
    {
        EmitError(KErrNoMemory, requestId);
    }
    END

    return requestId;
}


QNearFieldTarget::RequestId QNearFieldTagImplCommon::_sendCommand(const QByteArray &command)
{
    BEGIN
    NearFieldTagCommandRequest * rawCommandRequest = new NearFieldTagCommandRequest(*this);
    QNearFieldTarget::RequestId requestId = AllocateRequestId();

    if (rawCommandRequest)
    {
        LOG("send command request created");
        rawCommandRequest->SetInputCommand(command);
        rawCommandRequest->SetRequestId(requestId);
        rawCommandRequest->SetResponseBuffer(&mResponse);

        if (!_isProcessingRequest())
        {
            // issue the request
            LOG("the request will be issued at once");
            mCurrentRequest = rawCommandRequest;

            if (rawCommandRequest->IssueRequestNoDefer())
            {
                mPendingRequestList.append(rawCommandRequest);
            }
            else
            {
                END
                return QNearFieldTarget::RequestId();
            }
        }
        else
        {
            mPendingRequestList.append(rawCommandRequest);
        }
    }
    else
    {
        END
        return QNearFieldTarget::RequestId();
    }
    END
    return requestId;
}


QNearFieldTarget::RequestId QNearFieldTagImplCommon::_sendCommands(const QList<QByteArray> &commands)
{
    BEGIN
    NearFieldTagCommandsRequest * rawCommandsRequest = new NearFieldTagCommandsRequest(*this);
    QNearFieldTarget::RequestId requestId = AllocateRequestId();

    if (rawCommandsRequest)
    {
        LOG("send commands request created");
        rawCommandsRequest->SetInputCommands(commands);
        rawCommandsRequest->SetRequestId(requestId);
        rawCommandsRequest->SetResponseBuffer(&mResponse);

        if (!_isProcessingRequest())
        {
            // issue the request
            LOG("the request will be issued at once");
            mCurrentRequest = rawCommandsRequest;

            if (rawCommandsRequest->IssueRequestNoDefer())
            {
                mPendingRequestList.append(rawCommandsRequest);
            }
            else
            {
                END
                return QNearFieldTarget::RequestId();
            }
        }
        else
        {
            mPendingRequestList.append(rawCommandsRequest);
        }
    }
    else
    {
        END
        return QNearFieldTarget::RequestId();
    }
    END
    return requestId;
}


QByteArray QNearFieldTagImplCommon::_uid() const
{
    BEGIN
    if (mUid.isEmpty())
    {
        mUid = QNFCNdefUtility::TDesC2QByteArray(mTag->Uid());
        LOG(mUid);
    }
    END
    return mUid;
}


bool QNearFieldTagImplCommon::_isProcessingRequest() const
{
    BEGIN
    bool result = mPendingRequestList.count() > 0;
    LOG(result);
    END
    return result;

}


bool QNearFieldTagImplCommon::_waitForRequestCompleted(const QNearFieldTarget::RequestId &id, int msecs)
{
    BEGIN
    int index = -1;
    for (int i = 0; i < mPendingRequestList.count(); ++i)
    {
        if (id == mPendingRequestList.at(i)->RequestID())
        {
            index = i;
            break;
        }
    }

    if (index < 0)
    {
        // request ID is not in pending list. So maybe it is already completed.
        END
        return false;
    }

    MNearFieldTagAsyncRequest * request = mPendingRequestList.at(index);
    LOG("get the request from pending request list");
    END
    return request->WaitRequestCompleted(msecs);
}


bool QNearFieldTagImplCommon::_waitForRequestCompletedNoSignal(const QNearFieldTarget::RequestId &id, int msecs)
{
    BEGIN
    int index = -1;
    for (int i = 0; i < mPendingRequestList.count(); ++i)
    {
        if (id == mPendingRequestList.at(i)->RequestID())
        {
            index = i;
            break;
        }
    }

    if (index < 0)
    {
        // request ID is not in pending list. So either it may not be issued, or has already completed
        END
        return false;
    }

    MNearFieldTagAsyncRequest * request = mPendingRequestList.at(index);
    LOG("get the request from pending request list");
    END
    return (KErrNone == request->WaitRequestCompletedNoSignal(msecs));
}

void QNearFieldTagImplCommon::DoCancelSendCommand()
{
    BEGIN
    CNearFieldTag * tag = mTag->CastToTag();
    if (tag)
    {
        LOG("Cancel raw command operation");
        tag->SetTagOperationCallback(0);
        tag->Cancel();
    }
    END
}


void QNearFieldTagImplCommon::DoCancelNdefAccess()
{
    BEGIN
    CNearFieldNdefTarget * ndefTarget = mTag->CastToNdefTarget();
    if(ndefTarget)
    {
        LOG("Cancel ndef operation");
        ndefTarget->SetNdefOperationCallback(0);
        ndefTarget->Cancel();
    }
    END
}


QNearFieldTarget::Error QNearFieldTagImplCommon::SymbianError2QtError(int error)
{
    BEGIN
    QNearFieldTarget::Error qtError = QNearFieldTarget::InvalidParametersError;
    switch(error)
    {
        case KErrNone:
        {
            qtError = QNearFieldTarget::NoError;
            break;
        }
        case KErrNotSupported:
        {
            qtError = QNearFieldTarget::UnsupportedError;
            break;
        }
        case KErrTimedOut:
        {
            qtError = QNearFieldTarget::NoResponseError;
            break;
        }
        case KErrAccessDenied:
        case KErrEof:
        case KErrServerTerminated:
        {
            if (mCurrentRequest)
            {
                if(mCurrentRequest->Type() == MNearFieldTagAsyncRequest::ENdefRequest)
                {
                    NearFieldTagNdefRequest * req = static_cast<NearFieldTagNdefRequest *>(mCurrentRequest);
                    if (req->GetNdefRequestType() == NearFieldTagNdefRequest::EReadRequest)
                    {
                        qtError = QNearFieldTarget::NdefReadError;
                    }
                    else if (req->GetNdefRequestType() == NearFieldTagNdefRequest::EWriteRequest)
                    {
                        qtError = QNearFieldTarget::NdefWriteError;
                    }
                }
            }
            break;
        }
        case KErrTooBig:
        {
            qtError = QNearFieldTarget::TargetOutOfRangeError;
            break;
        }
        default:
        {
            break;
        }
    }
    LOG(qtError);
    END
    return qtError;
}

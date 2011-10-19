/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include "nearfieldtagcommandsrequest_symbian.h"
#include "nearfieldutility_symbian.h"
#include "nearfieldtagimplcommon_symbian.h"
#include "debug.h"

NearFieldTagCommandsRequest::NearFieldTagCommandsRequest(QNearFieldTagImplCommon& aOperator) : MNearFieldTagAsyncRequest(aOperator)
{
    iCurrentCommand = 0;
    iRequestCancelled = EFalse;
}

NearFieldTagCommandsRequest::~NearFieldTagCommandsRequest()
{
    BEGIN
    iRequestCancelled = ETrue;
    if (iRequestIssued)
    {
        iOperator.DoCancelSendCommand();
    }
    END
}

void NearFieldTagCommandsRequest::IssueRequest()
{
    BEGIN
    LOG("current command index = "<<iCurrentCommand);
    LOG("commands count = "<<iCommands.count());
    iRequestIssued = ETrue;
    if (iCurrentCommand < iCommands.count())
    {
        if (iWait && (iCurrentCommand == 0))
        {
            if (iWait->IsStarted() && !iTimer->IsActive())
            {
                // start timer here
                LOG("Start timer");
                TCallBack callback(MNearFieldTagAsyncRequest::TimeoutCallback, this);
                iTimer->Start(iMsecs, iMsecs, callback);
            }
        }
        iOperator.DoSendCommand(iCommands.at(iCurrentCommand), this);
        ++iCurrentCommand;
    }
    END
}

bool NearFieldTagCommandsRequest::IssueRequestNoDefer()
{
    BEGIN
    if (iCommands.count() > 0)
    {
        iRequestIssued = iOperator.DoSendCommand(iCommands.at(0), this, false);
        ++iCurrentCommand;
    }
    END
    return iRequestIssued;
}

void NearFieldTagCommandsRequest::ProcessResponse(TInt aError)
{
    BEGIN
    LOG("error is "<<aError);
    QByteArray result;
    if (KErrNone == aError)
    {
        result = QNFCNdefUtility::TDesC2QByteArray(*iResponse);
        LOG("result is "<<result);
        LOG("clear the buffer");
        iResponse->Zero();
        iDecodedResponses.append(iOperator.decodeResponse(iCommands.at(iCurrentCommand - 1), result));
    }
    else
    {
        // error occurs
        LOG("error occurs, append QVariant() to rest list");
        for (--iCurrentCommand; iCurrentCommand < iCommands.count(); ++iCurrentCommand)
        {
            iDecodedResponses.append(QVariant());
        }
    }
    iRequestIssued = EFalse;
    if (!iRequestCancelled && (iCurrentCommand < iCommands.count()))
    {
        LOG("issue another command in command list");
        IssueRequest();
    }
    else
    {
        // all commands finished
        LOG("all commands completed");
        MNearFieldTagAsyncRequest::ProcessResponse(aError);
    }
    END
}

void NearFieldTagCommandsRequest::HandleResponse(TInt aError)
{
    BEGIN
    iOperator.HandleResponse(iId, iDecodedResponses, aError);
    END
}

void NearFieldTagCommandsRequest::CommandComplete(TInt aError)
{
    BEGIN
    ProcessResponse(HandlePassiveCommand(aError));
    END
}

void NearFieldTagCommandsRequest::ProcessEmitSignal(TInt aError)
{
    BEGIN
    LOG(aError);
    if (aError != KErrNone)
    {
        iOperator.EmitError(aError, iId);
    }
    END
}

void NearFieldTagCommandsRequest::ProcessTimeout()
{
    BEGIN
    if (iWait)
    {
        if (iWait->IsStarted())
        {
            if (iRequestIssued)
            {
                iOperator.DoCancelSendCommand();
                iRequestCancelled = ETrue;
                iRequestIssued = EFalse;
            }
            LOG("wait timeout");
            ProcessResponse(HandlePassiveCommand(KErrTimedOut));
        }
    }
    END
}

TInt NearFieldTagCommandsRequest::HandlePassiveCommand(TInt aError)
{
    BEGIN
    TInt result = aError;
    TInt index = (iCurrentCommand == 0) ? iCurrentCommand : (iCurrentCommand - 1);
    QByteArray command = iCommands.at(index);
    // check if the command is passive ack
    if (command.count() == 6)
    {
        // it may be the select sector packet 2 command for tag type 2
        if ((command.at(1) == 0) && (command.at(2) == 0) && (command.at(3) == 0))
        {
            iResponse->Zero();
            if (KErrTimedOut == aError)
            {
                result = KErrNone;
                iResponse->Append(0x0A);
            }
            else
            {
                iResponse->Append(0x05);
            }
        }
    }
    END
    return result;
}

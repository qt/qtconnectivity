/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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
** $QT_END_LICENSE$
**
****************************************************************************/
#include "nearfieldtagcommandrequest_symbian.h"
#include "nearfieldutility_symbian.h"
#include "nearfieldtagimplcommon_symbian.h"
#include "debug.h"

NearFieldTagCommandRequest::NearFieldTagCommandRequest(QNearFieldTagImplCommon& aOperator) : MNearFieldTagAsyncRequest(aOperator)
{
}

NearFieldTagCommandRequest::~NearFieldTagCommandRequest()
{
    BEGIN
    if (iRequestIssued)
    {
        iOperator.DoCancelSendCommand();
    }
    END
}

void NearFieldTagCommandRequest::IssueRequest()
{
    BEGIN

    iOperator.DoSendCommand(iCommand, this);
    iRequestIssued = ETrue;
    if (iWait)
    {
        if (iWait->IsStarted())
        {
            // start timer here
            LOG("Start timer");
            TCallBack callback(MNearFieldTagAsyncRequest::TimeoutCallback, this);
            iTimer->Start(iMsecs, iMsecs, callback);
        }
    }

    END
}

bool NearFieldTagCommandRequest::IssueRequestNoDefer()
{
    BEGIN
    iRequestIssued = iOperator.DoSendCommand(iCommand, this, false);
    return iRequestIssued;
}

void NearFieldTagCommandRequest::CommandComplete(TInt aError)
{
    BEGIN
    iRequestIssued = EFalse;
    ProcessResponse(HandlePassiveCommand(aError));
    END
}


void NearFieldTagCommandRequest::ProcessEmitSignal(TInt aError)
{
    BEGIN
    LOG(aError);
    if (aError != KErrNone)
    {
        iOperator.EmitError(aError, iId);
    }
    END
}

void NearFieldTagCommandRequest::ProcessTimeout()
{
    if (iWait)
    {
        if (iWait->IsStarted())
        {
            if (iRequestIssued)
            {
                iOperator.DoCancelSendCommand();
                iRequestIssued = EFalse;
            }
            ProcessResponse(HandlePassiveCommand(KErrTimedOut));
        }
    }
}

void NearFieldTagCommandRequest::HandleResponse(TInt aError)
{
    BEGIN
    LOG(aError);
    if (aError == KErrNone)
    {
        iOperator.HandleResponse(iId, iCommand, iRequestResponse, iSendSignal);
    }
    END
}

TInt NearFieldTagCommandRequest::HandlePassiveCommand(TInt aError)
{
    BEGIN
    TInt result = aError;
    // check if the command is passive ack
    if (iCommand.count() == 6)
    {
        // it may be the select sector packet 2 command for tag type 2
        if ((iCommand.at(1) == 0) && (iCommand.at(2) == 0) && (iCommand.at(3) == 0))
        {
            result = KErrNone;
            if (KErrTimedOut == aError)
            {
                iResponse->Append(0x0A);
            }
            else
            {
                iResponse->Append(0x05);
            }
        }
    }
    iRequestResponse = QNFCNdefUtility::TDesC2QByteArray(*iResponse);
    END
    return result;
}

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

#include "nearfieldtagndefrequest_symbian.h"
#include "nearfieldutility_symbian.h"
#include "nearfieldtagimplcommon_symbian.h"
#include "debug.h"

NearFieldTagNdefRequest::NearFieldTagNdefRequest(QNearFieldTagImplCommon& aOperator) : MNearFieldTagAsyncRequest(aOperator)
{
}

NearFieldTagNdefRequest::~NearFieldTagNdefRequest()
{
    BEGIN
    if (iRequestIssued)
    {
        iOperator.DoCancelNdefAccess();
    }
    END
}

void NearFieldTagNdefRequest::IssueRequest()
{
    BEGIN

    iRequestIssued = ETrue;
    LOG(iType);
    switch(iType)
    {
        case EReadRequest:
        {
            iOperator.DoReadNdefMessages(this);
            break;
        }
        case EWriteRequest:
        {
            iOperator.DoSetNdefMessages(iMessages, this);
            break;
        }
        default:
        {
            iRequestIssued = EFalse;
            break;
        }
    }

    END
}

void NearFieldTagNdefRequest::ReadComplete(TInt aError, RPointerArray<CNdefMessage> * aMessage)
{
    BEGIN
    LOG(aError);
    iRequestIssued = EFalse;
    TRAP_IGNORE(
    if (aMessage != 0)
    {
        for(int i = 0; i < aMessage->Count(); ++i)
        {
        QNdefMessage message = QNFCNdefUtility::CNdefMsg2QNdefMsgL(*(*aMessage)[i]);
        iReadMessages.append(message);
        }
    }
    else
    {
        iReadMessages.clear();
    }
    ) // TRAP end
    ProcessResponse(aError);
    END
}

void NearFieldTagNdefRequest::WriteComplete(TInt aError)
{
    BEGIN
    iRequestIssued = EFalse;
    ProcessResponse(aError);
    END
}

void NearFieldTagNdefRequest::ProcessEmitSignal(TInt aError)
{
    BEGIN
    LOG("error code is "<<aError<<" request type is "<<iType);
    if (aError != KErrNone)
    {
        iOperator.EmitError(aError, iId);
    }
    else
    {
        if (EReadRequest == iType)
        {
            // since there is no error, iReadMessages can't be NULL.

            LOG("message count is "<<iReadMessages.count());
            for(int i = 0; i < iReadMessages.count(); ++i)
            {
                LOG("emit signal ndef message read");
                iOperator.EmitNdefMessageRead(iReadMessages.at(i));
            }
            iOperator.EmitRequestCompleted(iId);
        }
        else if (EWriteRequest == iType)
        {
            iOperator.EmitNdefMessagesWritten();
            //iOperator.EmitRequestCompleted(iId);
        }
    }
    END
}

void NearFieldTagNdefRequest::ProcessTimeout()
{
    if (iWait)
    {
        if (iWait->IsStarted())
        {
            if (iRequestIssued)
            {
                iOperator.DoCancelNdefAccess();
                iRequestIssued = EFalse;
            }
            ProcessResponse(KErrTimedOut);
        }
    }
}

void NearFieldTagNdefRequest::HandleResponse(TInt /*aError*/)
{
    BEGIN
    END
}

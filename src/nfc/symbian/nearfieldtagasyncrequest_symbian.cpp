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

#include "nearfieldtagasyncrequest_symbian.h"
#include "nearfieldutility_symbian.h"
#include "nearfieldtagimplcommon_symbian.h"
#include <e32std.h>
#include "debug.h"

TInt MNearFieldTagAsyncRequest::TimeoutCallback(TAny * aObj)
{
    BEGIN
    MNearFieldTagAsyncRequest * obj = static_cast<MNearFieldTagAsyncRequest *>(aObj);
    obj->ProcessTimeout();
    END
    return KErrNone;
}

MNearFieldTagAsyncRequest::MNearFieldTagAsyncRequest(QNearFieldTagImplCommon& aOperator) : iOperator(aOperator)
{
    iWait = 0;
    iTimer = 0;
    iRequestIssued = EFalse;
    iCurrentRequestResult = 0;
    iSendSignal = true;
    iRequestResult = 0;
}

MNearFieldTagAsyncRequest::~MNearFieldTagAsyncRequest()
{
    BEGIN
    if (iTimer)
    {
        LOG("delete timer");
        delete iTimer;
        iTimer = 0;
    }

    if (iWait)
    {
        if (iWait->IsStarted())
        {
            iWait->AsyncStop();
        }
        LOG("delete waiter");
        delete iWait;
    }
    END
}

void MNearFieldTagAsyncRequest::SetRequestId(QNearFieldTarget::RequestId aId)
{
    BEGIN
    iId = aId;
    END
}

QNearFieldTarget::RequestId MNearFieldTagAsyncRequest::RequestID()
{
    BEGIN
    END
    return iId;
}

bool MNearFieldTagAsyncRequest::WaitRequestCompleted(int aMsecs)
{
    BEGIN
    volatile bool result = false;
    if (iWait)
    {
        if (iWait->IsStarted())
        {
            LOG("waiter has already started");
            // the request is already waited, return false.
            return false;
        }
    }
    else
    {
        LOG("new a new waiter");
        iWait = new(ELeave) CActiveSchedulerWait();
        iCurrentRequestResult = &result;
    }

    if (iTimer)
    {
        LOG("cancel previous timer");
        iTimer->Cancel();
    }
    else
    {
        LOG("create a new timer");
        iTimer = CPeriodic::NewL(CActive::EPriorityStandard);
    }

    iMsecs = aMsecs * 1000;
    if (iRequestIssued)
    {
        // timer should be started when request is issued.
        LOG("Start timer");
        TCallBack callback(MNearFieldTagAsyncRequest::TimeoutCallback, this);
        iTimer->Start(iMsecs, iMsecs, callback);
    }
    LOG("Start waiter");
    iWait->Start();
    LOG("Waiting completed, "<<result);
    END
    return result;
}

int MNearFieldTagAsyncRequest::WaitRequestCompletedNoSignal(int aMsecs)
{
    BEGIN
    volatile int result = KErrNone;
    iSendSignal = false;
    iRequestResult = &result;
    if (iWait)
    {
        if (iWait->IsStarted())
        {
            LOG("waiter has already started");
            // the request is already waited, return false.
            return KErrInUse;
        }
    }
    else
    {
        LOG("new a new waiter");
        iWait = new(ELeave) CActiveSchedulerWait();
        iRequestResult = &result;
    }

    if (iTimer)
    {
        LOG("cancel previous timer");
        iTimer->Cancel();
    }
    else
    {
        LOG("create a new timer");
        iTimer = CPeriodic::NewL(CActive::EPriorityStandard);
    }

    iMsecs = aMsecs * 1000;
    if (iRequestIssued)
    {
        // timer should be started when request is issued.
        LOG("Start timer");
        TCallBack callback(MNearFieldTagAsyncRequest::TimeoutCallback, this);
        iTimer->Start(iMsecs, iMsecs, callback);
    }
    LOG("Start waiter");
    iWait->Start();
    LOG("Waiting completed, "<<result);
    END
    return result;
}


void MNearFieldTagAsyncRequest::ProcessResponse(TInt aError)
{
    BEGIN
    LOG("Error is "<<aError);

    iOperator.IssueNextRequest(iId);

    HandleResponse(aError);

    if (iWait)
    {
        ProcessWaitRequestCompleted(aError);
    }
    else
    {
        ProcessEmitSignal(aError);
    }

    LOG("remove the request from queue");
    iOperator.RemoveRequestFromQueue(iId);
    LOG("delete the request");
    iRequestIssued = EFalse;
    delete this;
    END
}

void MNearFieldTagAsyncRequest::ProcessWaitRequestCompleted(TInt aError)
{
    BEGIN
    if (iSendSignal)
    {
        if (iCurrentRequestResult)
        {
            (*iCurrentRequestResult) = (KErrNone == aError);
        }

        iCurrentRequestResult = 0;
        if (iTimer)
        {
            LOG("cancel timer");
            delete iTimer;
            iTimer = 0;
        }
        if (iWait)
        {
            if (iWait->IsStarted())
            {
                LOG("async stop waiter");
                iWait->AsyncStop();
            }
        }
        ProcessEmitSignal(aError);
    }
    else
    {
        // just for internal waiting operation.
        if (iRequestResult)
        {
            (*iRequestResult) = aError;
        }

        iRequestResult = 0;
        if (iTimer)
        {
            LOG("cancel timer");
            delete iTimer;
            iTimer = 0;
        }
        if (iWait)
        {
            if (iWait->IsStarted())
            {
                LOG("async stop waiter");
                iWait->AsyncStop();
            }
        }
    }

    END
}


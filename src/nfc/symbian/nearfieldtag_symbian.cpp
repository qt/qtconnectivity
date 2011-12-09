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

#include <nfctag.h>
#include <qglobal.h>
#include <nfcconnection.h>
#include "nearfieldtag_symbian.h"
#include "nearfieldtagoperationcallback_symbian.h"
#include "debug.h"
/*
    \class CNearFieldTag
    \brief The CNearFieldTag class provides ways to access tag

    \ingroup connectivity-nfc
    \inmodule QtNfc
    \internal
*/

CNearFieldTag::CNearFieldTag(MNfcTag * aNfcTag, RNfcServer& aNfcServer) :  CActive(EPriorityStandard),
                                                                           iNfcTag(aNfcTag),
                                                                           iNfcServer(aNfcServer)
    {
    CActiveScheduler::Add(this);
    }

CNearFieldTag* CNearFieldTag::NewLC(MNfcTag * aNfcTag, RNfcServer& aNfcServer)
    {
    CNearFieldTag* self = new (ELeave) CNearFieldTag(aNfcTag, aNfcServer);
    CleanupStack::PushL(self);
    return self;
    }

CNearFieldTag::~CNearFieldTag()
    {
    Cancel();
    if (iTagConnection)
    {
        if(iTagConnection->IsActivated())
        {
            CloseConnection();
        }
        delete iTagConnection;
    }

    delete iNfcTag;
    }

CNearFieldTag * CNearFieldTag::CastToTag()
    {
    BEGIN
    TInt error = KErrNone;

    if (!IsConnectionOpened())
        {
        error = OpenConnection();
        LOG("open connection, error is "<<error);
        }
    END
    return (error == KErrNone) ? const_cast<CNearFieldTag *>(this)
                               : reinterpret_cast<CNearFieldTag *>(0);
    }

TInt CNearFieldTag::OpenConnection()
    {
    BEGIN
    TInt error = iNfcTag->OpenConnection(*iTagConnection);
    LOG(error);
    END
    return error;
    }

void CNearFieldTag::CloseConnection()
    {
    BEGIN
    iNfcTag->CloseConnection(*iTagConnection);
    END
    }

TBool CNearFieldTag::IsConnectionOpened()
    {
    BEGIN
    LOG((int)iTagConnection);
    LOG("check if connection is opened");
    TBool result = iTagConnection->IsActivated();
    LOG("result is "<<result);
    END
    return result;
    }

TInt CNearFieldTag::RawModeAccess(const TDesC8& aCommand, TDes8& aResponse, TTimeIntervalMicroSeconds32& aTimeout)
    {
    BEGIN
    TInt error = KErrInUse;
    if (!IsActive())
        {
        LOG("AO is not active");
        // No ongoing request
        if (IsConnectionOpened())
            {
            LOG("Connection is open");
            error = KErrNone;
            iTagConnection->RawModeAccess(iStatus, aCommand, aResponse, aTimeout);
            SetActive();
            }
        }
    END
    return error;
    }

void CNearFieldTag::DoCancel()
    {
    BEGIN
    iTagConnection->CancelRawModeAccess();
    if (iCallback)
        {
        LOG("call back command complete with KErrCancel");
        TInt err;
        QT_TRYCATCH_ERROR(err, iCallback->CommandComplete(KErrCancel));
        Q_UNUSED(err);
        }
    END
    }

void CNearFieldTag::RunL()
    {
    BEGIN
    if (iCallback)
        {
        LOG("call back command complete with error"<<iStatus.Int());
        TInt err;
        QT_TRYCATCH_ERROR(err, iCallback->CommandComplete(iStatus.Int()));
        Q_UNUSED(err);
        }
    END
    }

void CNearFieldTag::SetTagOperationCallback(MNearFieldTagOperationCallback * const aCallback)
    {
    BEGIN
    iCallback = aCallback;
    END
    }

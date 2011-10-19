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
#include <nfctype1connection.h>
#include <nfctype2connection.h>
#include <nfctype3connection.h>
#include <iso14443connection.h>
#include <ndefconnection.h>
#include <nfctag.h>
#include <qglobal.h>
#include "nearfieldtag_symbian.h"
#include "nearfieldndeftarget_symbian.h"
#include "nearfieldtagndefoperationcallback_symbian.h"
#include "debug.h"

CNearFieldNdefTarget::CNearFieldNdefTarget(MNfcTag * aNfcTag, RNfcServer& aNfcServer) : iNfcTag(aNfcTag),
                                                                                        iNfcServer(aNfcServer),
                                                                                        iCurrentOperation(ENull)
    {
    }

CNearFieldNdefTarget* CNearFieldNdefTarget::NewLC(MNfcTag * aNfcTag, RNfcServer& aNfcServer)
    {
    CNearFieldNdefTarget* self = new (ELeave) CNearFieldNdefTarget(aNfcTag, aNfcServer);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

void CNearFieldNdefTarget::ConstructL()
    {
    iNdefConnection = CNdefConnection::NewL(iNfcServer, *this);
    }

void CNearFieldNdefTarget::SetRealTarget(CNearFieldTag * aRealTarget)
    {
    iTagConnection = aRealTarget;
    }

CNearFieldNdefTarget::~CNearFieldNdefTarget()
    {
    BEGIN
    // when connection is closed, cancel for each specific connection will be done.
    if (iNdefConnection)
        {
        if (iNdefConnection->IsActivated())
            {
            CloseConnection();
            }
        delete iNdefConnection;
        }

    // when Ndef target has a tag connection, iNfcTag ownership
    // will transfer to tag connection.
    if (iTagConnection)
        {
        delete iTagConnection;
        }
    else
        {
        delete iNfcTag;
        }
    END
    }

void CNearFieldNdefTarget::Cancel()
    {
    BEGIN
    if (ERead == iCurrentOperation)
    {
        iNdefConnection->CancelRead();
    }
    else if (EWrite == iCurrentOperation)
    {
        iNdefConnection->CancelWrite();
    }

    iCurrentOperation = ENull;
    END
    }

CNearFieldTag * CNearFieldNdefTarget::CastToTag()
    {
    BEGIN
    if (IsConnectionOpened())
        {
        LOG("Ndef connection will be closed");
        CloseConnection();
        }
    END
    return iTagConnection ? iTagConnection->CastToTag() : reinterpret_cast<CNearFieldTag *>(0);
    }

CNearFieldNdefTarget * CNearFieldNdefTarget::CastToNdefTarget()
    {
    BEGIN
    TInt error = KErrNone;
    if (iTagConnection)
        {
        LOG("Check if Tag Connection is opened");
        if (iTagConnection->IsConnectionOpened())
            {
            LOG("Close tag connection");
            iTagConnection->CloseConnection();
            }
        }

    if (!IsConnectionOpened())
        {
        LOG("Open ndef connection")
        error = OpenConnection();
        LOG("error code is"<<error);
        }
    END
    return (error == KErrNone) ? const_cast<CNearFieldNdefTarget *>(this)
                               : reinterpret_cast<CNearFieldNdefTarget *>(0);
    }

TInt CNearFieldNdefTarget::OpenConnection()
    {
    BEGIN
    END
    return iNfcTag->OpenConnection(*iNdefConnection);
    }

void CNearFieldNdefTarget::CloseConnection()
    {
    BEGIN
    END
    return iNfcTag->CloseConnection(*iNdefConnection);
    }

TBool CNearFieldNdefTarget::IsConnectionOpened()
    {
    BEGIN
    TBool result = iNdefConnection->IsActivated();
    LOG(result);
    END
    return result;
    }

const TDesC8& CNearFieldNdefTarget::Uid() const
    {
    BEGIN
    END
    return iNfcTag->Uid();
    }

void CNearFieldNdefTarget::ReadComplete( CNdefMessage* aMessage )
    {
    BEGIN
    if (iCallback)
        {
        TInt err = KErrNone;
        if (iMessages)
            {
            err = iMessages->Append(aMessage);
            LOG("append message, err = "<<err);
            }

        TInt errIgnore = KErrNone;
        QT_TRYCATCH_ERROR(errIgnore, iCallback->ReadComplete(err, iMessages));
        //TODO: consider it carefully
        //iMessages = 0;
        LOG("callback error is "<<errIgnore);
        }
    iCurrentOperation = ENull;
    LOG(iCurrentOperation);
    END
    }

void CNearFieldNdefTarget::WriteComplete()
    {
    BEGIN
    if (iCallback)
        {
        TInt errIgnore = KErrNone;
        QT_TRYCATCH_ERROR(errIgnore, iCallback->WriteComplete(KErrNone));
        LOG("callback error is "<<errIgnore);
        }
    iCurrentOperation = ENull;
    LOG(iCurrentOperation);
    END
    }

void CNearFieldNdefTarget::HandleError( TInt aError )
    {
    BEGIN
    if (iCallback)
        {
        LOG(iCurrentOperation);

        if (ERead == iCurrentOperation)
            {
            iCallback->ReadComplete(aError, iMessages);
            }
        else if (EWrite == iCurrentOperation)
            {
            iCallback->WriteComplete(aError);
            }
        //TODO: consider it carefully
        //iMessages = 0;
        }
    iCurrentOperation = ENull;
    END
    }

TInt CNearFieldNdefTarget::ndefMessages(RPointerArray<CNdefMessage>& aMessages)
    {
    BEGIN
    TInt error = KErrNone;
    LOG("iCurrentOperation = "<<iCurrentOperation);
    if (iCurrentOperation != ENull)
        {
        error = KErrInUse;
        }
    else
        {
        if (!IsConnectionOpened())
            {
            error = OpenConnection();
            LOG("Open connection, err = "<<error);
            }
        if (KErrNone == error)
            {
            LOG("begin to read message");
            iMessages = &aMessages;
            error = iNdefConnection->ReadMessage();
            LOG("read message err = "<<error);
            iCurrentOperation = (KErrNone == error) ? ERead : ENull;
            }
        }
    END
    return error;
    }

TInt CNearFieldNdefTarget::setNdefMessages(const RPointerArray<CNdefMessage>& aMessages)
    {
    BEGIN
    TInt error = KErrNone;
    CNdefMessage * message;
    LOG("iCurrentOperation = "<<iCurrentOperation);
    if (iCurrentOperation != ENull)
        {
        error = KErrInUse;
        }
    else
        {
        if (aMessages.Count() > 0)
            {
            LOG("message count > 0");
            // current only support single ndef message
            message = aMessages[0];
            if (!IsConnectionOpened())
                {
                error = OpenConnection();
                LOG("Open connection, err = "<<error);
                }
            if (KErrNone == error)
                {
                LOG("begin to write message");
                error = iNdefConnection->WriteMessage(*message);
                LOG("write message err = "<<error);
                iCurrentOperation = (KErrNone == error) ? EWrite : ENull;
                }
            }
        }
    END
    return error;
    }

void CNearFieldNdefTarget::SetNdefOperationCallback(MNearFieldNdefOperationCallback * const aCallback)
    {
    BEGIN
    iCallback = aCallback;
    END
    }


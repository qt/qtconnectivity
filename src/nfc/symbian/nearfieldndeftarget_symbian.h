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

#ifndef NEARFIELDNDEFTARGET_H
#define NEARFIELDNDEFTARGET_H

#include <e32base.h>
#include <nfcserver.h>
#include <ndefconnection.h>
#include <e32cmn.h>
#include <ndefhandler.h>

#include "debug.h"

class CNearFieldTag;
class CNdefMessage;

class CNdefConnection;
class MNfcTag;

class MNearFieldNdefOperationCallback;

class CNearFieldNdefTarget : public CBase,
                             public MNdefHandler
    {
    enum TOperation
        {
        ENull,
        ERead,
        EWrite
        };
public:
    // Cancel and destroy
    ~CNearFieldNdefTarget();

    // Two-phased constructor.
    static CNearFieldNdefTarget* NewLC(MNfcTag * aNfcTag, RNfcServer& aNfcServer);
public: // New functions
    void SetRealTarget(CNearFieldTag * aRealTarget);

    // NdefAccess
    TInt ndefMessages(RPointerArray<CNdefMessage>& aMessages);
    TInt setNdefMessages(const RPointerArray<CNdefMessage>& aMessages);
    void Cancel();

public:
    CNearFieldTag * CastToTag();
    CNearFieldNdefTarget * CastToNdefTarget();

    TInt OpenConnection();
    void CloseConnection();
    TBool IsConnectionOpened();
    const TDesC8& Uid() const;
    void SetNdefOperationCallback(MNearFieldNdefOperationCallback * const aCallback);

private:
    // C++ constructor
    CNearFieldNdefTarget(MNfcTag * aNfcTag, RNfcServer& aNfcServer);

    // Second-phase constructor
    void ConstructL();

private: // From MNdefHandler
    void ReadComplete( CNdefRecord* /*aRecord*/, CNdefRecord::TNdefMessagePart /*aPart*/ ){}
    void ReadComplete( CNdefMessage* aMessage );
    void ReadComplete( const RPointerArray<CNdefMessage>& /*aMessages*/ ){}
    void WriteComplete();
    void HandleError( TInt aError );

private:
    // own
    CNearFieldTag * iTagConnection;
    CNdefConnection * iNdefConnection;
    // own
    MNfcTag * iNfcTag;

    RNfcServer& iNfcServer;

    TOperation iCurrentOperation;

    // Not own
    MNearFieldNdefOperationCallback * iCallback;
    RPointerArray<CNdefMessage> * iMessages;
    };

#endif // NEARFIELDNDEFTARGET_H

/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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
#ifndef NEARFIELDTAGNDEFREQUEST_SYMBIAN_H
#define NEARFIELDTAGNDEFREQUEST_SYMBIAN_H

#include "nearfieldtagasyncrequest_symbian.h"
#include "nearfieldtagndefoperationcallback_symbian.h"

class NearFieldTagNdefRequest : public MNearFieldTagAsyncRequest,
                                public MNearFieldNdefOperationCallback
    {
public:
    enum TNdefRequestType
        {
        EReadRequest,
        EWriteRequest,
        ECheckRequest
        };
public:
    NearFieldTagNdefRequest(QNearFieldTagImplCommon& aOperator);
    ~NearFieldTagNdefRequest();
    void IssueRequest();
    void ProcessEmitSignal(TInt aError);
    void ProcessTimeout();
    void HandleResponse(TInt aError);
    void SetNdefRequestType(TNdefRequestType aType) { iType = aType; }
    void SetInputNdefMessages(QList<QNdefMessage> aMessages) { iMessages = aMessages; }
    TNdefRequestType GetNdefRequestType() { return iType; }
    TRequestType Type() { return ENdefRequest; }

private:
    void ReadComplete(TInt aError, RPointerArray<CNdefMessage> * aMessages);
    void WriteComplete(TInt aError);

private:
    TNdefRequestType iType;
    QList<QNdefMessage> iMessages;

    QList<QNdefMessage> iReadMessages;
    };
#endif

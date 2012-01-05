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

#ifndef NEARFIELDTAGIMPLCOMMON_SYMBIAN_H
#define NEARFIELDTAGIMPLCOMMON_SYMBIAN_H

#include <qnearfieldtarget.h>
#include <qnearfieldtarget_p.h>
#include "nearfieldtag_symbian.h"
#include "nearfieldndeftarget_symbian.h"
#include "nearfieldutility_symbian.h"
#include "nearfieldtagasyncrequest_symbian.h"

#include "nearfieldtagoperationcallback_symbian.h"
#include "nearfieldtagndefoperationcallback_symbian.h"

#include "nearfieldtagndefrequest_symbian.h"
#include "nearfieldtagcommandrequest_symbian.h"
#include "nearfieldtagcommandsrequest_symbian.h"

class QNearFieldTagImplCommon
{
public:
    bool DoReadNdefMessages(MNearFieldNdefOperationCallback * const aCallback);
    bool DoSetNdefMessages(const QList<QNdefMessage> &messages, MNearFieldNdefOperationCallback * const aCallback);
    bool DoHasNdefMessages();
    bool DoSendCommand(const QByteArray& command, MNearFieldTagOperationCallback * const aCallback, bool deferred = true);
    bool IssueNextRequest(QNearFieldTarget::RequestId aId);
    void RemoveRequestFromQueue(QNearFieldTarget::RequestId aId);
    QNearFieldTarget::RequestId AllocateRequestId();
    void DoCancelSendCommand();
    void DoCancelNdefAccess();
    QNearFieldTarget::Error SymbianError2QtError(int error);

    virtual void EmitNdefMessageRead(const QNdefMessage &message) = 0;
    virtual void EmitNdefMessagesWritten() = 0;
    virtual void EmitRequestCompleted(const QNearFieldTarget::RequestId &id) = 0;
    virtual void EmitError(int error, const QNearFieldTarget::RequestId &id) = 0;
    virtual void HandleResponse(const QNearFieldTarget::RequestId &id, const QByteArray &command, const QByteArray &response, bool emitRequestCompleted) = 0;
    virtual void HandleResponse(const QNearFieldTarget::RequestId &id, const QVariantList& response, int error) = 0;
    virtual QVariant decodeResponse(const QByteArray& command, const QByteArray& response) = 0;

public:
    QNearFieldTagImplCommon(CNearFieldNdefTarget *tag);
    virtual ~QNearFieldTagImplCommon();

protected:
    bool _hasNdefMessage();
    QNearFieldTarget::RequestId _ndefMessages();
    QNearFieldTarget::RequestId _setNdefMessages(const QList<QNdefMessage> &messages);

    void _setAccessMethods(const QNearFieldTarget::AccessMethods& accessMethods)
    {
        mAccessMethods = accessMethods;
    }

    QNearFieldTarget::AccessMethods _accessMethods() const
    {
        return mAccessMethods;
    }

    QNearFieldTarget::RequestId _sendCommand(const QByteArray &command);
    QNearFieldTarget::RequestId _sendCommands(const QList<QByteArray> &command);
    bool _waitForRequestCompleted(const QNearFieldTarget::RequestId &id, int msecs = 5000);
    bool _waitForRequestCompletedNoSignal(const QNearFieldTarget::RequestId &id, int msecs = 5000);

    QByteArray _uid() const;

    bool _isProcessingRequest() const;

protected:
    CNearFieldNdefTarget * mTag;
    QNearFieldTarget::AccessMethods mAccessMethods;
    mutable QByteArray mUid;
    RPointerArray<CNdefMessage> mMessageList;
    RBuf8 mResponse;

    QList<MNearFieldTagAsyncRequest *> mPendingRequestList;
    MNearFieldTagAsyncRequest * mCurrentRequest;

    TTimeIntervalMicroSeconds32 mTimeout;
    RPointerArray<CNdefMessage> mInputMessageList;
};

#endif

/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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


#ifndef QNEARFIELDTAGMIFARE_H
#define QNEARFIELDTAGMIFARE_H

#include "qnearfieldtarget.h"
#include "symbian/nearfieldndeftarget_symbian.h"
#include "symbian/nearfieldtag_symbian.h"
#include "symbian/nearfieldtagimpl_symbian.h"

QT_BEGIN_HEADER

QTCONNECTIVITY_BEGIN_NAMESPACE

class QNdefMessage;

class QNearFieldTagMifareSymbian : public QNearFieldTarget, private QNearFieldTagImpl<QNearFieldTagMifareSymbian>
{
    Q_OBJECT

public:
    explicit QNearFieldTagMifareSymbian(CNearFieldNdefTarget *tag, QObject *parent = 0);

    ~QNearFieldTagMifareSymbian();

    QByteArray uid() const;

    QNearFieldTarget::Type type() const { return QNearFieldTarget::MifareTag; }
    QNearFieldTarget::AccessMethods accessMethods() const
    {
        return _accessMethods();
    }

    void setAccessMethods(const QNearFieldTarget::AccessMethods& accessMethods)
    {
        _setAccessMethods(accessMethods);
    }
    // NdefAccess
    bool hasNdefMessage();
    RequestId readNdefMessages();
    RequestId writeNdefMessages(const QList<QNdefMessage> &messages);

    // TagTypeSpecificAccess
    RequestId sendCommand(const QByteArray &command);
    RequestId sendCommands(const QList<QByteArray> &commands);
    bool isProcessingCommand() const { return _isProcessingRequest(); }
    bool waitForRequestCompleted(const RequestId &id, int msecs = 5000);

    void handleTagOperationResponse(const RequestId &id, const QByteArray &command, const QByteArray &response, bool emitRequestCompleted);
    QVariant decodeResponse(const QByteArray &/*command*/, const QByteArray &response) { return response; }
signals:
    void disconnected();

    friend class QNearFieldTagImpl<QNearFieldTagMifareSymbian>;
};

QTCONNECTIVITY_END_NAMESPACE

QT_END_HEADER

#endif // QNEARFIELDTAGMIFARE_H

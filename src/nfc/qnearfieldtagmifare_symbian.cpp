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

#include <nfctag.h>
#include <QVariant>
#include "qnearfieldtagmifare_symbian_p.h"

QTNFC_BEGIN_NAMESPACE

QNearFieldTagMifareSymbian::QNearFieldTagMifareSymbian(CNearFieldNdefTarget *tag, QObject *parent)
                                : QNearFieldTarget(parent), QNearFieldTagImpl(tag)
{
}

QNearFieldTagMifareSymbian::~QNearFieldTagMifareSymbian()
{
}

QByteArray QNearFieldTagMifareSymbian::uid() const
{
    return _uid();
}

bool QNearFieldTagMifareSymbian::hasNdefMessage()
{
    return _hasNdefMessage();
}

QNearFieldTarget::RequestId QNearFieldTagMifareSymbian::readNdefMessages()
{
    return _ndefMessages();
}

QNearFieldTarget::RequestId QNearFieldTagMifareSymbian::writeNdefMessages(const QList<QNdefMessage> &messages)
{
    return _setNdefMessages(messages);
}

QNearFieldTarget::RequestId QNearFieldTagMifareSymbian::sendCommand(const QByteArray &command)
{
    return _sendCommand(command);
}

QNearFieldTarget::RequestId QNearFieldTagMifareSymbian::sendCommands(const QList<QByteArray> &commands)
{
    return _sendCommands(commands);
}

void QNearFieldTagMifareSymbian::handleTagOperationResponse(const RequestId &id, const QByteArray &command, const QByteArray &response, bool emitRequestCompleted)
{
    Q_UNUSED(command);
    QVariant decodedResponse = decodeResponse(command, response);
    setResponseForRequest(id, decodedResponse, emitRequestCompleted);
}

bool QNearFieldTagMifareSymbian::waitForRequestCompleted(const RequestId &id, int msecs)
{
    BEGIN
    END
    return _waitForRequestCompleted(id, msecs);
}
#include "moc_qnearfieldtagmifare_symbian_p.cpp"

QTNFC_END_NAMESPACE

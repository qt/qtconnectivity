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

#include <nfctag.h>
#include <QVariant>
#include "qnearfieldtagtype2_symbian_p.h"
#include <qnearfieldtarget_p.h>

//#define SYMBIAN_NEED_CRC
//#define SYMBIAN_RETURN_CRC
//#define SYMBIAN_RETURN_NO_ACK

QTNFC_BEGIN_NAMESPACE

static void OutputByteArray(const QByteArray& data)
{
    for(int i = 0; i < data.count(); ++i)
    {
        LOG("data ["<<i<<"] = "<<((quint16)(data.at(i))));
    }
}

QNearFieldTagType2Symbian::QNearFieldTagType2Symbian(CNearFieldNdefTarget *tag, QObject *parent)
                                : QNearFieldTagType2(parent), QNearFieldTagImpl(tag), mCurrentSector(0)
{
}

QNearFieldTagType2Symbian::~QNearFieldTagType2Symbian()
{
    BEGIN
    END
}

QVariant QNearFieldTagType2Symbian::decodeResponse(const QByteArray& command, const QByteArray& response)
{
    BEGIN
    OutputByteArray(response);

    QVariant result;
    switch(command.at(0))
    {
        case 0x30:
        {
            // read command
#ifdef SYMBIAN_RETURN_CRC
            result = response.left(16);
#else
            result = response;
#endif
            break;
        }
        case 0xA2:
        {
#ifdef SYMBIAN_RETURN_NO_ACK
            result = true;
#else
            // write command
            result = (response.at(0) == 0x0A);
#endif
            break;
        }
        default:
        {
#ifdef SYMBIAN_RETURN_CRC
            result = response.left(16);
#else
            result = response;
#endif
        }
    }
    END
    return result;
}

QNearFieldTarget::RequestId QNearFieldTagType2Symbian::readBlock(quint8 blockAddress)
{
    BEGIN
    QByteArray command;
    command.append(char(0x30));         // READ
    command.append(char(blockAddress)); // Block address

#ifdef SYMBIAN_NEED_CRC
    // append CRC
    quint16 crc = qNfcChecksum(command.constData(), command.count());
    command.append((unsigned char)(crc&0xFF));
    command.append((unsigned char)((crc>>8)&0xFF));
#endif
    END
    return sendCommand(command);
}

QNearFieldTarget::RequestId QNearFieldTagType2Symbian::writeBlock(quint8 blockAddress, const QByteArray &data)
{
    BEGIN
    if (data.length() != 4)
    {
        END
        return QNearFieldTarget::RequestId();
    }
    QByteArray command;
    command.append(char(0xa2));         // WRITE
    command.append(char(blockAddress)); // Block address
    command.append(data);               // Data

#ifdef SYMBIAN_NEED_CRC
    // append CRC
    quint16 crc = qNfcChecksum(command.constData(), command.count());
    command.append((unsigned char)(crc&0xFF));
    command.append((unsigned char)((crc>>8)&0xFF));
#endif

    END
    return sendCommand(command);
}

QNearFieldTarget::RequestId QNearFieldTagType2Symbian::selectSector(quint8 sector)
{
    BEGIN
    QByteArray command;
    command.append(char(0xc2));     // SECTOR SELECT (Command Packet 1)
    command.append(char(0xff));

#ifdef SYMBIAN_NEED_CRC
    // append CRC
    quint16 crc = qNfcChecksum(command.constData(), command.count());
    command.append((unsigned char)(crc&0xFF));
    command.append((unsigned char)((crc>>8)&0xFF));
#endif

    RequestId id = sendCommand(command);

    if (!_waitForRequestCompletedNoSignal(id, 1))
    {
        END
        return QNearFieldTarget::RequestId();
    }
    else
    {
        command.clear();
        command.append(char(sector));               // Sector number
        command.append(QByteArray(3, char(0x00)));  // RFU
#ifdef SYMBIAN_NEED_CRC
    // append CRC
    quint16 crc = qNfcChecksum(command.constData(), command.count());
    command.append((unsigned char)(crc&0xFF));
    command.append((unsigned char)((crc>>8)&0xFF));
#endif
        END
        return sendCommand(command);
    }
}

QNearFieldTarget::RequestId QNearFieldTagType2Symbian::sendCommand(const QByteArray &command)
{
    BEGIN
    END
    return _sendCommand(command);
}

QNearFieldTarget::RequestId QNearFieldTagType2Symbian::sendCommands(const QList<QByteArray> &commands)
{
    BEGIN
    END
    return _sendCommands(commands);
}

bool QNearFieldTagType2Symbian::hasNdefMessage()
{
    return _hasNdefMessage();
}

QNearFieldTarget::RequestId QNearFieldTagType2Symbian::readNdefMessages()
{
    BEGIN
    END
    return _ndefMessages();
}

QNearFieldTarget::RequestId QNearFieldTagType2Symbian::writeNdefMessages(const QList<QNdefMessage> &messages)
{
    BEGIN
    END
    return _setNdefMessages(messages);
}

QByteArray QNearFieldTagType2Symbian::uid() const
{
    BEGIN
    END
    return _uid();
}

void QNearFieldTagType2Symbian::handleTagOperationResponse(const RequestId &id, const QByteArray &command, const QByteArray &response, bool emitRequestCompleted)
{
    BEGIN
    Q_UNUSED(command);
    QVariant decodedResponse = decodeResponse(command, response);
    setResponseForRequest(id, decodedResponse, emitRequestCompleted);
    END
}

bool QNearFieldTagType2Symbian::waitForRequestCompleted(const RequestId &id, int msecs)
{
    BEGIN
    END
    return _waitForRequestCompleted(id, msecs);
}

#include "moc_qnearfieldtagtype2_symbian_p.cpp"

QTNFC_END_NAMESPACE

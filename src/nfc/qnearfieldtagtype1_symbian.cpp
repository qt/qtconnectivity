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

#include <nfctag.h>
#include <QVariant>
#include "symbian/nearfieldutility_symbian.h"
#include "qnearfieldtagtype1_symbian_p.h"
#include "debug.h"

//#define SYMBIAN_BUG_NOT_FIX

QTNFC_BEGIN_NAMESPACE

static void OutputByteArray(const QByteArray& data)
{
    for(int i = 0; i < data.count(); ++i)
    {
        LOG("data ["<<i<<"] = "<<((quint16)(data.at(i))));
    }
}

QVariant QNearFieldTagType1Symbian::decodeResponse(const QByteArray &command, const QByteArray &response)
{
    BEGIN
    OutputByteArray(response);
    END
    switch (command.at(0)) {
    case 0x01:  // READ
#ifndef SYMBIAN_BUG_NOT_FIX
        if (command.at(1) == response.at(0))
            return quint8(response.at(1));
        break;
#else
        return quint8(response.at(0));
#endif
    case 0x53: { // WRITE-E
#ifndef SYMBIAN_BUG_NOT_FIX
        quint8 address = command.at(1);
        quint8 data = command.at(2);
        quint8 writeAddress = response.at(0);
        quint8 writeData = response.at(1);
        return ((writeAddress == address) && (writeData == data));
#else
        quint8 data = command.at(2);

        quint8 writeData = response.at(0);
        return (writeData == data);
#endif
    }
    case 0x1a: { // WRITE-NE
#ifndef SYMBIAN_BUG_NOT_FIX
        quint8 address = command.at(1);
        quint8 data = command.at(2);
        quint8 writeAddress = response.at(0);
        quint8 writeData = response.at(1);

        return ((writeAddress == address) && ((writeData & data) == data));
#else
        quint8 data = command.at(2);
        quint8 writeData = response.at(0);
        return ((writeData & data) == data);
#endif
    }
    case 0x10: { // RSEG
#ifndef SYMBIAN_BUG_NOT_FIX
        quint8 segmentAddress = quint8(command.at(1)) >> 4;
        quint8 readSegmentAddress = quint8(response.at(0)) >> 4;
        if (readSegmentAddress == segmentAddress)
            return response.mid(1);
        break;
#else
        return response;
#endif
    }
    case 0x02: { // READ8
#ifndef SYMBIAN_BUG_NOT_FIX
        quint8 blockAddress = command.at(1);
        quint8 readBlockAddress = response.at(0);
        if (readBlockAddress == blockAddress)
            return response.mid(1);
        break;
#else
        return response;
#endif
    }
    case 0x54: { // WRITE-E8
#ifndef SYMBIAN_BUG_NOT_FIX
        quint8 blockAddress = command.at(1);
        QByteArray data = command.mid(2, 8);
        quint8 writeBlockAddress = response.at(0);
        QByteArray writeData = response.mid(1);

        return ((writeBlockAddress == blockAddress) && (writeData == data));
#else
        QByteArray data = command.mid(2, 8);

        return (response == data);
#endif
    }
    case 0x1b: { // WRITE-NE8
#ifndef SYMBIAN_BUG_NOT_FIX
        quint8 blockAddress = command.at(1);
        QByteArray data = command.mid(2, 8);
        quint8 writeBlockAddress = response.at(0);
        QByteArray writeData = response.mid(1);

        if (writeBlockAddress != blockAddress)
            return false;

        for (int i = 0; i < writeData.length(); ++i) {
            if ((writeData.at(i) & data.at(i)) != data.at(i))
                return false;
        }
        return true;
#else
        QByteArray data = command.mid(2, 8);
        QByteArray writeData = response;

        for (int i = 0; i < writeData.length(); ++i) {
            if ((writeData.at(i) & data.at(i)) != data.at(i))
                return false;
        }
        return true;
#endif
    }
    }

    return response;
}
/*
    \class QNearFieldTagType1Symbian
    \brief The QNearFieldTagType1Symbian class provides symbian backend implementation for communicating with an NFC Tag
           Type 1 tag.

    \ingroup connectivity-nfc
    \inmodule QtConnectivity
*/

/*
    Constructs a new tag type 1 near field target with \a tag and \a parent.
*/
QNearFieldTagType1Symbian::QNearFieldTagType1Symbian(CNearFieldNdefTarget *tag, QObject *parent)
                                : QNearFieldTagType1(parent), QNearFieldTagImpl(tag)
{
}

/*
    Destructor
*/
QNearFieldTagType1Symbian::~QNearFieldTagType1Symbian()
{
}

QByteArray QNearFieldTagType1Symbian::uid() const
{
    return _uid();
}

/*
    \reimp
*/
QNearFieldTarget::RequestId QNearFieldTagType1Symbian::readIdentification()
{
    BEGIN
    QByteArray command;
    command.append(char(0x78));     // RID
    command.append(char(0x00));     // Address (unused)
    command.append(char(0x00));     // Data (unused)
    command.append(uid().left(4));  // 4 bytes of UID
    END
    return sendCommand(command);
}

/*
    \reimp
*/
QNearFieldTarget::RequestId QNearFieldTagType1Symbian::readAll()
{
    BEGIN
    QByteArray command;
    command.append(char(0x00)); // RALL
    command.append(char(0x00));
    command.append(char(0x00));
    command.append(uid().left(4)); // UID
    END
    return sendCommand(command);
}

/*
    \reimp
*/
QNearFieldTarget::RequestId QNearFieldTagType1Symbian::readByte(quint8 address)
{
    BEGIN
    // MSB must be 0
    if (address & 0x80)
        return QNearFieldTarget::RequestId();

    QByteArray command;
    command.append(char(0x01));     // READ
    command.append(char(address));  // Address
    command.append(char(0x00));     // Data (unused)
    command.append(uid().left(4));  // 4 bytes of UID

    END
    return sendCommand(command);
}

/*
    \reimp
*/
QNearFieldTarget::RequestId QNearFieldTagType1Symbian::writeByte(quint8 address, quint8 data, WriteMode mode)
{
    BEGIN
    // MSB must be 0
    if (address & 0x80)
    {
        END
        return QNearFieldTarget::RequestId();
    }

    QByteArray command;

    if (mode == EraseAndWrite)
        command.append(char(0x53)); // WRITE-E
    else
        command.append(char(0x1a)); // WRITE-NE

    command.append(char(address));  // Address
    command.append(char(data));     // Data
    command.append(uid().left(4));  // 4 bytes of UID

    END
    return sendCommand(command);
}

/*
    \reimp
*/
QNearFieldTarget::RequestId QNearFieldTagType1Symbian::readSegment(quint8 segmentAddress)
{
    BEGIN
    if (segmentAddress & 0xf0)
    {
        END
        return QNearFieldTarget::RequestId();
    }

    QByteArray command;
    command.append(char(0x10));                 // RSEG
    command.append(char(segmentAddress << 4));  // Segment address
    command.append(QByteArray(8, char(0x00)));  // Data (unused)
    command.append(uid().left(4));              // 4 bytes of UIDD

    END
    return sendCommand(command);
}

/*
    \reimp
*/
QNearFieldTarget::RequestId QNearFieldTagType1Symbian::readBlock(quint8 blockAddress)
{
    BEGIN
    QByteArray command;
    command.append(char(0x02));                 // READ8
    command.append(char(blockAddress));         // Block address
    command.append(QByteArray(8, char(0x00)));  // Data (unused)
    command.append(uid().left(4));              // 4 bytes of UID

    END
    return sendCommand(command);
}

/*
    \reimp
*/
QNearFieldTarget::RequestId QNearFieldTagType1Symbian::writeBlock(quint8 blockAddress, const QByteArray &data,
                        WriteMode mode)
{
    BEGIN
    if (data.length() != 8)
    {
        END
        return QNearFieldTarget::RequestId();
    }

    QByteArray command;

    if (mode == EraseAndWrite)
        command.append(char(0x54));     // WRITE-E8
    else
        command.append(char(0x1b));     // WRITE-NE8

    command.append(char(blockAddress)); // Block address
    command.append(data);               // Data
    command.append(uid().left(4));      // 4 bytes of UID

    END
    return sendCommand(command);
}

bool QNearFieldTagType1Symbian::hasNdefMessage()
{
    return _hasNdefMessage();
}

QNearFieldTarget::RequestId QNearFieldTagType1Symbian::readNdefMessages()
{
    BEGIN
    END
    return _ndefMessages();
}

QNearFieldTarget::RequestId QNearFieldTagType1Symbian::writeNdefMessages(const QList<QNdefMessage> &messages)
{
    BEGIN
    END
    return _setNdefMessages(messages);
}

/*
    \reimp
*/
QNearFieldTarget::RequestId QNearFieldTagType1Symbian::sendCommand(const QByteArray &command)
{
    BEGIN
    END
    return _sendCommand(command);
}

/*
    \reimp
*/
QNearFieldTarget::RequestId QNearFieldTagType1Symbian::sendCommands(const QList<QByteArray> &commands)
{
    BEGIN
    END
    return _sendCommands(commands);
}

bool QNearFieldTagType1Symbian::waitForRequestCompleted(const RequestId &id, int msecs)
{
    BEGIN
    END
    return _waitForRequestCompleted(id, msecs);
}

void QNearFieldTagType1Symbian::handleTagOperationResponse(const RequestId &id, const QByteArray &command, const QByteArray &response, bool emitRequestCompleted)
{
    BEGIN
    QVariant decodedResponse;
    if (!response.isNull())
    {
        decodedResponse = decodeResponse(command, response);
    }
    else
    {
        decodedResponse = response;
    }

    setResponseForRequest(id, decodedResponse, emitRequestCompleted);
    END
}

#include "moc_qnearfieldtagtype1_symbian_p.cpp"

QTNFC_END_NAMESPACE

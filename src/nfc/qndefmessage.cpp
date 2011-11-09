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

#include "qndefmessage.h"
#include "qndefrecord_p.h"

QTNFC_BEGIN_NAMESPACE

/*!
    \class QNdefMessage
    \brief The QNdefMessage class provides an NFC NDEF message.

    \ingroup connectivity-nfc
    \inmodule QtConnectivity
    \since 5.0

    A QNdefMessage is a collection of 0 or more QNdefRecords. QNdefMessage inherits from
    QList<QNdefRecord> and therefore the standard QList functions can be used to manipulate the
    NDEF records in the message.

    NDEF messages can be parsed from a byte array conforming to the NFC Data Exchange Format
    technical specification by using the fromByteArray() static function. Conversely QNdefMessages
    can be converted into a byte array with the toByteArray() function.
*/

/*!
    \fn QNdefMessage::QNdefMessage()

    Constructs a new empty NDEF message.
*/

/*!
    \fn QNdefMessage::QNdefMessage(const QNdefRecord &record)

    Constructs a new NDEF message containing a single record \a record.
*/

/*!
    \fn QNdefMessage::QNdefMessage(const QNdefMessage &message)

    Constructs a new NDEF message that is a copy of \a message.
*/

/*!
    \fn QNdefMessage::QNdefMessage(const QList<QNdefRecord> &records)

    Constructs a new NDEF message that contains all of the records in \a records.
*/

/*!
    Returns an NDEF message parsed from the contents of \a message.

    The \a message parameter is interpreted as the raw message format defined in the NFC Data
    Exchange Format technical specification.

    If a parse error occurs an empty NDEF message is returned.
*/
QNdefMessage QNdefMessage::fromByteArray(const QByteArray &message)
{
    QNdefMessage result;

    bool seenMessageBegin = false;
    bool seenMessageEnd = false;

    QByteArray partialChunk;
    QNdefRecord record;

    QByteArray::const_iterator i = message.begin();
    while (i < message.constEnd()) {
        quint8 flags = *i;

        bool messageBegin = flags & 0x80;
        bool messageEnd = flags & 0x40;

        bool cf = flags & 0x20;
        bool sr = flags & 0x10;
        bool il = flags & 0x08;
        quint8 typeNameFormat = flags & 0x07;

        if (messageBegin && seenMessageBegin) {
            qWarning("Got message begin but already parsed some records");
            return QNdefMessage();
        } else if (!messageBegin && !seenMessageBegin) {
            qWarning("Haven't got message begin yet");
            return QNdefMessage();
        } else if (messageBegin && !seenMessageBegin) {
            seenMessageBegin = true;
        }
        if (messageEnd && seenMessageEnd) {
            qWarning("Got message end but already parsed final record");
            return QNdefMessage();
        } else if (messageEnd && !seenMessageEnd) {
            seenMessageEnd = true;
        }
        if (cf && (typeNameFormat != 0x06) && !partialChunk.isEmpty()) {
            qWarning("partial chunk not empty or typeNameFormat not 0x06 as expected");
            return QNdefMessage();
        }

        int headerLength = 1;
        headerLength += (sr) ? 1 : 4;
        headerLength += (il) ? 1 : 0;

        if (i + headerLength >= message.constEnd()) {
            qWarning("Unexpected end of message");
            return QNdefMessage();
        }

        quint8 typeLength = *(++i);

        if ((typeNameFormat == 0x06) && (typeLength != 0)) {
            qWarning("Invalid chunked data, TYPE_LENGTH != 0");
            return QNdefMessage();
        }

        quint32 payloadLength;
        if (sr)
            payloadLength = *(++i);
        else {
            payloadLength = quint8(*(++i)) << 24;
            payloadLength |= quint8(*(++i)) << 16;
            payloadLength |= quint8(*(++i)) << 8;
            payloadLength |= quint8(*(++i)) << 0;
        }

        quint8 idLength;
        if (il)
            idLength = *(++i);
        else
            idLength = 0;

        int contentLength = typeLength + payloadLength + idLength;
        if (i + contentLength >= message.constEnd()) {
            qWarning("Unexpected end of message");
            return QNdefMessage();
        }

        if ((typeNameFormat == 0x06) && (idLength != 0)) {
            qWarning("Invalid chunked data, IL != 0");
            return QNdefMessage();
        }

        if (typeNameFormat != 0x06)
            record.setTypeNameFormat(QNdefRecord::TypeNameFormat(typeNameFormat));

        if (typeLength > 0) {
            QByteArray type(++i, typeLength);
            record.setType(type);
            i += typeLength - 1;
        }

        if (idLength > 0) {
            QByteArray id(++i, idLength);
            record.setId(id);
            i += idLength - 1;
        }

        if (payloadLength > 0) {
            QByteArray payload(++i, payloadLength);


            if (cf) {
                // chunked payload, except last
                partialChunk.append(payload);
            } else if (typeNameFormat == 0x06) {
                // last chunk of chunked payload
                record.setPayload(partialChunk + payload);
                partialChunk.clear();
            } else {
                // non-chunked payload
                record.setPayload(payload);
            }

            i += payloadLength - 1;
        }

        if (!cf)
            result.append(record);

        if (!cf && seenMessageEnd)
            break;

        // move to start of next record
        ++i;
    }

    if (!seenMessageBegin && !seenMessageEnd) {
        qWarning("Malformed NDEF Message, missing begin or end.");
        return QNdefMessage();
    }

    return result;
}

/*!
    Returns true if this NDEF message is equivalent to \a other; otherwise returns false.

    An empty message (i.e. isEmpty() returns true) is equivalent to a NDEF message containing a
    single record of type QNdefRecord::Empty.
*/
bool QNdefMessage::operator==(const QNdefMessage &other) const
{
    // both records are empty
    if (isEmpty() && other.isEmpty())
        return true;

    // compare empty to really empty
    if (isEmpty() && other.count() == 1 && other.first().typeNameFormat() == QNdefRecord::Empty)
        return true;
    if (other.isEmpty() && count() == 1 && first().typeNameFormat() == QNdefRecord::Empty)
        return true;

    if (count() != other.count())
        return false;

    for (int i = 0; i < count(); ++i) {
        if (at(i) != other.at(i))
            return false;
    }

    return true;
}

/*!
    Returns the NDEF message as a byte array.

    The return value of this function conforms to the format defined in the NFC Data Exchange
    Format technical specification.
*/
QByteArray QNdefMessage::toByteArray() const
{
    // An empty message is treated as a message containing a single empty record.
    if (isEmpty())
        return QNdefMessage(QNdefRecord()).toByteArray();

    QByteArray m;

    for (int i = 0; i < count(); ++i) {
        const QNdefRecord &record = at(i);

        quint8 flags = record.typeNameFormat();

        if (i == 0)
            flags |= 0x80;
        if (i == count() - 1)
            flags |= 0x40;

        // cf (chunked records) not supported yet

        if (record.payload().length() < 255)
            flags |= 0x10;

        if (!record.id().isEmpty())
            flags |= 0x08;

        m.append(flags);
        m.append(record.type().length());

        if (flags & 0x10) {
            m.append(quint8(record.payload().length()));
        } else {
            quint32 length = record.payload().length();
            m.append(length >> 24);
            m.append(length >> 16);
            m.append(length >> 8);
            m.append(length & 0x000000ff);
        }

        if (flags & 0x08)
            m.append(record.id().length());

        if (!record.type().isEmpty())
            m.append(record.type());

        if (!record.id().isEmpty())
            m.append(record.id());

        if (!record.payload().isEmpty())
            m.append(record.payload());
    }

    return m;
}

QTNFC_END_NAMESPACE

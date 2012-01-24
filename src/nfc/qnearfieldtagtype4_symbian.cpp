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
#include <QtEndian>
#include "symbian/nearfieldutility_symbian.h"
#include "qnearfieldtagtype4_symbian_p.h"

QTNFC_BEGIN_NAMESPACE

static void OutputByteArray(const QByteArray& data)
{
    for(int i = 0; i < data.count(); ++i)
    {
        LOG("data ["<<i<<"] = "<<((quint16)(data.at(i))));
    }
}

QNearFieldTagType4Symbian::QNearFieldTagType4Symbian(CNearFieldNdefTarget *tag, QObject *parent)
                                : QNearFieldTagType4(parent), QNearFieldTagImpl(tag)
{
}

QNearFieldTagType4Symbian::~QNearFieldTagType4Symbian()
{
}

QByteArray QNearFieldTagType4Symbian::uid() const
{
    return _uid();
}

quint8 QNearFieldTagType4Symbian::version()
{
    BEGIN
    quint8 result = 0;
    QByteArray resp;
    resp.append(char(0x90));
    resp.append(char(0x00));

    QNearFieldTarget::RequestId id = selectNdefApplication();
    if (_waitForRequestCompletedNoSignal(id))
    {
        if (requestResponse(id).toByteArray().right(2) == resp)
        {
            // response is ok
            // select cc
            LOG("select cc");
            QNearFieldTarget::RequestId id1 = selectCC();
            if (_waitForRequestCompletedNoSignal(id1))
            {
                if (requestResponse(id1).toBool())
                {
                    // response is ok
                    // read cc
                    LOG("read cc");
                    QNearFieldTarget::RequestId id2 = read(0x0001,0x0002);
                    if (_waitForRequestCompletedNoSignal(id2))
                    {
                        if (requestResponse(id2).toByteArray().right(2) == resp)
                        {
                            // response is ok
                            result = requestResponse(id2).toByteArray().at(0);
                        }
                    }
                }
            }
        }
    }
    LOG("version is "<<result);
    END
    return result;
}

QVariant QNearFieldTagType4Symbian::decodeResponse(const QByteArray &command, const QByteArray &response)
{
    BEGIN
    QVariant result;

    OutputByteArray(response);
    if ((command.count() > 2) && (0x00 == command.at(0)))
    {
        if ( (0xA4 == command.at(1)) || (0xD6 == command.at(1)) )
        {
            if (response.count() >= 2)
            {
                LOG("select or write command");
                QByteArray resp = response.right(2);
                result = ((resp.at(0) == 0x90) && (resp.at(1) == 0x00));
            }
        }
        else
        {
            LOG("read command");
            result = response;
        }
    }
    END
    return result;
}

bool QNearFieldTagType4Symbian::hasNdefMessage()
{
    BEGIN
    QByteArray resp;
    resp.append(char(0x90));
    resp.append(char(0x00));

    QNearFieldTarget::RequestId id = selectNdefApplication();
    if (!_waitForRequestCompletedNoSignal(id))
    {
        LOG("select NDEF application failed");
        END
        return false;
    }

    if (!requestResponse(id).toBool())
    {
        LOG("select NDEF application response is not ok");
        END
        return false;
    }

    QNearFieldTarget::RequestId id1 = selectCC();
    if (!_waitForRequestCompletedNoSignal(id1))
    {
        LOG("select CC failed");
        END
        return false;
    }

    if (!requestResponse(id1).toBool())
    {
        LOG("select CC response is not ok");
        END
        return false;
    }

    QNearFieldTarget::RequestId id2 = read(0x000F,0x0000);
    if (!_waitForRequestCompletedNoSignal(id2))
    {
        LOG("read CC failed");
        END
        return false;
    }

    QByteArray ccContent = requestResponse(id2).toByteArray();
    if (ccContent.right(2) != resp)
    {
        LOG("read CC response is "<<ccContent.right(2));
        END
        return false;
    }

    if ((ccContent.count() != (15 + 2)) && (ccContent.at(1) != 0x0F))
    {
        LOG("CC is invalid"<<ccContent);
        END
        return false;
    }

    quint8 temp = ccContent.at(9);
    quint16 fileId = 0;
    fileId |= temp;
    fileId<<=8;

    temp = ccContent.at(10);
    fileId |= temp;

    temp = ccContent.at(11);
    quint16 maxNdefLen = 0;
    maxNdefLen |= temp;
    maxNdefLen<<=8;

    temp = ccContent.at(12);
    maxNdefLen |= temp;

    QNearFieldTarget::RequestId id3 = select(fileId);
    if (!_waitForRequestCompletedNoSignal(id3))
        {
            LOG("select NDEF failed");
            END
            return false;
        }
    if (!requestResponse(id3).toBool())
    {
        END
        return false;
    }

    QNearFieldTarget::RequestId id4 = read(0x0002, 0x0000);
    if (!_waitForRequestCompletedNoSignal(id4))
        {
            LOG("read NDEF failed");
            END
            return false;
        }
    QByteArray ndefContent = requestResponse(id4).toByteArray();
    if (ndefContent.right(2) != resp)
    {
        LOG("read NDEF response is "<<ndefContent.right(2));
        END
        return false;
    }

    if (ndefContent.count() != (2 + 2))
    {
        LOG("ndef content invalid");
        END
        return false;
    }

    temp = ndefContent.at(0);
    quint16 nLen = 0;
    nLen |= temp;
    nLen<<=8;

    temp = ndefContent.at(1);
    nLen |= temp;

    END
    return ( (nLen > 0) && (nLen < maxNdefLen -2) );
}

QNearFieldTarget::RequestId QNearFieldTagType4Symbian::readNdefMessages()
{
    return _ndefMessages();
}

QNearFieldTarget::RequestId QNearFieldTagType4Symbian::writeNdefMessages(const QList<QNdefMessage> &messages)
{
    return _setNdefMessages(messages);
}

QNearFieldTarget::RequestId QNearFieldTagType4Symbian::sendCommand(const QByteArray &command)
{
    return _sendCommand(command);
}

QNearFieldTarget::RequestId QNearFieldTagType4Symbian::sendCommands(const QList<QByteArray> &commands)
{
    return _sendCommands(commands);
}

QNearFieldTarget::RequestId QNearFieldTagType4Symbian::select(const QByteArray &name)
{
    BEGIN
    QByteArray command;
    command.append(char(0x00)); // CLA
    command.append(char(0xA4)); // INS
    command.append(char(0x04)); // P1, select by name
    command.append(char(0x00)); // First or only occurrence
    command.append(char(0x07)); // Lc
    command.append(name);
    END
    return _sendCommand(command);
}

QNearFieldTarget::RequestId QNearFieldTagType4Symbian::select(quint16 fileIdentifier)
{
    BEGIN
    QByteArray command;
    command.append(char(0x00)); // CLA
    command.append(char(0xA4)); // INS
    command.append(char(0x00)); // P1, select by file identifier
    command.append(char(0x00)); // First or only occurrence
    command.append(char(0x02)); // Lc
    quint16 temp = qToBigEndian<quint16>(fileIdentifier);
    command.append(reinterpret_cast<const char*>(&temp),
                   sizeof(quint16));

    END
    return _sendCommand(command);
}

QNearFieldTarget::RequestId QNearFieldTagType4Symbian::read(quint16 length, quint16 startOffset)
{
    BEGIN
    QByteArray command;
    command.append(char(0x00)); // CLA
    command.append(char(0xB0)); // INS
    quint16 temp = qToBigEndian<quint16>(startOffset);
    command.append(reinterpret_cast<const char*>(&temp),
                   sizeof(quint16)); // P1/P2 offset
    /*temp = qToBigEndian<quint16>(length);
    command.append(reinterpret_cast<const char*>(&temp),
                   sizeof(quint16)); // Le*/
    command.append((quint8)length);

    END
    return _sendCommand(command);
}

QNearFieldTarget::RequestId QNearFieldTagType4Symbian::write(const QByteArray &data, quint16 startOffset)
{
    BEGIN
    QByteArray command;
    command.append(char(0x00)); // CLA
    command.append(char(0xD6)); // INS
    quint16 temp = qToBigEndian<quint16>(startOffset);
    command.append(reinterpret_cast<const char *>(&temp), sizeof(quint16));
    quint16 length = data.count();
    if ((length > 0xFF) || (length < 0x01))
    {
        END
        return QNearFieldTarget::RequestId();
    }
    else
    {
        /*quint16 temp = qToBigEndian<quint16>(length);
        command.append(reinterpret_cast<const char *>(&temp),
                       sizeof(quint16));*/
        command.append((quint8)length);
    }

    command.append(data);
    END
    return _sendCommand(command);
}

QNearFieldTarget::RequestId QNearFieldTagType4Symbian::selectNdefApplication()
{
    BEGIN
    QByteArray command;
    command.append(char(0xD2));
    command.append(char(0x76));
    command.append(char(0x00));
    command.append(char(0x00));
    command.append(char(0x85));
    command.append(char(0x01));
    command.append(char(0x00));
    QNearFieldTarget::RequestId id = select(command);
    END
    return id;
}

QNearFieldTarget::RequestId QNearFieldTagType4Symbian::selectCC()
{
    BEGIN
    END
    return select(0xe103);
}

void QNearFieldTagType4Symbian::handleTagOperationResponse(const RequestId &id, const QByteArray &command, const QByteArray &response, bool emitRequestCompleted)
{
    BEGIN
    Q_UNUSED(command);
    QVariant decodedResponse = decodeResponse(command, response);
    setResponseForRequest(id, decodedResponse, emitRequestCompleted);
    END
}

bool QNearFieldTagType4Symbian::waitForRequestCompleted(const RequestId &id, int msecs)
{
    BEGIN
    END
    return _waitForRequestCompleted(id, msecs);
}
#include "moc_qnearfieldtagtype4_symbian_p.cpp"

QTNFC_END_NAMESPACE

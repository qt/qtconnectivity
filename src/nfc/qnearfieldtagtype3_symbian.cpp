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

#include <nfctag.h>
#include "qnearfieldtagtype3_symbian_p.h"
#include <nfctype3connection.h>
#include <QtEndian>
#include <QVariant>

QTNFC_BEGIN_NAMESPACE

/* For all #if 0 blocks: this is due to symbian RawModeAccess API is not
 * available yet. After the RawModeAccess API is stable, those blocks
 * should be enabled.
 */

#define SYMBIAN_RESP_INCLUDE_LEN

static void OutputByteArray(const QByteArray& data)
{
    for(int i = 0; i < data.count(); ++i)
    {
        LOG("data ["<<i<<"] = "<<((quint16)(data.at(i))));
    }
}

static void OutputCmdMap(const QMap<quint16, QList<quint16> >& data)
{
    QList<quint16> mapKeys = data.keys();
    for(int i = 0; i < mapKeys.count(); ++i)
    {
        LOG("cmd Map key "<<mapKeys.at(i)<<" value "<<data.value(mapKeys.at(i)));
    }
}

static void OutputRspMap(const QMap<quint16, QByteArray>& data)
{
    QList<quint16> mapKeys = data.keys();
    for(int i = 0; i < mapKeys.count(); ++i)
    {
        LOG("rsp Map key "<<mapKeys.at(i));
        OutputByteArray(data.value(mapKeys.at(i)));
    }
}

QNearFieldTagType3Symbian::QNearFieldTagType3Symbian(CNearFieldNdefTarget *tag, QObject *parent)
                                : QNearFieldTagType3(parent), QNearFieldTagImpl(tag)
{
    // It's silly, but easy.
    getIDm();
}

QNearFieldTagType3Symbian::~QNearFieldTagType3Symbian()
{
}

QByteArray QNearFieldTagType3Symbian::uid() const
{
    return _uid();
}

QVariant QNearFieldTagType3Symbian::decodeResponse(const QByteArray & command, const QByteArray &response)
{
    BEGIN
    OutputByteArray(response);
#ifndef SYMBIAN_RESP_INCLUDE_LEN
    QVariant result;
    if (command.count() < 0)
    {
        END;
        return result;
    }

    switch(command.at(0))
    {
        case 0x06:
        {
            // check command
            result.setValue(checkResponse2ServiceBlockList(cmd2ServiceBlockList(command), response));
            break;
        }
        case 0x08:
        {
            result = (response.at(9) == 0);
            break;
        }
        default:
        {
            result = response;
        }

    }
    END
#else
    QVariant result;
    if (command.count() < 0)
    {
        END;
        return result;
    }
    QByteArray newResponse = response.right(response.count() - 1);
    switch(command.at(0))
    {
        case 0x06:
        {
            // check command
            result.setValue(checkResponse2ServiceBlockList(cmd2ServiceBlockList(command), newResponse));
            break;
        }
        case 0x08:
        {
            result = (newResponse.at(9) == 0);
            break;
        }
        default:
        {
            result = newResponse;
        }
    }
    END
#endif
    return result;
}

bool QNearFieldTagType3Symbian::hasNdefMessage()
{
#if 0
    BEGIN
    bool hasNdef = false;
    QList<quint16> blockList;
    // first block
    blockList.append(0);
    // NDEF service
    quint16 serviceCode = 0x0B;

    QMap<quint16, QList<quint16> > serviceBlockList;
    serviceBlockList.insert(serviceCode, blockList);

    QNearFieldTarget::RequestId id = check(serviceBlockList);

    if (_waitForRequestCompletedNoSignal(id))
    {
        QMap<quint16, QByteArray> result = requestResponse(id).value<QMap<quint16, QByteArray> >();
        if (result.contains(serviceCode))
        {
            const QByteArray& lens = result.value(serviceCode);
            if (!lens.isEmpty())
            {
                quint32 len = lens.at(11);
                len<<=8;
                len |= lens.at(12);
                len<<=8;
                len |= lens.at(13);
                hasNdef = (len > 0);
            }
        }
    }
    END
    return hasNdef;
#endif
    return _hasNdefMessage();
}

QNearFieldTarget::RequestId QNearFieldTagType3Symbian::readNdefMessages()
{
    return _ndefMessages();
}

QNearFieldTarget::RequestId QNearFieldTagType3Symbian::writeNdefMessages(const QList<QNdefMessage> &messages)
{
    return _setNdefMessages(messages);
}

QNearFieldTarget::RequestId QNearFieldTagType3Symbian::sendCommand(const QByteArray &command)
{
    return _sendCommand(command);
}

QNearFieldTarget::RequestId QNearFieldTagType3Symbian::sendCommands(const QList<QByteArray> &commands)
{
    return _sendCommands(commands);
}

#if 0
quint16 QNearFieldTagType3Symbian::systemCode()
{
    return 0;
}

QList<quint16> QNearFieldTagType3Symbian::services()
{
    return QList<quint16>();
}

int QNearFieldTagType3Symbian::serviceMemorySize(quint16 serviceCode)
{
    Q_UNUSED(serviceCode);
    return 0;
}

QNearFieldTarget::RequestId QNearFieldTagType3Symbian::serviceData(quint16 serviceCode)
{
    Q_UNUSED(serviceCode);
    return QNearFieldTarget::RequestId();
}

QNearFieldTarget::RequestId QNearFieldTagType3Symbian::writeServiceData(quint16 serviceCode, const QByteArray &data)
{
    Q_UNUSED(serviceCode);
    Q_UNUSED(data);

    return RequestId();
}
#endif

QNearFieldTarget::RequestId QNearFieldTagType3Symbian::check(const QMap<quint16, QList<quint16> > &serviceBlockList)
{
    BEGIN
    quint8 numberOfBlocks;
    QByteArray command;
    command.append(0x06); // command code
    command.append(serviceBlockList2CmdParam(serviceBlockList, numberOfBlocks, true));
    if (command.count() > 1)
    {
        END
        return (_sendCommand(command));
    }
    else
    {
        END
        return QNearFieldTarget::RequestId();
    }
}

QNearFieldTarget::RequestId QNearFieldTagType3Symbian::update(const QMap<quint16, QList<quint16> > &serviceBlockList, const QByteArray &data)
{
    BEGIN
    quint8 numberOfBlocks;
    QByteArray command;
    command.append(0x08); // command code
    command.append(serviceBlockList2CmdParam(serviceBlockList, numberOfBlocks, false));
    if (command.count() > 1)
    {
        command.append(data);
        END
        return (_sendCommand(command));
    }
    else
    {
        END
        return QNearFieldTarget::RequestId();
    }
}

const QByteArray& QNearFieldTagType3Symbian::getIDm()
{
    BEGIN
    if (mIDm.isEmpty())
    {
        // this is the first time to get IDm
        CNearFieldTag * tag = mTag->CastToTag();

        if (tag)
        {
            CNfcType3Connection * connection = static_cast<CNfcType3Connection *>(tag->TagConnection());
            TBuf8<8> IDm;
            TInt error = connection->GetIDm(IDm);
            if (KErrNone == error)
            {
                mIDm = QNFCNdefUtility::TDesC2QByteArray(IDm);
            }
        }
    }
    OutputByteArray(mIDm);
    END
    return mIDm;
}

QByteArray QNearFieldTagType3Symbian::serviceBlockList2CmdParam(const QMap<quint16, QList<quint16> > &serviceBlockList, quint8& numberOfBlocks, bool isCheckCommand)
{
    BEGIN
    OutputCmdMap(serviceBlockList);
    QByteArray command;
    command.append(getIDm());
    numberOfBlocks = 0;

    if (command.isEmpty())
    {
        // can't get IDm
        END
        return command;
    }

    quint8 numberOfServices = serviceBlockList.keys().count();

    if ((numberOfServices > 16) || (numberOfServices < 1))
    {
        // out of range of services number
        END
        return QByteArray();
    }
    else
    {
        command.append(numberOfServices);
    }

    quint8 serviceCodeListOrder = 0;
    QByteArray serviceCodeList;
    QByteArray blockList;
    foreach(const quint16 serviceCode, serviceBlockList.keys())
    {
        serviceCodeList.append(reinterpret_cast<const char *>(&serviceCode), sizeof(quint16));

        numberOfBlocks += serviceBlockList.value(serviceCode).count();
        LOG("numberOfBlocks "<<numberOfBlocks)
        if ( (isCheckCommand && (numberOfBlocks > 12)) ||
             (!isCheckCommand && (numberOfBlocks > 8)) )
        {
            // out of range of block number
            LOG("out of range of block number");
            END
            return QByteArray();
        }

        foreach(const quint16 blockNumber, serviceBlockList.value(serviceCode))
        {
            if (blockNumber > 255)
            {
                // 3 bytes format
                blockList.append(0x00 | (serviceCodeListOrder & 0x0F));
                quint16 blkNum = blockNumber;
                blkNum = qToLittleEndian(blkNum);
                blockList.append(reinterpret_cast<const char *>(&blkNum), sizeof(quint16));
            }
            else // 2 bytes format
            {
                blockList.append(0x80 | (serviceCodeListOrder & 0x0F));
                quint8 blkNum = blockNumber;
                blockList.append(blkNum);
            }
        }
    }

    if (numberOfBlocks < 1)
    {
        // out of range of block number
        LOG("out of range of block number, number of blocks < 1");
        END
        return QByteArray();
    }

    command.append(serviceCodeList);
    command.append(numberOfBlocks);
    command.append(blockList);
    OutputByteArray(command);
    END
    return command;
}

QMap<quint16, QList<quint16> > QNearFieldTagType3Symbian::cmd2ServiceBlockList(const QByteArray& cmd)
{
    BEGIN
    QMap<quint16, QList<quint16> > result;
    // skip command code and IDm
    QByteArray data = cmd.right(cmd.count() - 9);

    quint8 numberOfServices = data.at(0);

    QByteArray serviceCodeList = data.mid(1, 2 * numberOfServices);

    quint8 numberOfBlocks = data.at(2 * numberOfServices + 1);

    QByteArray blockList = data.right(data.count() - 1 - 2*numberOfServices - 1);

    QList<quint16> svcList;
    for (int i = 0; i < numberOfServices; ++i)
    {
        unsigned char bytes[2];
        bytes[0] = serviceCodeList.at(i*2);
        bytes[1] = serviceCodeList.at(i*2+1);
        quint16 serviceCode = qFromLittleEndian<quint16>(bytes);
        svcList.append(serviceCode);
    }

    for (int j = 0, k = 0; j < numberOfBlocks; ++j, ++k)
    {
        quint8 byte0 = blockList.at(k);
        quint8 serviceCodeListOrder = byte0 & 0x0F;
        quint16 blockNumber = 0;

        if (byte0 & 0x80)
        {
            // two bytes format
            blockNumber = blockList.at(++k);
        }
        else
        {
            // three bytes format
            unsigned char bytes[2];
            bytes[0] = blockList.at(++k);
            bytes[1] = blockList.at(++k);
            blockNumber = qFromLittleEndian<quint16>(bytes);
        }

        if (result.contains(svcList.at(serviceCodeListOrder)))
        {
            result[svcList.at(serviceCodeListOrder)].append(blockNumber);
        }
        else
        {
            QList<quint16> blocks;
            blocks.append(blockNumber);
            result.insert(svcList.at(serviceCodeListOrder), blocks);
        }
    }

    OutputCmdMap(result);

    END
    return result;
}

QMap<quint16, QByteArray> QNearFieldTagType3Symbian::checkResponse2ServiceBlockList(const QMap<quint16, QList<quint16> > &serviceBlockList, const QByteArray& response)
{
    BEGIN
    OutputCmdMap(serviceBlockList);
    QMap<quint16, QByteArray> result;
    // at least, the response should contain resp code + IDM + status flags
    if (response.count() < 11)
    {
        END
        return result;
    }

    if ((response.at(0) != 0x07) || (response.mid(1,8) != getIDm()) || (response.at(10) != 0))
    {
        END
        return result;
    }

    quint32 index = 12;
    foreach(const quint16 serviceCode, serviceBlockList.keys())
    {
        quint8 blockCount = serviceBlockList.value(serviceCode).count();
        result.insert(serviceCode, response.mid(index, 16*blockCount));
        index+=16*blockCount;
    }
    OutputRspMap(result);
    END
    return result;
}


void QNearFieldTagType3Symbian::handleTagOperationResponse(const RequestId &id, const QByteArray &command, const QByteArray &response, bool emitRequestCompleted)
{
    Q_UNUSED(command);
    QVariant decodedResponse = decodeResponse(command, response);
    setResponseForRequest(id, decodedResponse, emitRequestCompleted);
}

bool QNearFieldTagType3Symbian::waitForRequestCompleted(const RequestId &id, int msecs)
{
    BEGIN
    END
    return _waitForRequestCompleted(id, msecs);
}

#include "moc_qnearfieldtagtype3_symbian_p.cpp"

QTNFC_END_NAMESPACE

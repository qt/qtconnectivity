/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qlowenergycontrollernew_p.h"
#include "qbluetoothsocket_p.h"

#include <QtCore/QLoggingCategory>
#include <QtBluetooth/QBluetoothSocket>

#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>

#define ATTRIBUTE_CHANNEL_ID 4

#define GATT_PRIMARY_SERVICE    0x2800

#define ATT_OP_READ_BY_GROUP_REQUEST    0x10
#define ATT_OP_READ_BY_GROUP_RESPONSE   0x11

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

static inline QBluetoothUuid convert_uuid128(const uint128_t *p)
{
    uint128_t dst_hostOrder, dst_bigEndian;

    // Bluetooth LE data comes as little endian
    // uuids are constructed using high endian
    btoh128(p, &dst_hostOrder);
    hton128(&dst_hostOrder, &dst_bigEndian);

    // convert to Qt's own data type
    quint128 qtdst;
    memcpy(&qtdst, &dst_bigEndian, sizeof(uint128_t));

    return QBluetoothUuid(qtdst);
}

void QLowEnergyControllerNewPrivate::connectToDevice()
{
    setState(QLowEnergyControllerNew::ConnectingState);
    if (l2cpSocket)
        delete l2cpSocket;

    l2cpSocket = new QBluetoothSocket(QBluetoothServiceInfo::L2capProtocol, this);
    connect(l2cpSocket, SIGNAL(connected()), this, SLOT(l2cpConnected()));
    connect(l2cpSocket, SIGNAL(disconnected()), this, SLOT(l2cpDisconnected()));
    connect(l2cpSocket, SIGNAL(error(QBluetoothSocket::SocketError)),
            this, SLOT(l2cpErrorChanged(QBluetoothSocket::SocketError)));
    connect(l2cpSocket, SIGNAL(readyRead()), this, SLOT(l2cpReadyRead()));

    l2cpSocket->d_ptr->isLowEnergySocket = true;
    l2cpSocket->connectToService(remoteDevice, ATTRIBUTE_CHANNEL_ID);
}

void QLowEnergyControllerNewPrivate::l2cpConnected()
{
    Q_Q(QLowEnergyControllerNew);

    setState(QLowEnergyControllerNew::ConnectedState);
    emit q->connected();
}

void QLowEnergyControllerNewPrivate::disconnectFromDevice()
{
    setState(QLowEnergyControllerNew::ClosingState);
    l2cpSocket->close();
}

void QLowEnergyControllerNewPrivate::l2cpDisconnected()
{
    Q_Q(QLowEnergyControllerNew);

    setState(QLowEnergyControllerNew::UnconnectedState);
    emit q->disconnected();
}

void QLowEnergyControllerNewPrivate::l2cpErrorChanged(QBluetoothSocket::SocketError e)
{
    switch (e) {
    case QBluetoothSocket::HostNotFoundError:
        setError(QLowEnergyControllerNew::UnknownRemoteDeviceError);
        qCDebug(QT_BT_BLUEZ) << "The passed remote device address cannot be found";
        break;
    case QBluetoothSocket::NetworkError:
        setError(QLowEnergyControllerNew::NetworkError);
        qCDebug(QT_BT_BLUEZ) << "Network IO error while talking to LE device";
        break;
    case QBluetoothSocket::UnknownSocketError:
    case QBluetoothSocket::UnsupportedProtocolError:
    case QBluetoothSocket::OperationError:
    case QBluetoothSocket::ServiceNotFoundError:
    default:
        // these errors shouldn't happen -> as it means
        // the code in this file has bugs
        qCDebug(QT_BT_BLUEZ) << "Unknown l2cp socket error: " << e << l2cpSocket->errorString();
        setError(QLowEnergyControllerNew::UnknownError);
        break;
    }

    setState(QLowEnergyControllerNew::UnconnectedState);
}

void QLowEnergyControllerNewPrivate::l2cpReadyRead()
{
    Q_Q(QLowEnergyControllerNew);

    const QByteArray response = l2cpSocket->readAll();
    //qDebug() << response.size() << "data:" << response.toHex();

    if (response.isEmpty())
        return;

    const quint8 command = response.constData()[0];
    QLowEnergyHandle start = 0, end = 0;

    switch (command) {
    case ATT_OP_READ_BY_GROUP_RESPONSE:
    {
        const quint16 elementLength = response.constData()[1];
        const quint16 numElements = (response.size() - 2) / elementLength;
        //qDebug() << numElements << elementLength << response.size() - 2;
        quint16 offset = 2;
        const char *data = response.constData();
        for (int i = 0; i < numElements; i++) {
            start = bt_get_le16(&data[offset]);
            end = bt_get_le16(&data[offset+2]);

            QBluetoothUuid uuid;
            if (elementLength == 6) //16 bit uuid
                uuid = QBluetoothUuid(bt_get_le16(&data[offset+4]));
            else if (elementLength == 20) //128 bit uuid
                uuid = convert_uuid128((uint128_t *)&data[offset+4]);
            //else -> do nothing

            offset += elementLength;


            //qDebug() << "Found uuid:" << uuid << "start handle:" << hex
            //         << start << "end handle:" << end;
            HandlePair pair(start, end);
            serviceList.insert(uuid, pair);
            emit q->serviceDiscovered(uuid);
        }

        break;
    }
    default:
        qCDebug(QT_BT_BLUEZ) << "Unknown packet: " << response.toHex();
    }

    if (end != 0xFFFF)
        sendReadByGroupRequest(end+1, 0xFFFF);
    else
        emit q->discoveryFinished();
}

void QLowEnergyControllerNewPrivate::discoverServices()
{
    sendReadByGroupRequest(0x0001, 0xFFFF);
}

void QLowEnergyControllerNewPrivate::sendReadByGroupRequest(
                    QLowEnergyHandle start, QLowEnergyHandle end)
{
    //call for primary services
    bt_uuid_t primary;
    bt_uuid16_create(&primary, GATT_PRIMARY_SERVICE);

#define GRP_TYPE_REQ_SIZE 7
    quint8 packet[GRP_TYPE_REQ_SIZE];

    packet[0] = ATT_OP_READ_BY_GROUP_REQUEST;
    bt_put_unaligned(htobs(start), (quint16 *) &packet[1]);
    bt_put_unaligned(htobs(end), (quint16 *) &packet[3]);
    bt_put_unaligned(htobs(primary.value.u16), (quint16 *) &packet[5]);

    QByteArray data(GRP_TYPE_REQ_SIZE, Qt::Uninitialized);
    memcpy(data.data(), packet,  GRP_TYPE_REQ_SIZE);

    //qDebug() << "Sending grp request" << hex << start << end;
    qint64 result = l2cpSocket->write(data.constData(), GRP_TYPE_REQ_SIZE);
    if (result == -1) {
        qCDebug(QT_BT_BLUEZ) << "Cannot write ReadByGroupRequest";
        setError(QLowEnergyControllerNew::NetworkError);
    }
}

QT_END_NAMESPACE

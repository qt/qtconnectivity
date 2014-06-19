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
#include <QtBluetooth/QLowEnergyService>

#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>

#define ATTRIBUTE_CHANNEL_ID 4

#define GATT_PRIMARY_SERVICE    0x2800
#define GATT_CHARACTERISTIC     0x2803

#define ATT_OP_READ_BY_TYPE_REQUEST     0x8
#define ATT_OP_READ_BY_TYPE_RESPONSE    0x9
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
    requestPending = false;
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

    invalidateServices();
    setState(QLowEnergyControllerNew::UnconnectedState);
}

void QLowEnergyControllerNewPrivate::l2cpReadyRead()
{
    requestPending = false;
    const QByteArray reply = l2cpSocket->readAll();
    //qDebug() << response.size() << "data:" << response.toHex();
    if (reply.isEmpty())
        return;

    Q_ASSERT(!openRequests.isEmpty());
    const Request request = openRequests.dequeue();
    processReply(request, reply);

    sendNextPendingRequest();
}

void QLowEnergyControllerNewPrivate::sendNextPendingRequest()
{
    if (openRequests.isEmpty() || requestPending)
        return;

    const Request &request = openRequests.head();
//    qDebug() << "Sending request, type:" << hex << request.command
//             << request.payload.toHex();

    requestPending = true;
    qint64 result = l2cpSocket->write(request.payload.constData(),
                                      request.payload.size());
    if (result == -1) {
        qCDebug(QT_BT_BLUEZ) << "Cannot write L2CP command:" << hex
                             << request.payload.toHex()
                             << l2cpSocket->errorString();
        setError(QLowEnergyControllerNew::NetworkError);
    }
}

void QLowEnergyControllerNewPrivate::processReply(
        const Request &request, const QByteArray &response)
{
    Q_Q(QLowEnergyControllerNew);

    const quint8 command = response.constData()[0];

    switch (command) {
    case ATT_OP_READ_BY_GROUP_RESPONSE:
    {
        Q_ASSERT(request.command == ATT_OP_READ_BY_GROUP_REQUEST);
        QLowEnergyHandle start = 0, end = 0;
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

            QLowEnergyServicePrivate *priv = new QLowEnergyServicePrivate();
            priv->uuid = uuid;
            priv->startHandle = start;
            priv->endHandle = end;
            priv->setController(this);

            QSharedPointer<QLowEnergyServicePrivate> pointer(priv);

            serviceList.insert(uuid, pointer);
            emit q->serviceDiscovered(uuid);
        }

        if (end != 0xFFFF)
            sendReadByGroupRequest(end+1, 0xFFFF);
        else
            emit q->discoveryFinished();

    }
        break;
    case ATT_OP_READ_BY_TYPE_RESPONSE:
    {
        Q_ASSERT(request.command == ATT_OP_READ_BY_TYPE_REQUEST);

        /* packet format:
         *  <opcode><elementLength>[<handle><property><charHandle><uuid>]+
         *
         *  The uuid can be 16 or 128 bit.
         */
        QLowEnergyHandle startHandle, valueHandle;
        //qDebug() << "readByType response received:" << response.toHex();
        const quint16 elementLength = response.constData()[1];
        const quint16 numElements = (response.size() - 2) / elementLength;
        quint16 offset = 2;
        const char *data = response.constData();
        for (int i = 0; i < numElements; i++) {
            startHandle = bt_get_le16(&data[offset]);
            QLowEnergyCharacteristicInfo::PropertyTypes flags =
                    (QLowEnergyCharacteristicInfo::PropertyTypes)data[offset+2];
            valueHandle = bt_get_le16(&data[offset+3]);
            QBluetoothUuid uuid;

            if (elementLength == 7) // 16 bit uuid
                uuid = QBluetoothUuid(bt_get_le16(&data[offset+5]));
            else
                uuid = convert_uuid128((uint128_t *)&data[offset+5]);

            offset += elementLength;

            qDebug() << "Found handle:" << hex << startHandle
                     << "properties:" << flags
                     << "value handle:" << valueHandle
                     << "uuid:" << uuid.toString();
        }

        QSharedPointer<QLowEnergyServicePrivate> p =
                request.reference.value<QSharedPointer<QLowEnergyServicePrivate> >();
        if (startHandle + 1 < p->endHandle) // more chars to discover
            sendReadByTypeRequest(p, startHandle + 1);
        else
            p->setState(QLowEnergyService::ServiceDiscovered);
    }
        break;
    default:
        qCDebug(QT_BT_BLUEZ) << "Unknown packet: " << response.toHex();
    }


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

    Request request;
    request.payload = data;
    request.command = ATT_OP_READ_BY_GROUP_REQUEST;
    openRequests.enqueue(request);

    sendNextPendingRequest();
}

void QLowEnergyControllerNewPrivate::discoverServiceDetails(const QBluetoothUuid &service)
{
    if (!serviceList.contains(service)) {
        qCWarning(QT_BT_BLUEZ) << "Discovery of unknown service" << service.toString()
                               << "not possible";
        return;
    }

    QSharedPointer<QLowEnergyServicePrivate> serviceData = serviceList.value(service);
    sendReadByTypeRequest(serviceData, serviceData->startHandle);
}

void QLowEnergyControllerNewPrivate::sendReadByTypeRequest(
        QSharedPointer<QLowEnergyServicePrivate> serviceData,
        QLowEnergyHandle nextHandle)
{
    bt_uuid_t uuid;
    bt_uuid16_create(&uuid, GATT_CHARACTERISTIC);

#define READ_BY_TYPE_REQ_SIZE 7
    quint8 packet[READ_BY_TYPE_REQ_SIZE];

    packet[0] = ATT_OP_READ_BY_TYPE_REQUEST;
    bt_put_unaligned(htobs(nextHandle), (quint16 *) &packet[1]);
    bt_put_unaligned(htobs(serviceData->endHandle), (quint16 *) &packet[3]);
    bt_put_unaligned(htobs(uuid.value.u16), (quint16 *) &packet[5]);

    QByteArray data(READ_BY_TYPE_REQ_SIZE, Qt::Uninitialized);
    memcpy(data.data(), packet,  READ_BY_TYPE_REQ_SIZE);
    qDebug() << "Sending read_by_type request, startHandle:" << hex
             << nextHandle << "endHandle:" << serviceData->endHandle
             << "packet:" << data.toHex();

    Request request;
    request.payload = data;
    request.command = ATT_OP_READ_BY_TYPE_REQUEST;
    request.reference = QVariant::fromValue(serviceData);
    openRequests.enqueue(request);

    sendNextPendingRequest();
}


QT_END_NAMESPACE

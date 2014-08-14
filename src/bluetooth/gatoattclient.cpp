/****************************************************************************
**
** Copyright (C) 2013 Javier de San Pedro <dev.git@javispedro.com>
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

#include <QtCore/QDataStream>
#include <QtCore/QDebug>

#include "gatoattclient.h"
#include "helpers.h"

#define PROTOCOL_DEBUG 0

#define ATT_CID 4
#define ATT_PSM 31

#define ATT_DEFAULT_LE_MTU 23
#define ATT_MAX_LE_MTU 0x200

QT_BEGIN_NAMESPACE

enum AttOpcode {
    AttOpNone = 0,
    AttOpErrorResponse = 0x1,
    AttOpExchangeMTURequest = 0x2,
    AttOpExchangeMTUResponse = 0x3,
    AttOpFindInformationRequest = 0x4,
    AttOpFindInformationResponse = 0x5,
    AttOpFindByTypeValueRequest = 0x6,
    AttOpFindByTypeValueResponse = 0x7,
    AttOpReadByTypeRequest = 0x8,
    AttOpReadByTypeResponse = 0x9,
    AttOpReadRequest = 0xA,
    AttOpReadResponse = 0xB,
    AttOpReadBlobRequest = 0xC,
    AttOpReadBlobResponse = 0xD,
    AttOpReadMultipleRequest = 0xE,
    AttOpReadMultipleResponse = 0xF,
    AttOpReadByGroupTypeRequest = 0x10,
    AttOpReadByGroupTypeResponse = 0x11,
    AttOpWriteRequest = 0x12,
    AttOpWriteResponse = 0x13,
    AttOpWriteCommand = 0x52,
    AttOpPrepareWriteRequest = 0x16,
    AttOpPrepareWriteResponse = 0x17,
    AttOpExecuteWriteRequest = 0x18,
    AttOpExecuteWriteResponse = 0x19,
    AttOpHandleValueNotification = 0x1B,
    AttOpHandleValueIndication = 0x1D,
    AttOpHandleValueConfirmation = 0x1E,
    AttOpSignedWriteCommand = 0xD2
};

static QByteArray remove_method_signature(const char *sig)
{
    const char* bracketPosition = strchr(sig, '(');
    if (!bracketPosition || !(sig[0] >= '0' && sig[0] <= '3')) {
        qWarning("Invalid slot specification");
        return QByteArray();
    }
    return QByteArray(sig + 1, bracketPosition - 1 - sig);
}

GatoAttClient::GatoAttClient(QObject *parent) :
    QObject(parent), socket(new GatoSocket(this)), cur_mtu(ATT_DEFAULT_LE_MTU), next_id(1)
{
    connect(socket, SIGNAL(connected()), SLOT(handleSocketConnected()));
    connect(socket, SIGNAL(disconnected()), SLOT(handleSocketDisconnected()));
    connect(socket, SIGNAL(readyRead()), SLOT(handleSocketReadyRead()));
}

GatoAttClient::~GatoAttClient()
{
}

GatoSocket::State GatoAttClient::state() const
{
    return socket->state();
}

bool GatoAttClient::connectTo(const GatoAddress &addr)
{
    return socket->connectTo(addr, ATT_CID);
}

void GatoAttClient::close()
{
    socket->close();
}

int GatoAttClient::mtu() const
{
    return cur_mtu;
}

uint GatoAttClient::request(int opcode, const QByteArray &data, QObject *receiver, const char *member)
{
    Request req;
    req.id = next_id++;
    req.opcode = opcode;
    req.pkt = data;
    req.pkt.prepend(static_cast<char>(opcode));
    req.receiver = receiver;
    req.member = remove_method_signature(member);

    pending_requests.enqueue(req);

    if (pending_requests.size() == 1) {
        // So we can just send this request instead of waiting for others to complete
        sendARequest();
    }

    return req.id;
}

void GatoAttClient::cancelRequest(uint id)
{
    QQueue<Request>::iterator it = pending_requests.begin();
    while (it != pending_requests.end()) {
        if (it->id == id) {
            it = pending_requests.erase(it);
        } else {
            ++it;
        }
    }
}

uint GatoAttClient::requestExchangeMTU(quint16 client_mtu, QObject *receiver, const char *member)
{
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::LittleEndian);
    s << client_mtu;

    return request(AttOpExchangeMTURequest, data, receiver, member);
}

uint GatoAttClient::requestFindInformation(GatoHandle start, GatoHandle end, QObject *receiver, const char *member)
{
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::LittleEndian);
    s << start << end;

    return request(AttOpFindInformationRequest, data, receiver, member);
}

uint GatoAttClient::requestFindByTypeValue(GatoHandle start, GatoHandle end, const GatoUUID &uuid, const QByteArray &value, QObject *receiver, const char *member)
{
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::LittleEndian);
    s << start << end;

    bool uuid16_ok;
    quint16 uuid16 = uuid.toUInt16(&uuid16_ok);
    if (uuid16_ok) {
        s << uuid16;
    } else {
        qWarning() << "FindByTypeValue does not support UUIDs other than UUID16";
        return -1;
    }

    s << value;

    return request(AttOpFindByTypeValueRequest, data, receiver, member);
}

uint GatoAttClient::requestReadByType(GatoHandle start, GatoHandle end, const GatoUUID &uuid, QObject *receiver, const char *member)
{
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::LittleEndian);
    s << start << end;
    write_gatouuid(s, uuid, true, false);

    return request(AttOpReadByTypeRequest, data, receiver, member);
}

uint GatoAttClient::requestRead(GatoHandle handle, QObject *receiver, const char *member)
{
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::LittleEndian);
    s << handle;

    return request(AttOpReadRequest, data, receiver, member);
}

uint GatoAttClient::requestReadByGroupType(GatoHandle start, GatoHandle end, const GatoUUID &uuid, QObject *receiver, const char *member)
{
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::LittleEndian);
    s << start << end;
    write_gatouuid(s, uuid, true, false);

    return request(AttOpReadByGroupTypeRequest, data, receiver, member);
}

uint GatoAttClient::requestWrite(GatoHandle handle, const QByteArray &value, QObject *receiver, const char *member)
{
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::LittleEndian);
    s << handle;
    s.writeRawData(value.constData(), value.length());

    return request(AttOpWriteRequest, data, receiver, member);
}

void GatoAttClient::command(int opcode, const QByteArray &data)
{
    QByteArray packet = data;
    packet.prepend(static_cast<char>(opcode));

    socket->send(packet);

#if PROTOCOL_DEBUG
    qDebug() << "Wrote" << packet.size() << "bytes (command)" << packet.toHex();
#endif
}

void GatoAttClient::commandWrite(GatoHandle handle, const QByteArray &value)
{
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::LittleEndian);
    s << handle;
    s.writeRawData(value.constData(), value.length());

    command(AttOpWriteCommand, data);
}

void GatoAttClient::sendARequest()
{
    if (pending_requests.isEmpty()) {
        return;
    }

    Request &req = pending_requests.head();
    socket->send(req.pkt);

#if PROTOCOL_DEBUG
    qDebug() << "Wrote" << req.pkt.size() << "bytes (request)" << req.pkt.toHex();
#endif
}

bool GatoAttClient::handleEvent(const QByteArray &event)
{
    const char *data = event.constData();
    quint8 opcode = event[0];
    GatoHandle handle;

    switch (opcode) {
    case AttOpHandleValueNotification:
        handle = read_le<GatoHandle>(&data[1]);
        emit attributeUpdated(handle, event.mid(3), false);
        return true;
    case AttOpHandleValueIndication:
        handle = read_le<GatoHandle>(&data[1]);

        // Send the confirmation back
        command(AttOpHandleValueConfirmation, QByteArray());

        emit attributeUpdated(handle, event.mid(3), true);
        return true;
    default:
        return false;
    }
}

bool GatoAttClient::handleResponse(const Request &req, const QByteArray &response)
{
    // If we know the request, we can provide a decoded answer
    switch (req.opcode) {
    case AttOpExchangeMTURequest:
        if (response[0] == AttOpExchangeMTUResponse) {
            if (req.receiver) {
                QMetaObject::invokeMethod(req.receiver, req.member.constData(),
                                          Q_ARG(uint, req.id),
                                          Q_ARG(quint16, read_le<quint16>(response.constData() + 1)));
            }
            return true;
        } else if (response[0] == AttOpErrorResponse && response[1] == AttOpExchangeMTURequest) {
            if (req.receiver) {
                QMetaObject::invokeMethod(req.receiver, req.member.constData(),
                                          Q_ARG(uint, req.id),
                                          Q_ARG(quint16, 0));
            }
            return true;
        } else {
            return false;
        }
        break;
    case AttOpFindInformationRequest:
        if (response[0] == AttOpFindInformationResponse) {
            if (req.receiver) {
                QMetaObject::invokeMethod(req.receiver, req.member.constData(),
                                          Q_ARG(uint, req.id),
                                          Q_ARG(QList<GatoAttClient::InformationData>, parseInformationData(response.mid(1))));
            }
            return true;
        } else if (response[0] == AttOpErrorResponse && response[1] == AttOpFindInformationRequest) {
            if (req.receiver) {
                QMetaObject::invokeMethod(req.receiver, req.member.constData(),
                                          Q_ARG(uint, req.id),
                                          Q_ARG(QList<GatoAttClient::InformationData>, QList<InformationData>()));
            }
            return true;
        } else {
            return false;
        }
        break;
    case AttOpFindByTypeValueRequest:
        if (response[0] == AttOpFindByTypeValueResponse) {
            if (req.receiver) {
                QMetaObject::invokeMethod(req.receiver, req.member.constData(),
                                          Q_ARG(uint, req.id),
                                          Q_ARG(QList<GatoAttClient::HandleInformation>, parseHandleInformation(response.mid(1))));
            }
            return true;
        } else if (response[0] == AttOpErrorResponse && response[1] == AttOpFindByTypeValueRequest) {
            if (req.receiver) {
                QMetaObject::invokeMethod(req.receiver, req.member.constData(),
                                          Q_ARG(uint, req.id),
                                          Q_ARG(QList<GatoAttClient::HandleInformation>, QList<HandleInformation>()));
            }
            return true;
        } else {
            return false;
        }
        break;
    case AttOpReadByTypeRequest:
        if (response[0] == AttOpReadByTypeResponse) {
            if (req.receiver) {
                QMetaObject::invokeMethod(req.receiver, req.member.constData(),
                                          Q_ARG(uint, req.id),
                                          Q_ARG(QList<GatoAttClient::AttributeData>, parseAttributeData(response.mid(1))));
            }
            return true;
        } else if (response[0] == AttOpErrorResponse && response[1] == AttOpReadByTypeRequest) {
            if (req.receiver) {
                QMetaObject::invokeMethod(req.receiver, req.member.constData(),
                                          Q_ARG(uint, req.id),
                                          Q_ARG(QList<GatoAttClient::AttributeData>, QList<AttributeData>()));
            }
            return true;
        } else {
            return false;
        }
        break;
    case AttOpReadRequest:
        if (response[0] == AttOpReadResponse) {
            if (req.receiver) {
                QMetaObject::invokeMethod(req.receiver, req.member.constData(),
                                          Q_ARG(uint, req.id),
                                          Q_ARG(QByteArray, response.mid(1)));
            }
            return true;
        } else if (response[0] == AttOpErrorResponse && response[1] == AttOpReadRequest) {
            if (req.receiver) {
                QMetaObject::invokeMethod(req.receiver, req.member.constData(),
                                          Q_ARG(uint, req.id),
                                          Q_ARG(QByteArray, QByteArray()));
            }
            return true;
        } else {
            return false;
        }
        break;
    case AttOpReadByGroupTypeRequest:
        if (response[0] == AttOpReadByGroupTypeResponse) {
            if (req.receiver) {
                QMetaObject::invokeMethod(req.receiver, req.member.constData(),
                                          Q_ARG(uint, req.id),
                                          Q_ARG(QList<GatoAttClient::AttributeGroupData>, parseAttributeGroupData(response.mid(1))));
            }
            return true;
        } else if (response[0] == AttOpErrorResponse && response[1] == AttOpReadByGroupTypeRequest) {
            if (req.receiver) {
                QMetaObject::invokeMethod(req.receiver, req.member.constData(),
                                          Q_ARG(uint, req.id),
                                          Q_ARG(QList<GatoAttClient::AttributeGroupData>, QList<AttributeGroupData>()));
            }
            return true;
        } else {
            return false;
        }
        break;
    case AttOpWriteRequest:
        if (response[0] == AttOpWriteResponse) {
            if (req.receiver) {
                QMetaObject::invokeMethod(req.receiver, req.member.constData(),
                                          Q_ARG(uint, req.id),
                                          Q_ARG(bool, true));
            }
            return true;
        } else if (response[0] == AttOpErrorResponse && response[1] == AttOpWriteRequest) {
            if (req.receiver) {
                QMetaObject::invokeMethod(req.receiver, req.member.constData(),
                                          Q_ARG(uint, req.id),
                                          Q_ARG(bool, false));
            }
            return true;
        } else {
            return false;
        }
        break;
    default: // Otherwise just send a QByteArray.
        if (req.receiver) {
            QMetaObject::invokeMethod(req.receiver, req.member.constData(),
                                      Q_ARG(const QByteArray&, response));
        }
        return true;
    }
}

QList<GatoAttClient::InformationData> GatoAttClient::parseInformationData(const QByteArray &data)
{
    const int format = data[0];
    QList<InformationData> list;
    int item_len;

    switch (format) {
    case 1:
        item_len = 2 + 2;
        break;
    case 2:
        item_len = 2 + 16;
        break;
    default:
        qWarning() << "Unknown InformationData format!";
        return list;
    }

    int items = (data.size() - 1) / item_len;
    list.reserve(items);

    int pos = 1;
    const char *s = data.constData();
    for (int i = 0; i < items; i++) {
        InformationData d;
        QByteArray uuid;
        d.handle = read_le<GatoHandle>(&s[pos]);
        switch (format) {
        case 1:
            uuid = data.mid(pos + 2, 2);
            break;
        case 2:
            uuid = data.mid(pos + 2, 16);
            break;
        }
        d.uuid = bytearray_to_gatouuid(uuid);

        list.append(d);

        pos += item_len;
    }

    return list;
}

QList<GatoAttClient::HandleInformation> GatoAttClient::parseHandleInformation(const QByteArray &data)
{
    const int item_len = 2;
    const int items = data.size() / item_len;
    QList<HandleInformation> list;
    list.reserve(items);

    int pos = 0;
    const char *s = data.constData();
    for (int i = 0; i < items; i++) {
        HandleInformation d;
        d.start = read_le<GatoHandle>(&s[pos]);
        d.end = read_le<GatoHandle>(&s[pos + 2]);
        list.append(d);

        pos += item_len;
    }

    return list;
}

QList<GatoAttClient::AttributeData> GatoAttClient::parseAttributeData(const QByteArray &data)
{
    const int item_len = data[0];
    const int items = (data.size() - 1) / item_len;
    QList<AttributeData> list;
    list.reserve(items);

    int pos = 1;
    const char *s = data.constData();
    for (int i = 0; i < items; i++) {
        AttributeData d;
        d.handle = read_le<GatoHandle>(&s[pos]);
        d.value = data.mid(pos + 2, item_len - 2);
        list.append(d);

        pos += item_len;
    }

    return list;
}

QList<GatoAttClient::AttributeGroupData> GatoAttClient::parseAttributeGroupData(const QByteArray &data)
{
    const int item_len = data[0];
    const int items = (data.size() - 1) / item_len;
    QList<AttributeGroupData> list;
    list.reserve(items);

    int pos = 1;
    const char *s = data.constData();
    for (int i = 0; i < items; i++) {
        AttributeGroupData d;
        d.start = read_le<GatoHandle>(&s[pos]);
        d.end = read_le<GatoHandle>(&s[pos + 2]);
        d.value = data.mid(pos + 4, item_len - 4);
        list.append(d);

        pos += item_len;
    }

    return list;
}

void GatoAttClient::handleSocketConnected()
{
    requestExchangeMTU(ATT_MAX_LE_MTU, this, SLOT(handleServerMTU(quint16)));
    emit connected();
}

void GatoAttClient::handleSocketDisconnected()
{
    emit disconnected();
}

void GatoAttClient::handleSocketReadyRead()
{
    QByteArray pkt = socket->receive();
    if (!pkt.isEmpty()) {
#if PROTOCOL_DEBUG
        qDebug() << "Received" << pkt.size() << "bytes" << pkt.toHex();
#endif

        // Check if it is an event
        if (handleEvent(pkt)) {
            return;
        }

        // Otherwise, if we have a request waiting, check if this answers it
        if (!pending_requests.isEmpty()) {
            if (handleResponse(pending_requests.head(), pkt)) {
                pending_requests.dequeue();
                // Proceed to next request
                if (!pending_requests.isEmpty()) {
                    sendARequest();
                }
                return;
            }
        }

        qDebug() << "No idea what this packet ("
                 << QString("0x%1").arg(uint(pkt.at(0)), 2, 16, QLatin1Char('0'))
                 << ") is";
    }
}

void GatoAttClient::handleServerMTU(uint req, quint16 server_mtu)
{
    Q_UNUSED(req);
    if (server_mtu) {
        cur_mtu = server_mtu;
        if (cur_mtu < ATT_DEFAULT_LE_MTU) {
            cur_mtu = ATT_DEFAULT_LE_MTU;
        }
    }
}

QT_END_NAMESPACE

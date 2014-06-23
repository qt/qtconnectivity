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

#ifndef GATOATTCLIENT_H
#define GATOATTCLIENT_H

#include <QtCore/QObject>
#include <QtCore/QQueue>
#include "gatosocket.h"
#include "gatouuid.h"

class GatoAttClient : public QObject
{
    Q_OBJECT

public:
    explicit GatoAttClient(QObject *parent = 0);
    ~GatoAttClient();

    GatoSocket::State state() const;

    bool connectTo(const GatoAddress& addr);
    void close();

    struct InformationData
    {
        GatoHandle handle;
        GatoUUID uuid;
    };
    struct HandleInformation
    {
        GatoHandle start;
        GatoHandle end;
    };
    struct AttributeData
    {
        GatoHandle handle;
        QByteArray value;
    };
    struct AttributeGroupData
    {
        GatoHandle start;
        GatoHandle end;
        QByteArray value;
    };

    int mtu() const;

    uint request(int opcode, const QByteArray &data, QObject *receiver, const char *member);
    uint requestExchangeMTU(quint16 client_mtu, QObject *receiver, const char *member);
    uint requestFindInformation(GatoHandle start, GatoHandle end, QObject *receiver, const char *member);
    uint requestFindByTypeValue(GatoHandle start, GatoHandle end, const GatoUUID &uuid, const QByteArray& value, QObject *receiver, const char *member);
    uint requestReadByType(GatoHandle start, GatoHandle end, const GatoUUID &uuid, QObject *receiver, const char *member);
    uint requestRead(GatoHandle handle, QObject *receiver, const char *member);
    uint requestReadByGroupType(GatoHandle start, GatoHandle end, const GatoUUID &uuid, QObject *receiver, const char *member);
    uint requestWrite(GatoHandle handle, const QByteArray &value, QObject *receiver, const char *member);
    void cancelRequest(uint id);

    void command(int opcode, const QByteArray &data);
    void commandWrite(GatoHandle handle, const QByteArray &value);

signals:
    void connected();
    void disconnected();

    void attributeUpdated(GatoHandle handle, const QByteArray &value, bool confirmed);

private:
    struct Request
    {
        uint id;
        quint8 opcode;
        QByteArray pkt;
        QObject *receiver;
        QByteArray member;
    };

    void sendARequest();
    bool handleEvent(const QByteArray &event);
    bool handleResponse(const Request& req, const QByteArray &response);

    QList<InformationData> parseInformationData(const QByteArray &data);
    QList<HandleInformation> parseHandleInformation(const QByteArray &data);
    QList<AttributeData> parseAttributeData(const QByteArray &data);
    QList<AttributeGroupData> parseAttributeGroupData(const QByteArray &data);

private slots:
    void handleSocketConnected();
    void handleSocketDisconnected();
    void handleSocketReadyRead();

    void handleServerMTU(uint req, quint16 server_mtu);

private:
    GatoSocket *socket;
    quint16 cur_mtu;
    uint next_id;
    QQueue<Request> pending_requests;
};

#endif // GATOATTCLIENT_H

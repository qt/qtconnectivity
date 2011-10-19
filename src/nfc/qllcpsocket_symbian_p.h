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

#ifndef QLLCPSOCKET_P_H
#define QLLCPSOCKET_P_H

#include <qconnectivityglobal.h>
#include "qllcpsocket.h"

#include <QtCore/QObject>
#include <QSharedPointer>

class CLlcpSocketType1;
class CLlcpSocketType2;

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

class QLLCPSocketState;

class QLlcpSocketPrivate
{
    Q_DECLARE_PUBLIC(QLlcpSocket)

public:

    QLlcpSocketPrivate(QLlcpSocket *q);
    QLlcpSocketPrivate(CLlcpSocketType2* socketType2_symbian);
    ~QLlcpSocketPrivate();

public: //Implementation of QLlcpSocket API
    void connectToService(QNearFieldTarget *target, const QString &serviceUri);
    void disconnectFromService();

    bool bind(quint8 port);

    bool hasPendingDatagrams() const;
    qint64 pendingDatagramSize() const;

    qint64 writeDatagram(const char *data, qint64 size);
    qint64 writeDatagram(const QByteArray &datagram);

    qint64 readDatagram(char *data, qint64 maxSize,
                        QNearFieldTarget **target = 0, quint8 *port = 0);
    qint64 writeDatagram(const char *data, qint64 size,
                         QNearFieldTarget *target, quint8 port);
    qint64 writeDatagram(const QByteArray &datagram, QNearFieldTarget *target, quint8 port);

    QLlcpSocket::SocketError error() const;
    QLlcpSocket::SocketState state() const;

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

    qint64 bytesAvailable() const;
    bool canReadLine() const;

    bool waitForReadyRead(int msecs);
    bool waitForBytesWritten(int msecs);
    bool waitForConnected(int msecs);
    bool waitForDisconnected(int msecs);
    void attachCallbackHandler(QLlcpSocket *q);

public:
    CLlcpSocketType1* socketType1Instance();
    CLlcpSocketType2* socketType2Instance();
    CLlcpSocketType1* socketType1Handler() {return m_symbianSocketType1;}
    CLlcpSocketType2* socketType2Handler() {return m_symbianSocketType2;}

    QLlcpSocket* qllcpsocket(CLlcpSocketType2*);

public:
    enum State {
        ListeningState = QAbstractSocket::ListeningState,
    };

public:
    void invokeReadyRead();
    void invokeBytesWritten(qint64 bytes) ;
    void invokeStateChanged(QLlcpSocket::SocketState socketState);
    void invokeError();
    void invokeDisconnected();
    void invokeConnected();

// state machine part
public:
    void changeState(QLLCPSocketState*);
    QLLCPSocketState* getUnconnectedState() { return m_unconnectedState;}
    QLLCPSocketState* getConnectedState() { return m_connectedState;}
    QLLCPSocketState* getConnectingState() { return m_connectingState;}
    QLLCPSocketState* getBindState() { return m_bindState;}

private:
    CLlcpSocketType1* m_symbianSocketType1;  // own
    CLlcpSocketType2* m_symbianSocketType2; // own

private:
    QLlcpSocket::SocketError m_error;
    QLLCPSocketState* m_state;  // not own
    QLLCPSocketState* m_unconnectedState;  // own
    QLLCPSocketState* m_connectedState; // own
    QLLCPSocketState* m_connectingState; // own
    QLLCPSocketState* m_bindState; // own

    QLlcpSocket *q_ptr; // not own

    bool m_emittedReadyRead;
    bool m_emittedBytesWritten;

public:
    int m_writeDatagramRefCount;
};

QTNFC_END_NAMESPACE

QT_END_HEADER

#endif // QLLCPSOCKET_P_H

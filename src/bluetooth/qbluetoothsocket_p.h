/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#ifndef QBLUETOOTHSOCKET_P_H
#define QBLUETOOTHSOCKET_P_H

#include "qbluetoothsocket.h"

#ifndef QPRIVATELINEARBUFFER_BUFFERSIZE
#define QPRIVATELINEARBUFFER_BUFFERSIZE Q_INT64_C(16384)
#endif
#include "qprivatelinearbuffer_p.h"

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QSocketNotifier)

QT_BEGIN_HEADER

QTBLUETOOTH_BEGIN_NAMESPACE

class QBluetoothServiceDiscoveryAgent;

class QSocketServerPrivate
{
public:
    QSocketServerPrivate();
    ~QSocketServerPrivate();
};



class QBluetoothSocket;
class QBluetoothServiceDiscoveryAgent;

class QBluetoothSocketPrivate
{
    Q_DECLARE_PUBLIC(QBluetoothSocket)
public:

    QBluetoothSocketPrivate();
    ~QBluetoothSocketPrivate();

    void connectToService(const QBluetoothAddress &address, quint16 port, QIODevice::OpenMode openMode);

    bool ensureNativeSocket(QBluetoothSocket::SocketType type);

    QString localName() const;
    QBluetoothAddress localAddress() const;
    quint16 localPort() const;

    QString peerName() const;
    QBluetoothAddress peerAddress() const;
    quint16 peerPort() const;
    //QBluetoothServiceInfo peerService() const;

    void abort();
    void close();

    //qint64 readBufferSize() const;
    //void setReadBufferSize(qint64 size);

    qint64 writeData(const char *data, qint64 maxSize);
    qint64 readData(char *data, qint64 maxSize);

    bool setSocketDescriptor(int socketDescriptor, QBluetoothSocket::SocketType socketType,
                             QBluetoothSocket::SocketState socketState = QBluetoothSocket::ConnectedState,
                             QBluetoothSocket::OpenMode openMode = QBluetoothSocket::ReadWrite);
    int socketDescriptor() const;

    qint64 bytesAvailable() const;

public:
    QPrivateLinearBuffer buffer;
    QPrivateLinearBuffer txBuffer;
    int socket;
    QBluetoothSocket::SocketType socketType;
    QBluetoothSocket::SocketState state;
    QBluetoothSocket::SocketError socketError;
    QSocketNotifier *readNotifier;
    QSocketNotifier *connectWriteNotifier;
    bool connecting;

    QBluetoothServiceDiscoveryAgent *discoveryAgent;
    QBluetoothSocket::OpenMode openMode;


//    QByteArray rxBuffer;
//    qint64 rxOffset;
    QString errorString;

    // private slots
    void _q_readNotify();
    void _q_writeNotify();
    void _q_serviceDiscovered(const QBluetoothServiceInfo &service);
    void _q_discoveryFinished();

protected:
    QBluetoothSocket *q_ptr;

private:
    mutable QString m_localName;
    mutable QString m_peerName;
};


static inline void convertAddress(quint64 from, quint8 (&to)[6])
{
    to[0] = (from >> 0) & 0xff;
    to[1] = (from >> 8) & 0xff;
    to[2] = (from >> 16) & 0xff;
    to[3] = (from >> 24) & 0xff;
    to[4] = (from >> 32) & 0xff;
    to[5] = (from >> 40) & 0xff;
}

static inline void convertAddress(quint8 (&from)[6], quint64 &to)
{
    to = (quint64(from[0]) << 0) |
         (quint64(from[1]) << 8) |
         (quint64(from[2]) << 16) |
         (quint64(from[3]) << 24) |
         (quint64(from[4]) << 32) |
         (quint64(from[5]) << 40);
}

QTBLUETOOTH_END_NAMESPACE

QT_END_HEADER

#endif

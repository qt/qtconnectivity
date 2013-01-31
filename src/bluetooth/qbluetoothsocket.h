/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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

#ifndef QBLUETOOTHSOCKET_H
#define QBLUETOOTHSOCKET_H

#include "qbluetoothglobal.h"

#include <qbluetoothaddress.h>
#include <qbluetoothuuid.h>

#include <QIODevice>
#include <QtNetwork/QAbstractSocket>

QTBLUETOOTH_BEGIN_NAMESPACE

class QBluetoothSocketPrivate;
class QBluetoothServiceInfo;

class Q_BLUETOOTH_EXPORT QBluetoothSocket : public QIODevice
{
    Q_OBJECT    
    Q_DECLARE_PRIVATE(QBluetoothSocket)

    friend class QRfcommServer;
    friend class QRfcommServerPrivate;
    friend class QL2capServer;
    friend class QL2capServerPrivate;

public:
    enum SocketType {
        UnknownSocketType = -1,
        L2capSocket,
        RfcommSocket
    };

    enum SocketState {
        UnconnectedState = QAbstractSocket::UnconnectedState,
        ServiceLookupState = QAbstractSocket::HostLookupState,
        ConnectingState = QAbstractSocket::ConnectingState,
        ConnectedState = QAbstractSocket::ConnectedState,
        BoundState = QAbstractSocket::BoundState,
        ClosingState = QAbstractSocket::ClosingState,
        ListeningState = QAbstractSocket::ListeningState
    };

    enum SocketError {
        NoSocketError = -2,
        UnknownSocketError = QAbstractSocket::UnknownSocketError,
        ConnectionRefusedError = QAbstractSocket::ConnectionRefusedError,        
        RemoteHostClosedError = QAbstractSocket::RemoteHostClosedError,
        HostNotFoundError = QAbstractSocket::HostNotFoundError,
        ServiceNotFoundError = QAbstractSocket::SocketAddressNotAvailableError,
        NetworkError = QAbstractSocket::NetworkError
    };

    explicit QBluetoothSocket(SocketType socketType, QObject *parent = 0);   // create socket of type socketType
    QBluetoothSocket(QObject *parent = 0);  // create a blank socket
    virtual ~QBluetoothSocket();

    void abort();
    virtual void close();

    bool isSequential() const;

    virtual qint64 bytesAvailable() const;
    virtual qint64 bytesToWrite() const;

    virtual bool canReadLine() const;

    void connectToService(const QBluetoothServiceInfo &service, OpenMode openMode = ReadWrite);
    void connectToService(const QBluetoothAddress &address, const QBluetoothUuid &uuid, OpenMode openMode = ReadWrite);
    void connectToService(const QBluetoothAddress &address, quint16 port, OpenMode openMode = ReadWrite);
    void disconnectFromService();

    //bool flush();
    //bool isValid() const;

    QString localName() const;
    QBluetoothAddress localAddress() const;
    quint16 localPort() const;

    QString peerName() const;
    QBluetoothAddress peerAddress() const;
    quint16 peerPort() const;
    //QBluetoothServiceInfo peerService() const;

    //qint64 readBufferSize() const;
    //void setReadBufferSize(qint64 size);

    bool setSocketDescriptor(int socketDescriptor, SocketType socketType,
                             SocketState socketState = ConnectedState,
                             OpenMode openMode = ReadWrite);
    int socketDescriptor() const;

    SocketType socketType() const;
    SocketState state() const;
    SocketError error() const;
    QString errorString() const;

    //bool waitForConnected(int msecs = 30000);
    //bool waitForDisconnected(int msecs = 30000);
    //virtual bool waitForReadyRead(int msecs = 30000);

Q_SIGNALS:
    void connected();
    void disconnected();
    void error(QBluetoothSocket::SocketError error);
    void stateChanged(QBluetoothSocket::SocketState state);

protected:
    virtual qint64 readData(char *data, qint64 maxSize);
    virtual qint64 writeData(const char *data, qint64 maxSize);

    void setSocketState(SocketState state);
    void setSocketError(SocketError error);

    void doDeviceDiscovery(const QBluetoothServiceInfo &service, OpenMode openMode);

private Q_SLOTS:
    void serviceDiscovered(const QBluetoothServiceInfo &service);
    void discoveryFinished();


protected:
    QBluetoothSocketPrivate *d_ptr;

private:
    Q_PRIVATE_SLOT(d_func(), void _q_readNotify())
    Q_PRIVATE_SLOT(d_func(), void _q_writeNotify())
};

#ifndef QT_NO_DEBUG_STREAM
Q_BLUETOOTH_EXPORT QDebug operator<<(QDebug, QBluetoothSocket::SocketError);
Q_BLUETOOTH_EXPORT QDebug operator<<(QDebug, QBluetoothSocket::SocketState);
#endif

QTBLUETOOTH_END_NAMESPACE

#endif

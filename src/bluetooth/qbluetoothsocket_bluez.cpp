/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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

#include "qbluetoothsocket.h"
#include "qbluetoothsocket_p.h"

#include "bluez/manager_p.h"
#include "bluez/adapter_p.h"
#include "bluez/device_p.h"

#include <qplatformdefs.h>

#include <QtCore/QLoggingCategory>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/l2cap.h>

#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <QtCore/QSocketNotifier>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

QBluetoothSocketPrivate::QBluetoothSocketPrivate()
    : socket(-1),
      socketType(QBluetoothServiceInfo::UnknownProtocol),
      state(QBluetoothSocket::UnconnectedState),
      readNotifier(0),
      connectWriteNotifier(0),
      connecting(false),
      discoveryAgent(0)
{
}

QBluetoothSocketPrivate::~QBluetoothSocketPrivate()
{
    delete readNotifier;
    readNotifier = 0;
    delete connectWriteNotifier;
    connectWriteNotifier = 0;
}

bool QBluetoothSocketPrivate::ensureNativeSocket(QBluetoothServiceInfo::Protocol type)
{
    if (socket != -1) {
        if (socketType == type)
            return true;

        delete readNotifier;
        readNotifier = 0;
        delete connectWriteNotifier;
        connectWriteNotifier = 0;
        QT_CLOSE(socket);
    }

    socketType = type;

    switch (type) {
    case QBluetoothServiceInfo::L2capProtocol:
        socket = ::socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
        break;
    case QBluetoothServiceInfo::RfcommProtocol:
        socket = ::socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
        break;
    default:
        socket = -1;
    }

    if (socket == -1)
        return false;

    int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);

    Q_Q(QBluetoothSocket);
    readNotifier = new QSocketNotifier(socket, QSocketNotifier::Read);
    QObject::connect(readNotifier, SIGNAL(activated(int)), q, SLOT(_q_readNotify()));
    connectWriteNotifier = new QSocketNotifier(socket, QSocketNotifier::Write, q);    
    QObject::connect(connectWriteNotifier, SIGNAL(activated(int)), q, SLOT(_q_writeNotify()));

    connectWriteNotifier->setEnabled(false);
    readNotifier->setEnabled(false);


    return true;
}

void QBluetoothSocketPrivate::connectToService(const QBluetoothAddress &address, quint16 port, QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);
    Q_UNUSED(openMode);
    int result = -1;

    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        sockaddr_rc addr;

        memset(&addr, 0, sizeof(addr));
        addr.rc_family = AF_BLUETOOTH;
        addr.rc_channel = port;

        convertAddress(address.toUInt64(), addr.rc_bdaddr.b);

        connectWriteNotifier->setEnabled(true);
        readNotifier->setEnabled(true);QString();

        result = ::connect(socket, (sockaddr *)&addr, sizeof(addr));
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        sockaddr_l2 addr;

        memset(&addr, 0, sizeof(addr));
        addr.l2_family = AF_BLUETOOTH;
        addr.l2_psm = port;

        convertAddress(address.toUInt64(), addr.l2_bdaddr.b);

        connectWriteNotifier->setEnabled(true);
        readNotifier->setEnabled(true);

        result = ::connect(socket, (sockaddr *)&addr, sizeof(addr));
    }

    if (result >= 0 || (result == -1 && errno == EINPROGRESS)) {
        connecting = true;
        q->setSocketState(QBluetoothSocket::ConnectingState);
    } else {
        errorString = QString::fromLocal8Bit(strerror(errno));
        q->setSocketError(QBluetoothSocket::UnknownSocketError);
    }
}

void QBluetoothSocketPrivate::_q_writeNotify()
{
    Q_Q(QBluetoothSocket);
    if(connecting && state == QBluetoothSocket::ConnectingState){
        int errorno, len;
        len = sizeof(errorno);
        ::getsockopt(socket, SOL_SOCKET, SO_ERROR, &errorno, (socklen_t*)&len);
        if(errorno) {
            errorString = QString::fromLocal8Bit(strerror(errorno));
            emit q->error(QBluetoothSocket::UnknownSocketError);
            return;
        }

        q->setSocketState(QBluetoothSocket::ConnectedState);
        emit q->connected();

        connectWriteNotifier->setEnabled(false);
        connecting = false;
    }
    else {
        if (txBuffer.size() == 0) {
            connectWriteNotifier->setEnabled(false);
            return;
        }

        char buf[1024];
        Q_Q(QBluetoothSocket);

        int size = txBuffer.read(buf, 1024);

        if (::write(socket, buf, size) != size) {
            socketError = QBluetoothSocket::NetworkError;
            emit q->error(socketError);
        }
        else {
            emit q->bytesWritten(size);
        }

        if (txBuffer.size()) {
            connectWriteNotifier->setEnabled(true);
        }
        else if (state == QBluetoothSocket::ClosingState) {
            connectWriteNotifier->setEnabled(false);
            this->close();
        }
    }    
}

// TODO: move to private backend?

void QBluetoothSocketPrivate::_q_readNotify()
{
    Q_Q(QBluetoothSocket);
    char *writePointer = buffer.reserve(QPRIVATELINEARBUFFER_BUFFERSIZE);
//    qint64 readFromDevice = q->readData(writePointer, QPRIVATELINEARBUFFER_BUFFERSIZE);
    int readFromDevice = ::read(socket, writePointer, QPRIVATELINEARBUFFER_BUFFERSIZE);
    if(readFromDevice <= 0){
        int errsv = errno;
        readNotifier->setEnabled(false);
        connectWriteNotifier->setEnabled(false);
        errorString = QString::fromLocal8Bit(strerror(errsv));
        qCWarning(QT_BT_BLUEZ) << Q_FUNC_INFO << socket << "error:" << readFromDevice << errorString;
        if(errsv == EHOSTDOWN)
            emit q->error(QBluetoothSocket::HostNotFoundError);
        else
            emit q->error(QBluetoothSocket::UnknownSocketError);

        q->disconnectFromService();
        q->setSocketState(QBluetoothSocket::UnconnectedState);        
    }
    else {
        buffer.chop(QPRIVATELINEARBUFFER_BUFFERSIZE - (readFromDevice < 0 ? 0 : readFromDevice));

        emit q->readyRead();
    }
}

void QBluetoothSocketPrivate::abort()
{
    delete readNotifier;
    readNotifier = 0;
    delete connectWriteNotifier;
    connectWriteNotifier = 0;

    // We don't transition through Closing for abort, so
    // we don't call disconnectFromService or
    // QBluetoothSocket::close
    QT_CLOSE(socket);

    Q_Q(QBluetoothSocket);
    emit q->disconnected();
}

QString QBluetoothSocketPrivate::localName() const
{
    if (!m_localName.isEmpty())
        return m_localName;

    const QBluetoothAddress address = localAddress();
    if (address.isNull())
        return QString();

    OrgBluezManagerInterface manager(QLatin1String("org.bluez"), QLatin1String("/"),
                                     QDBusConnection::systemBus());

    QDBusPendingReply<QDBusObjectPath> reply = manager.FindAdapter(address.toString());
    reply.waitForFinished();
    if (reply.isError())
        return QString();

    OrgBluezAdapterInterface adapter(QLatin1String("org.bluez"), reply.value().path(),
                                     QDBusConnection::systemBus());

    QDBusPendingReply<QVariantMap> properties = adapter.GetProperties();
    properties.waitForFinished();
    if (properties.isError())
        return QString();

    m_localName = properties.value().value(QLatin1String("Name")).toString();

    return m_localName;
}

QBluetoothAddress QBluetoothSocketPrivate::localAddress() const
{
    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        sockaddr_rc addr;
        socklen_t addrLength = sizeof(addr);

        if (::getsockname(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0) {
            quint64 bdaddr;
            convertAddress(addr.rc_bdaddr.b, bdaddr);
            return QBluetoothAddress(bdaddr);
        }
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        sockaddr_l2 addr;
        socklen_t addrLength = sizeof(addr);

        if (::getsockname(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0) {
            quint64 bdaddr;
            convertAddress(addr.l2_bdaddr.b, bdaddr);
            return QBluetoothAddress(bdaddr);
        }
    }

    return QBluetoothAddress();
}

quint16 QBluetoothSocketPrivate::localPort() const
{
    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        sockaddr_rc addr;
        socklen_t addrLength = sizeof(addr);

        if (::getsockname(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0)
            return addr.rc_channel;
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        sockaddr_l2 addr;
        socklen_t addrLength = sizeof(addr);

        if (::getsockname(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0)
            return addr.l2_psm;
    }

    return 0;
}

QString QBluetoothSocketPrivate::peerName() const
{
    if (!m_peerName.isEmpty())
        return m_peerName;

    quint64 bdaddr;

    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        sockaddr_rc addr;
        socklen_t addrLength = sizeof(addr);

        if (::getpeername(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) < 0)
            return QString();

        convertAddress(addr.rc_bdaddr.b, bdaddr);
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        sockaddr_l2 addr;
        socklen_t addrLength = sizeof(addr);

        if (::getpeername(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) < 0)
            return QString();

        convertAddress(addr.l2_bdaddr.b, bdaddr);
    } else {
        qCWarning(QT_BT_BLUEZ) << "peerName() called on socket of known type";
        return QString();
    }

    const QString peerAddress = QBluetoothAddress(bdaddr).toString();
    const QString localAdapter = localAddress().toString();

    OrgBluezManagerInterface manager(QLatin1String("org.bluez"), QLatin1String("/"),
                                     QDBusConnection::systemBus());

    QDBusPendingReply<QDBusObjectPath> reply = manager.FindAdapter(localAdapter);
    reply.waitForFinished();
    if (reply.isError())
        return QString();

    OrgBluezAdapterInterface adapter(QLatin1String("org.bluez"), reply.value().path(),
                                     QDBusConnection::systemBus());

    QDBusPendingReply<QDBusObjectPath> deviceObjectPath = adapter.CreateDevice(peerAddress);
    deviceObjectPath.waitForFinished();
    if (deviceObjectPath.isError()) {
        if (deviceObjectPath.error().name() != QLatin1String("org.bluez.Error.AlreadyExists"))
            return QString();

        deviceObjectPath = adapter.FindDevice(peerAddress);
        deviceObjectPath.waitForFinished();
        if (deviceObjectPath.isError())
            return QString();
    }

    OrgBluezDeviceInterface device(QLatin1String("org.bluez"), deviceObjectPath.value().path(),
                                   QDBusConnection::systemBus());

    QDBusPendingReply<QVariantMap> properties = device.GetProperties();
    properties.waitForFinished();
    if (properties.isError())
        return QString();

    m_peerName = properties.value().value(QLatin1String("Alias")).toString();

    return m_peerName;
}

QBluetoothAddress QBluetoothSocketPrivate::peerAddress() const
{
    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        sockaddr_rc addr;
        socklen_t addrLength = sizeof(addr);

        if (::getpeername(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0) {
            quint64 bdaddr;
            convertAddress(addr.rc_bdaddr.b, bdaddr);
            return QBluetoothAddress(bdaddr);
        }
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        sockaddr_l2 addr;
        socklen_t addrLength = sizeof(addr);

        if (::getpeername(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0) {
            quint64 bdaddr;
            convertAddress(addr.l2_bdaddr.b, bdaddr);
            return QBluetoothAddress(bdaddr);
        }
    }

    return QBluetoothAddress();
}

quint16 QBluetoothSocketPrivate::peerPort() const
{
    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        sockaddr_rc addr;
        socklen_t addrLength = sizeof(addr);

        if (::getpeername(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0)
            return addr.rc_channel;
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        sockaddr_l2 addr;
        socklen_t addrLength = sizeof(addr);

        if (::getpeername(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0)
            return addr.l2_psm;
    }

    return 0;
}

qint64 QBluetoothSocketPrivate::writeData(const char *data, qint64 maxSize)
{
    Q_Q(QBluetoothSocket);
    if (q->openMode() & QIODevice::Unbuffered) {
        if (::write(socket, data, maxSize) != maxSize) {
            socketError = QBluetoothSocket::NetworkError;
            emit q->error(socketError);
        }

        emit q->bytesWritten(maxSize);

        return maxSize;
    }
    else {

        if(!connectWriteNotifier)
            return 0;

        if(txBuffer.size() == 0) {
            connectWriteNotifier->setEnabled(true);        
            QMetaObject::invokeMethod(q, "_q_writeNotify", Qt::QueuedConnection);
        }

        char *txbuf = txBuffer.reserve(maxSize);
        memcpy(txbuf, data, maxSize);

        return maxSize;
    }
}

qint64 QBluetoothSocketPrivate::readData(char *data, qint64 maxSize)
{
    if(!buffer.isEmpty()){
        int i = buffer.read(data, maxSize);
        return i;

    }
    return 0;
}

void QBluetoothSocketPrivate::close()
{
    Q_Q(QBluetoothSocket);

    // Only go through closing if the socket was fully opened
    if(state == QBluetoothSocket::ConnectedState)
        q->setSocketState(QBluetoothSocket::ClosingState);

    if(txBuffer.size() > 0 &&
       state == QBluetoothSocket::ClosingState){
        connectWriteNotifier->setEnabled(true);
    }
    else {

        delete readNotifier;
        readNotifier = 0;
        delete connectWriteNotifier;
        connectWriteNotifier = 0;

        // We are disconnected now, so go to unconnected.
        q->setSocketState(QBluetoothSocket::UnconnectedState);
        emit q->disconnected();
        ::close(socket);
    }

}

bool QBluetoothSocketPrivate::setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType_,
                                           QBluetoothSocket::SocketState socketState, QBluetoothSocket::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);
    delete readNotifier;
    readNotifier = 0;
    delete connectWriteNotifier;
    connectWriteNotifier = 0;

    socketType = socketType_;
    socket = socketDescriptor;

    // ensure that O_NONBLOCK is set on new connections.
    int flags = fcntl(socket, F_GETFL, 0);
    if (!(flags & O_NONBLOCK))
        fcntl(socket, F_SETFL, flags | O_NONBLOCK);

    readNotifier = new QSocketNotifier(socket, QSocketNotifier::Read);
    QObject::connect(readNotifier, SIGNAL(activated(int)), q, SLOT(_q_readNotify()));
    connectWriteNotifier = new QSocketNotifier(socket, QSocketNotifier::Write, q);
    QObject::connect(connectWriteNotifier, SIGNAL(activated(int)), q, SLOT(_q_writeNotify()));

    q->setSocketState(socketState);
    q->setOpenMode(openMode);

    return true;
}

int QBluetoothSocketPrivate::socketDescriptor() const
{
    return socket;
}

qint64 QBluetoothSocketPrivate::bytesAvailable() const
{
    return buffer.size();
}

QT_END_NAMESPACE

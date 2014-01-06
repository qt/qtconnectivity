/***************************************************************************
**
** Copyright (C) 2012 Research In Motion
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
#include "qbluetoothlocaldevice.h"
#include <sys/stat.h>

QT_BEGIN_NAMESPACE

QBluetoothSocketPrivate::QBluetoothSocketPrivate()
    : socket(-1),
      socketType(QBluetoothServiceInfo::UnknownProtocol),
      state(QBluetoothSocket::UnconnectedState),
      readNotifier(0),
      connectWriteNotifier(0),
      connecting(false),
      discoveryAgent(0),
      isServerSocket(false)
{
    ppsRegisterControl();
}

QBluetoothSocketPrivate::~QBluetoothSocketPrivate()
{
    ppsUnregisterControl(this);
    close();
}

bool QBluetoothSocketPrivate::ensureNativeSocket(QBluetoothServiceInfo::Protocol type)
{
    socketType = type;
    return false;
}

void QBluetoothSocketPrivate::connectToService(const QBluetoothAddress &address, QBluetoothUuid uuid, QIODevice::OpenMode openMode)
{
    Q_UNUSED(openMode);
    qCDebug(QT_BT_QNX) << "Connecting socket";
    if (isServerSocket) {
        m_peerAddress = address;
        m_uuid = uuid;
        return;
    }

    if (state != QBluetoothSocket::UnconnectedState) {
        qCDebug(QT_BT_QNX) << "Socket already connected";
        return;
    }
    state = QBluetoothSocket::ConnectingState;

    m_uuid = uuid;
    m_peerAddress = address;

    ppsSendControlMessage("connect_service", 0x1101, uuid, address.toString(), QString(), this, BT_SPP_CLIENT_SUBTYPE);
    ppsRegisterForEvent(QStringLiteral("service_connected"),this);
    ppsRegisterForEvent(QStringLiteral("get_mount_point_path"),this);
    socketType = QBluetoothServiceInfo::RfcommProtocol;
}

void QBluetoothSocketPrivate::_q_writeNotify()
{
    Q_Q(QBluetoothSocket);
    if (connecting && state == QBluetoothSocket::ConnectingState){
        q->setSocketState(QBluetoothSocket::ConnectedState);
        emit q->connected();

        connectWriteNotifier->setEnabled(false);
        connecting = false;
    } else {
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

void QBluetoothSocketPrivate::_q_readNotify()
{
    Q_Q(QBluetoothSocket);
    char *writePointer = buffer.reserve(QPRIVATELINEARBUFFER_BUFFERSIZE);
    int readFromDevice = ::read(socket, writePointer, QPRIVATELINEARBUFFER_BUFFERSIZE);
    if (readFromDevice <= 0){
        int errsv = errno;
        readNotifier->setEnabled(false);
        connectWriteNotifier->setEnabled(false);
        errorString = QString::fromLocal8Bit(strerror(errsv));
        qCWarning(QT_BT_QNX) << Q_FUNC_INFO << socket << " error:" << readFromDevice << errorString; //TODO Try if this actually works
        emit q->error(QBluetoothSocket::UnknownSocketError);

        q->disconnectFromService();
        q->setSocketState(QBluetoothSocket::UnconnectedState);
    } else {
        buffer.chop(QPRIVATELINEARBUFFER_BUFFERSIZE - (readFromDevice < 0 ? 0 : readFromDevice));

        emit q->readyRead();
    }
}

void QBluetoothSocketPrivate::abort()
{
    Q_Q(QBluetoothSocket);
    qCDebug(QT_BT_QNX) << "Disconnecting service";
    if (q->state() != QBluetoothSocket::ClosingState)
        ppsSendControlMessage("disconnect_service", 0x1101, m_uuid, m_peerAddress.toString(), QString(), 0,
                          isServerSocket ? BT_SPP_SERVER_SUBTYPE : BT_SPP_CLIENT_SUBTYPE);
    delete readNotifier;
    readNotifier = 0;
    delete connectWriteNotifier;
    connectWriteNotifier = 0;

    ::close(socket);

    q->setSocketState(QBluetoothSocket::UnconnectedState);
    Q_EMIT q->disconnected();
    isServerSocket = false;
}

QString QBluetoothSocketPrivate::localName() const
{
    QBluetoothLocalDevice ld;
    return ld.name();
}

QBluetoothAddress QBluetoothSocketPrivate::localAddress() const
{
    QBluetoothLocalDevice ld;
    return ld.address();
}

quint16 QBluetoothSocketPrivate::localPort() const
{
    return 0;
}

QString QBluetoothSocketPrivate::peerName() const
{
    return m_peerName;
}

QBluetoothAddress QBluetoothSocketPrivate::peerAddress() const
{
    return m_peerAddress;
}

quint16 QBluetoothSocketPrivate::peerPort() const
{
    return 0;
}

qint64 QBluetoothSocketPrivate::writeData(const char *data, qint64 maxSize)
{
    Q_Q(QBluetoothSocket);
    if (q->openMode() & QIODevice::Unbuffered) {
        if (::write(socket, data, maxSize) != maxSize) {
            socketError = QBluetoothSocket::NetworkError;
            qCWarning(QT_BT_QNX) << Q_FUNC_INFO << "Socket error";
            Q_EMIT q->error(socketError);
        }

        Q_EMIT q->bytesWritten(maxSize);

        return maxSize;
    } else {
        if (!connectWriteNotifier)
            return 0;

        if (txBuffer.size() == 0) {
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
    if (!buffer.isEmpty()){
        int i = buffer.read(data, maxSize);
        return i;
    }
    return 0;
}

void QBluetoothSocketPrivate::close()
{
    abort();
}

bool QBluetoothSocketPrivate::setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType,
                                                  QBluetoothSocket::SocketState socketState, QBluetoothSocket::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);
    delete readNotifier;
    readNotifier = 0;
    delete connectWriteNotifier;
    connectWriteNotifier = 0;

    socket = socketDescriptor;
    socketType = socketType;

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

    if (openMode == QBluetoothSocket::ConnectedState)
        emit q->connected();

    isServerSocket = true;
    ppsRegisterForEvent(QStringLiteral("service_disconnected"),this);

    return true;
}

int QBluetoothSocketPrivate::socketDescriptor() const
{
    return 0;
}

qint64 QBluetoothSocketPrivate::bytesAvailable() const
{
    return buffer.size();
}

void QBluetoothSocketPrivate::controlReply(ppsResult result)
{
    Q_Q(QBluetoothSocket);

    if (result.msg == QStringLiteral("connect_service")) {
        if (!result.errorMsg.isEmpty()) {
            qCWarning(QT_BT_QNX) << Q_FUNC_INFO << "Error connecting to service:" << result.errorMsg;
            errorString = result.errorMsg;
            socketError = QBluetoothSocket::UnknownSocketError;
            emit q->error(QBluetoothSocket::UnknownSocketError);
            q->setSocketState(QBluetoothSocket::UnconnectedState);
            return;
        } else {
            qCDebug(QT_BT_QNX) << Q_FUNC_INFO << "Sending request for mount point";
            ppsSendControlMessage("get_mount_point_path", 0x1101, m_uuid, m_peerAddress.toString(), QString(), this, BT_SPP_CLIENT_SUBTYPE);
        }
    } else if (result.msg == QStringLiteral("get_mount_point_path")) {
        QString path;
        path = result.dat.first();
        qCDebug(QT_BT_QNX) << Q_FUNC_INFO << "PATH is" << path;

        if (!result.errorMsg.isEmpty()) {
            qCWarning(QT_BT_QNX) << Q_FUNC_INFO << result.errorMsg;
            errorString = result.errorMsg;
            socketError = QBluetoothSocket::UnknownSocketError;
            emit q->error(QBluetoothSocket::UnknownSocketError);
            q->setSocketState(QBluetoothSocket::UnconnectedState);
            return;
        } else {
            qCDebug(QT_BT_QNX) << "Mount point path is:" << path;
            socket = ::open(path.toStdString().c_str(), O_RDWR);
            if (socket == -1) {
                errorString = QString::fromLocal8Bit(strerror(errno));
                qCWarning(QT_BT_QNX) << Q_FUNC_INFO << socket << " error:" << errno << errorString; //TODO Try if this actually works
                emit q->error(QBluetoothSocket::UnknownSocketError);

                q->disconnectFromService();
                q->setSocketState(QBluetoothSocket::UnconnectedState);
                return;
            }

            Q_Q(QBluetoothSocket);
            readNotifier = new QSocketNotifier(socket, QSocketNotifier::Read);
            QObject::connect(readNotifier, SIGNAL(activated(int)), q, SLOT(_q_readNotify()));
            connectWriteNotifier = new QSocketNotifier(socket, QSocketNotifier::Write, q);
            QObject::connect(connectWriteNotifier, SIGNAL(activated(int)), q, SLOT(_q_writeNotify()));

            connectWriteNotifier->setEnabled(true);
            readNotifier->setEnabled(true);
            emit q->connected();
            ppsRegisterForEvent(QStringLiteral("service_disconnected"),this);
        }
    }
}

void QBluetoothSocketPrivate::controlEvent(ppsResult result)
{
    Q_Q(QBluetoothSocket);
    if (result.msg == QStringLiteral("service_disconnected")) {
        q->setSocketState(QBluetoothSocket::ClosingState);
        close();
    }
}

QT_END_NAMESPACE


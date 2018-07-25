/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbluetoothsocket.h"
#include "qbluetoothsocket_win_p.h"

#include <QtCore/qloggingcategory.h>

#include <QtBluetooth/qbluetoothdeviceinfo.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

QBluetoothSocketPrivateWin::QBluetoothSocketPrivateWin()
    : QBluetoothSocketBasePrivate()
{
}

QBluetoothSocketPrivateWin::~QBluetoothSocketPrivateWin()
{
}

bool QBluetoothSocketPrivateWin::ensureNativeSocket(QBluetoothServiceInfo::Protocol type)
{
    socketType = type;
    return false;
}

void QBluetoothSocketPrivateWin::connectToServiceHelper(const QBluetoothAddress &address, quint16 port, QIODevice::OpenMode openMode)
{
    Q_UNUSED(openMode);
    Q_UNUSED(address);
    Q_UNUSED(port);
}

void QBluetoothSocketPrivateWin::connectToService(
        const QBluetoothServiceInfo &service, QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);

    if (q->state() != QBluetoothSocket::UnconnectedState
            && q->state() != QBluetoothSocket::ServiceLookupState) {
        //qCWarning(QT_BT_WINDOWS) << "QBluetoothSocketPrivateWIN::connectToService called on busy socket";
        errorString = QBluetoothSocket::tr("Trying to connect while connection is in progress");
        q->setSocketError(QBluetoothSocket::OperationError);
        return;
    }

    // we are checking the service protocol and not socketType()
    // socketType will change in ensureNativeSocket()
    if (service.socketProtocol() == QBluetoothServiceInfo::UnknownProtocol) {
        qCWarning(QT_BT_WINDOWS) << "QBluetoothSocket::connectToService cannot "
                                  "connect with 'UnknownProtocol' (type provided by given service)";
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q->setSocketError(QBluetoothSocket::UnsupportedProtocolError);
        return;
    }

    if (service.protocolServiceMultiplexer() > 0) {
        Q_ASSERT(service.socketProtocol() == QBluetoothServiceInfo::L2capProtocol);

        if (!ensureNativeSocket(QBluetoothServiceInfo::L2capProtocol)) {
            errorString = QBluetoothSocket::tr("Unknown socket error");
            q->setSocketError(QBluetoothSocket::UnknownSocketError);
            return;
        }
        connectToServiceHelper(service.device().address(), service.protocolServiceMultiplexer(),
                               openMode);
    } else if (service.serverChannel() > 0) {
        Q_ASSERT(service.socketProtocol() == QBluetoothServiceInfo::RfcommProtocol);

        if (!ensureNativeSocket(QBluetoothServiceInfo::RfcommProtocol)) {
            errorString = QBluetoothSocket::tr("Unknown socket error");
            q->setSocketError(QBluetoothSocket::UnknownSocketError);
            return;
        }
        connectToServiceHelper(service.device().address(), service.serverChannel(), openMode);
    } else {
        // try doing service discovery to see if we can find the socket
        if (service.serviceUuid().isNull()
                && !service.serviceClassUuids().contains(QBluetoothUuid::SerialPort)) {
            qCWarning(QT_BT_WINDOWS) << "No port, no PSM, and no UUID provided. Unable to connect";
            return;
        }
        qCDebug(QT_BT_WINDOWS) << "Need a port/psm, doing discovery";
        q->doDeviceDiscovery(service, openMode);
    }
}

void QBluetoothSocketPrivateWin::connectToService(
        const QBluetoothAddress &address, const QBluetoothUuid &uuid,
        QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);

    if (q->state() != QBluetoothSocket::UnconnectedState) {
        qCWarning(QT_BT_WINDOWS) << "QBluetoothSocketPrivateWin::connectToService called on busy socket";
        errorString = QBluetoothSocket::tr("Trying to connect while connection is in progress");
        q->setSocketError(QBluetoothSocket::OperationError);
        return;
    }

    if (q->socketType() == QBluetoothServiceInfo::UnknownProtocol) {
        qCWarning(QT_BT_WINDOWS) << "QBluetoothSocketPrivateWin::connectToService cannot "
                                  "connect with 'UnknownProtocol' (type provided by given service)";
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q->setSocketError(QBluetoothSocket::UnsupportedProtocolError);
        return;
    }

    QBluetoothServiceInfo service;
    QBluetoothDeviceInfo device(address, QString(), QBluetoothDeviceInfo::MiscellaneousDevice);
    service.setDevice(device);
    service.setServiceUuid(uuid);
    q->doDeviceDiscovery(service, openMode);
}

void QBluetoothSocketPrivateWin::connectToService(
        const QBluetoothAddress &address, quint16 port, QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);

    if (q->socketType() == QBluetoothServiceInfo::UnknownProtocol) {
        qCWarning(QT_BT_WINDOWS) << "QBluetoothSocketPrivateWin::connectToService cannot "
                                  "connect with 'UnknownProtocol' (type provided by given service)";
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q->setSocketError(QBluetoothSocket::UnsupportedProtocolError);
        return;
    }

    if (q->state() != QBluetoothSocket::UnconnectedState) {
        qCWarning(QT_BT_WINDOWS) << "QBluetoothSocketPrivateWin::connectToService called on busy socket";
        errorString = QBluetoothSocket::tr("Trying to connect while connection is in progress");
        q->setSocketError(QBluetoothSocket::OperationError);
        return;
    }

    q->setOpenMode(openMode);
    connectToServiceHelper(address, port, openMode);
}

void QBluetoothSocketPrivateWin::abort()
{
}

QString QBluetoothSocketPrivateWin::localName() const
{
    return QString();
}

QBluetoothAddress QBluetoothSocketPrivateWin::localAddress() const
{
    return QBluetoothAddress();
}

quint16 QBluetoothSocketPrivateWin::localPort() const
{
    return 0;
}

QString QBluetoothSocketPrivateWin::peerName() const
{
    return QString();
}

QBluetoothAddress QBluetoothSocketPrivateWin::peerAddress() const
{
    return QBluetoothAddress();
}

quint16 QBluetoothSocketPrivateWin::peerPort() const
{
    return 0;
}

qint64 QBluetoothSocketPrivateWin::writeData(const char *data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);

    Q_Q(QBluetoothSocket);

    if (state != QBluetoothSocket::ConnectedState) {
        errorString = QBluetoothSocket::tr("Cannot write while not connected");
        q->setSocketError(QBluetoothSocket::OperationError);
        return -1;
    }
    return -1;
}

qint64 QBluetoothSocketPrivateWin::readData(char *data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);

    Q_Q(QBluetoothSocket);

    if (state != QBluetoothSocket::ConnectedState) {
        errorString = QBluetoothSocket::tr("Cannot write while not connected");
        q->setSocketError(QBluetoothSocket::OperationError);
        return -1;
    }

    return -1;
}

void QBluetoothSocketPrivateWin::close()
{
}

bool QBluetoothSocketPrivateWin::setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType,
                                           QBluetoothSocket::SocketState socketState, QBluetoothSocket::OpenMode openMode)
{
    Q_UNUSED(socketDescriptor);
    Q_UNUSED(socketType)
    Q_UNUSED(socketState);
    Q_UNUSED(openMode);
    return false;
}

qint64 QBluetoothSocketPrivateWin::bytesAvailable() const
{
    return 0;
}

bool QBluetoothSocketPrivateWin::canReadLine() const
{
   return false;
}

qint64 QBluetoothSocketPrivateWin::bytesToWrite() const
{
   return 0;
}

QT_END_NAMESPACE

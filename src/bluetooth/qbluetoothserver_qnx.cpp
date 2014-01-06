/***************************************************************************
**
** Copyright (C) 2012 - 2013 BlackBerry Limited. All rights reserved.
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

#include "qbluetoothserver.h"
#include "qbluetoothserver_p.h"
#include "qbluetoothsocket.h"
#include "qbluetoothsocket_p.h"
#include "qbluetoothlocaldevice.h"

#include <QSocketNotifier>

#include <QCoreApplication>

QT_BEGIN_NAMESPACE

extern QHash<QBluetoothServerPrivate*, int> __fakeServerPorts;

QBluetoothServerPrivate::QBluetoothServerPrivate(QBluetoothServiceInfo::Protocol sType)
    : socket(0),maxPendingConnections(1), securityFlags(QBluetooth::NoSecurity), serverType(sType),
      m_lastError(QBluetoothServer::NoError)
{
    ppsRegisterControl();
}

QBluetoothServerPrivate::~QBluetoothServerPrivate()
{
    Q_Q(QBluetoothServer);
    q->close();
    __fakeServerPorts.remove(this);
    ppsUnregisterControl(this);
}

void QBluetoothServerPrivate::controlReply(ppsResult result)
{
    Q_Q(QBluetoothServer);

    if (result.msg == QStringLiteral("register_server")) {
        qCDebug(QT_BT_QNX) << "SPP: Server registration succesfull";

    } else if (result.msg == QStringLiteral("get_mount_point_path")) {
        qCDebug(QT_BT_QNX) << "SPP: Mount point for server" << result.dat.first();

        int socketFD = ::open(result.dat.first().toStdString().c_str(), O_RDWR | O_NONBLOCK);
        if (socketFD == -1) {
            m_lastError = QBluetoothServer::InputOutputError;
            emit q->error(m_lastError);
            qCWarning(QT_BT_QNX) << Q_FUNC_INFO << "RFCOMM Server: Could not open socket FD" << errno;
        } else {
            if (!socket) { // Should never happen
                qCWarning(QT_BT_QNX) << "Socket not valid";
                m_lastError = QBluetoothServer::UnknownError;
                emit q->error(m_lastError);
                return;
            }

            socket->setSocketDescriptor(socketFD, QBluetoothServiceInfo::RfcommProtocol,
                                           QBluetoothSocket::ConnectedState);
            socket->connectToService(QBluetoothAddress(nextClientAddress), m_uuid);
            activeSockets.append(socket);
            socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
            socket->setSocketState(QBluetoothSocket::ListeningState);
            emit q->newConnection();
        }
    }
}

void QBluetoothServerPrivate::controlEvent(ppsResult result)
{
    Q_Q(QBluetoothServer);
    if (result.msg == QStringLiteral("service_connected")) {
        qCDebug(QT_BT_QNX) << "SPP: Server: Sending request for mount point path";
        qCDebug(QT_BT_QNX) << result.dat;
        for (int i=0; i<result.dat.size(); i++) {
            qCDebug(QT_BT_QNX) << result.dat.at(i);
        }

        if (result.dat.contains(QStringLiteral("addr")) && result.dat.contains(QStringLiteral("uuid"))
                && result.dat.contains(QStringLiteral("subtype"))) {
            nextClientAddress = result.dat.at(result.dat.indexOf(QStringLiteral("addr")) + 1);
            m_uuid = QBluetoothUuid(result.dat.at(result.dat.indexOf(QStringLiteral("uuid")) + 1));
            int subtype = result.dat.at(result.dat.indexOf(QStringLiteral("subtype")) + 1).toInt();
            qCDebug(QT_BT_QNX) << "Getting mount point path" << m_uuid << nextClientAddress<< subtype;
            ppsSendControlMessage("get_mount_point_path", 0x1101, m_uuid, nextClientAddress,
                                  m_serviceName, this, BT_SPP_SERVER_SUBTYPE);
        } else {
            m_lastError = QBluetoothServer::InputOutputError;
            emit q->error(m_lastError);
            qCWarning(QT_BT_QNX) << Q_FUNC_INFO << "address not specified in service connect reply";
        }
    }
}

void QBluetoothServer::close()
{
    Q_D(QBluetoothServer);
    if (!d->socket) {
        d->m_lastError = UnknownError;
        emit error(d->m_lastError);
        return;
    }
    d->socket->close();
    delete d->socket;
    d->socket = 0;
    ppsSendControlMessage("deregister_server", 0x1101, d->m_uuid, QString(), QString(), 0);
    // force active object (socket) to run and shutdown socket.
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

bool QBluetoothServer::listen(const QBluetoothAddress &address, quint16 port)
{
    Q_UNUSED(address)
    Q_D(QBluetoothServer);
    if (serverType() != QBluetoothServiceInfo::RfcommProtocol) {
        d->m_lastError = UnsupportedProtocolError;
        emit error(d->m_lastError);
        return false;
    }

    // listen has already been called before
    if (d->socket && d->socket->state() == QBluetoothSocket::ListeningState)
        return true;

    d->socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);

    //We can not register an actual Rfcomm port, because the platform does not allow it
    //but we need a way to associate a server with a service

    if (port == 0) { //Try to assign a non taken port id
        for (int i=1; ; i++){
            if (__fakeServerPorts.key(i) == 0) {
                port = i;
                break;
            }
        }
    }

    if (__fakeServerPorts.key(port) == 0) {
        __fakeServerPorts[d] = port;
        qCDebug(QT_BT_QNX) << "Port" << port << "registered";
    } else {
        qCWarning(QT_BT_QNX) << "server with port" << port << "already registered or port invalid";
        d->m_lastError = ServiceAlreadyRegisteredError;
        emit error(d->m_lastError);
        return false;
    }

    d->socket->setSocketState(QBluetoothSocket::ListeningState);

    ppsRegisterForEvent(QStringLiteral("service_connected"),d);
    return true;
}

void QBluetoothServer::setMaxPendingConnections(int numConnections)
{
    Q_D(QBluetoothServer);
    d->maxPendingConnections = numConnections; //Currently not used
}

QBluetoothAddress QBluetoothServer::serverAddress() const
{
    Q_D(const QBluetoothServer);
    if (d->socket)
        return d->socket->localAddress();
    else
        return QBluetoothAddress();
}

quint16 QBluetoothServer::serverPort() const
{
    //Currently we do not have access to the port
    Q_D(const QBluetoothServer);
    return __fakeServerPorts.value((QBluetoothServerPrivate*)d);
}

bool QBluetoothServer::hasPendingConnections() const
{
    Q_D(const QBluetoothServer);
    return !d->activeSockets.isEmpty();
}

QBluetoothSocket *QBluetoothServer::nextPendingConnection()
{
    Q_D(QBluetoothServer);
    if (d->activeSockets.isEmpty())
        return 0;

    return d->activeSockets.takeFirst();
}

void QBluetoothServer::setSecurityFlags(QBluetooth::SecurityFlags security)
{
    Q_D(QBluetoothServer);
    d->securityFlags = security; //not used
}

QBluetooth::SecurityFlags QBluetoothServer::securityFlags() const
{
    Q_D(const QBluetoothServer);
    return d->securityFlags; //not used
}

QT_END_NAMESPACE


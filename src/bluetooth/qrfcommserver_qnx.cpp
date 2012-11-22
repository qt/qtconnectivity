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

#include "qrfcommserver.h"
#include "qrfcommserver_p.h"
#include "qbluetoothsocket.h"
#include "qbluetoothsocket_p.h"
#include "qbluetoothlocaldevice.h"

#include <QSocketNotifier>

#include <QCoreApplication>

QTBLUETOOTH_BEGIN_NAMESPACE

QRfcommServerPrivate::QRfcommServerPrivate()
    : socket(0),maxPendingConnections(1),securityFlags(QBluetooth::NoSecurity)
{
    ppsRegisterControl();
    rfcomm_server_p = this;
    serverRegistered = false;
}

QRfcommServerPrivate::~QRfcommServerPrivate()
{
    if (serverRegistered) {
        ppsSendControlMessage("deregister_server", 0x1101, m_uuid, QString(), this);
    }
    delete socket;
    ppsUnregisterControl();
}

void QRfcommServerPrivate::registerService(QBluetoothUuid uuid)
{
    if (serverRegistered)
        ppsSendControlMessage("deregister_server", 0x1101, m_uuid, QString(), this);

    serverRegistered = true;
    m_uuid = uuid;
    qBBBluetoothDebug() << "deregistering server";
    ppsSendControlMessage("deregister_server", 0x1101, uuid, QString(), this);
    qBBBluetoothDebug() << "registering server";
    ppsSendControlMessage("register_server", 0x1101, uuid, QString(), this);
}

QRfcommServerPrivate *QRfcommServerPrivate::rfcomm_server_p = 0;

void QRfcommServerPrivate::controlReply(ppsResult result)
{
    Q_Q(QRfcommServer);

    if (result.msg == QStringLiteral("register_server")) {
        qBBBluetoothDebug() << "SPP: Server registration succesfull";
        ppsRegisterForEvent("service_connected",this);
        //Unregistering for event?
    } else if (result.msg == QStringLiteral("get_mount_point_path")) {
        qBBBluetoothDebug() << "SPP: Mount point for server" << result.dat.first();

        int socketFD = ::open(result.dat.first().toStdString().c_str(), O_RDWR | O_NONBLOCK);
        if (socketFD == -1) {
            qWarning() << Q_FUNC_INFO << "RFCOMM Server: Could not open socket FD";
        } else {
            QBluetoothSocket *newSocket =  new QBluetoothSocket;
            newSocket->setSocketDescriptor(socketFD, QBluetoothSocket::RfcommSocket,
                                           QBluetoothSocket::ConnectedState);//, );
            newSocket->connectToService(QBluetoothAddress(nextClientAddress), m_uuid);
            activeSockets.append(newSocket);
            emit q->newConnection();
        }
    }
}

void QRfcommServerPrivate::controlEvent(ppsResult result)
{
    if (result.msg == QStringLiteral("service_connected")) {
        qBBBluetoothDebug() << "SPP: Server: Sending request for mount point path";
        qBBBluetoothDebug() << result.dat;

        if (result.dat.size() >= 2) {
            nextClientAddress = result.dat.at(1);
            ppsSendControlMessage("get_mount_point_path", 0x1101, m_uuid, result.dat.at(1), this);
        } else {
            qWarning() << Q_FUNC_INFO << "address not specified in service connect reply";
        }
    }
}

void QRfcommServer::close()
{
    Q_D(QRfcommServer);
    if (!d->socket)
    {
        // there is no way to propagate the error to user
        // so just ignore the problem ;)
        return;
    }
    d->socket->close();
    // force active object (socket) to run and shutdown socket.
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

bool QRfcommServer::listen(const QBluetoothAddress &address, quint16 port)
{
    Q_UNUSED(port)
    Q_UNUSED(address)
    Q_D(QRfcommServer);
    // listen has already been called before
    if (d->socket)
        return true;

    d->socket = new QBluetoothSocket(QBluetoothSocket::RfcommSocket, this);

    if (!d->socket)
        return false;

    //TODO Security flags?
    return true;
}

void QRfcommServer::setMaxPendingConnections(int numConnections)
{
    Q_D(QRfcommServer);
    d->maxPendingConnections = numConnections;
}

QBluetoothAddress QRfcommServer::serverAddress() const
{
    Q_D(const QRfcommServer);
    if (d->socket)
        return d->socket->localAddress();
    else
        return QBluetoothAddress();
}

quint16 QRfcommServer::serverPort() const
{
    //Currently we do not have access to the port
    Q_D(const QRfcommServer);
    if (d->socket)
        return d->socket->localPort();
    else
        return 0;
}

bool QRfcommServer::hasPendingConnections() const
{
    Q_D(const QRfcommServer);
    return !d->activeSockets.isEmpty();
}

QBluetoothSocket *QRfcommServer::nextPendingConnection()
{
    Q_D(QRfcommServer);
    if (d->activeSockets.isEmpty())
        return 0;

    QBluetoothSocket *next = d->activeSockets.takeFirst();
    return next;
}

void QRfcommServer::setSecurityFlags(QBluetooth::SecurityFlags security)
{
    Q_D(QRfcommServer);
    d->securityFlags = security;
}

QBluetooth::SecurityFlags QRfcommServer::securityFlags() const
{
    Q_D(const QRfcommServer);
    return d->securityFlags;
}

QTBLUETOOTH_END_NAMESPACE


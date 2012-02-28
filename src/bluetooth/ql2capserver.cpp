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

#include "ql2capserver.h"
#include "ql2capserver_p.h"
#include "qbluetoothsocket.h"

QTBLUETOOTH_BEGIN_NAMESPACE

/*!
    \class QL2capServer
    \brief The QL2capServer class allows you to connect to a service using the L2CAP protocol.

    \ingroup connectivity-bluetooth
    \inmodule QtBluetooth

    QL2capServer is used to implement Bluetooth services over L2CAP.

    Start listening for incoming connections with listen(). Wait till the newConnection() signal
    is emitted when the connection is established, and call nextPendingConnection() to get a QBluetoothSocket
    for the connection.

    To enable other devices to find your service, create a QBluetoothServiceInfo with the applicable
    attributes for your service and register it using QBluetoothServiceInfo::registerService(). Call
    serverPort() to get the L2CAP port number that is being used.

    \sa QBluetoothServiceInfo, QBluetoothSocket
*/

/*!
    \fn void QL2capServer::newConnection()

    This signal is emitted when a new connection is available.

    The connected slot should call nextPendingConnection() to get a QBluetoothSocket object to send
    and receive data over the connection.

    \sa nextPendingConnection(), hasPendingConnections()
*/

/*!
    \fn void QL2capServer::close()

    Closes and resets the listening socket.
*/

/*!
    \fn bool QL2capServer::listen(const QBluetoothAddress &address, quint16 port)

    Start listening for incoming connections to \a address on \a port.

    Returns true if the operation succeeded and the L2CAP server is listening for incoming
    connections, otherwise returns false.

    \sa isListening(), newConnection()
*/

/*!
    \fn void QL2capServer::setMaxPendingConnections(int numConnections)

    Sets the maximum number of pending connections to \a numConnections.

    \sa maxPendingConnections()
*/

/*!
    \fn bool QL2capServer::hasPendingConnections() const

    Returns true if a connection is pending, otherwise false.
*/

/*!
    \fn QBluetoothSocket *QL2capServer::nextPendingConnection()

    Returns a pointer to a QBluetoothSocket for the next pending connection. It is the callers
    responsibility to delete the pointer.
*/

/*!
    \fn QBluetoothAddress QL2capServer::serverAddress() const

    Returns the server address.
*/

/*!
    \fn quint16 QL2capServer::serverPort() const

    Returns the server port number.
*/

/*!
    Constructs an L2CAP server with \a parent.
*/
QL2capServer::QL2capServer(QObject *parent)
:   QObject(parent), d_ptr(new QL2capServerPrivate)
{
    d_ptr->q_ptr = this;
}

/*!
    Destroys the L2CAP server.
*/
QL2capServer::~QL2capServer()
{
    delete d_ptr;
}

/*!
    Returns true if the L2CAP server is listening for incoming connections, otherwise false.
*/
bool QL2capServer::isListening() const
{
    Q_D(const QL2capServer);

    return d->socket->state() == QBluetoothSocket::ListeningState;
}

/*!
    Returns the maximum number of pending connections.

    \sa setMaxPendingConnections()
*/
int QL2capServer::maxPendingConnections() const
{
    Q_D(const QL2capServer);

    return d->maxPendingConnections;
}

/*!
    \fn void QL2capServer::setSecurityFlags(QBluetooth::SecurityFlags security)
    Sets the Bluetooth security flags to \a security. This function must be called before calling listen().
*/

/*!
    \fn QBluetooth::SecurityFlags QL2capServer::securityFlags() const

    Returns the Bluetooth security flags.
*/

#include "moc_ql2capserver.cpp"

QTBLUETOOTH_END_NAMESPACE

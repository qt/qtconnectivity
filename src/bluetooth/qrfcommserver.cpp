/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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

#include "qrfcommserver.h"
#include "qrfcommserver_p.h"
#include "qbluetoothsocket.h"

QTBLUETOOTH_BEGIN_NAMESPACE

/*!
    \class QRfcommServer
    \brief The QRfcommServer class provides an RFCOMM server.

    \ingroup connectivity-bluetooth
    \inmodule QtConnectivity
    \since 5.0

    QRfcommServer is used to implement Bluetooth services over RFCOMM.

    Start listening for incoming connections with listen(). The newConnection() signal is emitted
    when a new connection is established. Call nextPendingConnection() to get a QBluetoothSocket
    for the new connection.

    To enable other devices to find your service create a QBluetoothServiceInfo with the
    applicable attributes for your service and register it with
    QBluetoothServiceInfo::registerService(). Call serverPort() to get the RFCOMM channel number
    that is being used.

    \sa QBluetoothServiceInfo, QBluetoothSocket
*/

/*!
    \fn void QRfcommServer::newConnection()

    This signal is emitted when a new connection is available.

    The connected slot should call nextPendingConnection() to get a QBluetoothSocket object to
    send and receive data over the connection.

    \sa nextPendingConnection(), hasPendingConnections()
*/

/*!
    \fn void QRfcommServer::close()

    Closes and resets the listening socket.
*/

/*!
    \fn bool QRfcommServer::listen(const QBluetoothAddress &address, quint16 port)

    Start listening for incoming connections to \a address on \a port.

    Returns true if the operation succeeded and the RFCOMM server is listening for
    incoming connections; otherwise returns false.

    \sa isListening(), newConnection()
*/

/*!
    \fn void QRfcommServer::setMaxPendingConnections(int numConnections)

    Sets the maximum number of pending connections to \a numConnections.

    \sa maxPendingConnections()
*/

/*!
    \fn bool QRfcommServer::hasPendingConnections() const
    Returns true if a connection is pending; otherwise returns false.
*/

/*!
    \fn QBluetoothSocket *QRfcommServer::nextPendingConnection()

    Returns a pointer QBluetoothSocket for the next pending connection. It is the callers
    responsibility to delete pointer.
*/

/*!
    \fn QBluetoothAddress QRfcommServer::serverAddress() const

    Returns the server address.
*/

/*!
    \fn quint16 QRfcommServer::serverPort() const

    Returns the server's port number.
*/

/*!
    Constructs an RFCOMM server with \a parent.
*/
QRfcommServer::QRfcommServer(QObject *parent)
    : QObject(parent), d_ptr(new QRfcommServerPrivate)
{
    d_ptr->q_ptr = this;
}

/*!
    Destroys the RFCOMM server.
*/
QRfcommServer::~QRfcommServer()
{
    delete d_ptr;
}

/*!
    Returns true if the RFCOMM server is listening for incoming connections; otherwise returns
    false.
*/
bool QRfcommServer::isListening() const
{
    Q_D(const QRfcommServer);

    return d->socket->state() == QBluetoothSocket::ListeningState;
}

/*!
    Returns the maximum number of pending connections.

    \sa setMaxPendingConnections()
*/
int QRfcommServer::maxPendingConnections() const
{
    Q_D(const QRfcommServer);

    return d->maxPendingConnections;
}

/*!
  \fn QRfcommServer::setSecurityFlags(QBluetooth::SecurityFlags security)
    Sets the Bluetooth security flags to \a security. This function must be called prior to calling
    listen().
*/

/*!
  \fn QBluetooth::SecurityFlags QRfcommServer::securityFlags() const
    Returns the Bluetooth security flags.
*/

#include "moc_qrfcommserver.cpp"

QTBLUETOOTH_END_NAMESPACE

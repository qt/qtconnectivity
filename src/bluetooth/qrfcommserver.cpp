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

#include "qrfcommserver.h"
#include "qrfcommserver_p.h"
#include "qbluetoothsocket.h"
#include "qbluetoothserviceinfo.h"

QT_BEGIN_NAMESPACE_BLUETOOTH

/*!
    \class QRfcommServer
    \inmodule QtBluetooth
    \brief The QRfcommServer class uses the RFCOMM protocol to communicate with
    a Bluetooth device.

    QRfcommServer is used to implement Bluetooth services over RFCOMM.

    Start listening for incoming connections with listen(). Wait till the newConnection() signal
    is emitted when a new connection is established, and call nextPendingConnection() to get a QBluetoothSocket
    for the new connection.

    To enable other devices to find your service, create a QBluetoothServiceInfo with the
    applicable attributes for your service and register it using QBluetoothServiceInfo::registerService().
    Call serverPort() to get the RFCOMM channel number that is being used.

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
    incoming connections, otherwise returns false.

    \sa isListening(), newConnection()
*/

/*!
    \fn void QRfcommServer::setMaxPendingConnections(int numConnections)

    Sets the maximum number of pending connections to \a numConnections.

    \sa maxPendingConnections()
*/

/*!
    \fn bool QRfcommServer::hasPendingConnections() const
    Returns true if a connection is pending, otherwise false.
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

    Returns the server port number.
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
    \fn QBluetoothServiceInfo QRfcommServer::listen(const QBluetoothUuid &uuid, const QString &serviceName)

    Convenience function for registering an SPP service with \a uuid and \a serviceName.
    Because this function already registers the service, the QBluetoothServiceInfo object
    which is returned can not be changed any more.

    Returns a registered QBluetoothServiceInfo instance if sucessful otherwise an
    invalid QBluetoothServiceInfo.

    This function is equivalent to following code snippet.

    \snippet qrfcommserver.cpp listen

    \sa isListening(), newConnection(), listen()
*/
QBluetoothServiceInfo QRfcommServer::listen(const QBluetoothUuid &uuid, const QString &serviceName)
{
    if (!listen())
        return QBluetoothServiceInfo();
//! [listen]
    QBluetoothServiceInfo serviceInfo;
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, serviceName);
    serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList,
                             QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));

    QBluetoothServiceInfo::Sequence classId;
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);
    serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList,
                             classId);

    serviceInfo.setServiceUuid(uuid);

    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    protocol.clear();
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
             << QVariant::fromValue(quint8(serverPort()));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                             protocolDescriptorList);
    bool result = serviceInfo.registerService();
//! [listen]
    if (!result)
        return QBluetoothServiceInfo();
    return serviceInfo;
}

/*!
    Returns true if the RFCOMM server is listening for incoming connections, otherwise false.
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
    Sets the Bluetooth security flags to \a security. This function must be called before calling listen().
*/

/*!
    \fn QBluetooth::SecurityFlags QRfcommServer::securityFlags() const
    Returns the Bluetooth security flags.
*/

#include "moc_qrfcommserver.cpp"

QT_END_NAMESPACE_BLUETOOTH

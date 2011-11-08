/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarativebluetoothsocket_p.h"

#include <QPointer>
#include <QStringList>
#include <QDataStream>
#include <QByteArray>


#include <qbluetoothdeviceinfo.h>
#include <qbluetoothaddress.h>
#include <qbluetoothsocket.h>
#include <ql2capserver.h>
#include <qrfcommserver.h>

/* ==================== QDeclarativeBluetoothSocket ======================= */

/*!
   \qmlclass BluetoothSocket QDeclarativeBluetoothSocket
   \brief The BluetoothSocket element represents a single bluetooth client socket.

   \ingroup connectivity-qml
   \inmodule QtConnectivity

   \sa QBluetoothSocket
   \sa QDataStream

    The BluetoothSocket element is part of the \bold{QtConnectivity} module.

    It allows a QML class easily connect to another bluetooth device and send
    and received QString's from the device. Data is sent and received via a
    QDataStream allowing type safe transfer of string data.  QDataStream is a well known
    format and maybe decoded by non-Qt applications.

    Connections to remove devices can be over rfcomm or l2cap.  Either the remote port
    or service UUID is required.  This is specified by creating a BluetoothService,
    or passing in the service return from BluetoothDiscoveryModel.
 */

class QDeclarativeBluetoothSocketPrivate
{
public:
    QDeclarativeBluetoothSocketPrivate(QDeclarativeBluetoothSocket *bs)
        : m_dbs(bs), m_service(0), m_socket(0),
          m_error(QLatin1String("No Error")),
          m_state(QLatin1String("No Service Set")),
          m_componentCompleted(false),
          m_connected(false)
    {

    }

    ~QDeclarativeBluetoothSocketPrivate()
    {
        delete m_socket;
    }

    void connect()
    {
        Q_ASSERT(m_service);
        qDebug() << "Connecting to: " << m_service->serviceInfo()->device().address().toString();
        m_error = QLatin1String("No Error");

        if (m_socket)
            m_socket->deleteLater();

//        delete m_socket;
        m_socket = new QBluetoothSocket();
        m_socket->connectToService(*m_service->serviceInfo());
        QObject::connect(m_socket, SIGNAL(connected()), m_dbs, SLOT(socket_connected()));
        QObject::connect(m_socket, SIGNAL(disconnected()), m_dbs, SLOT(socket_disconnected()));
        QObject::connect(m_socket, SIGNAL(error(QBluetoothSocket::SocketError)), m_dbs, SLOT(socket_error(QBluetoothSocket::SocketError)));
        QObject::connect(m_socket, SIGNAL(stateChanged(QBluetoothSocket::SocketState)), m_dbs, SLOT(socket_state(QBluetoothSocket::SocketState)));
        QObject::connect(m_socket, SIGNAL(readyRead()), m_dbs, SLOT(socket_readyRead()));

        m_stream = new QDataStream(m_socket);
    }

    QDeclarativeBluetoothSocket *m_dbs;
    QDeclarativeBluetoothService *m_service;
    QBluetoothSocket *m_socket;
    QString m_error;
    QString m_state;
    bool m_componentCompleted;
    bool m_connected;
    QDataStream *m_stream;

};

QDeclarativeBluetoothSocket::QDeclarativeBluetoothSocket(QObject *parent) :
    QObject(parent)
{
    d = new QDeclarativeBluetoothSocketPrivate(this);
}

QDeclarativeBluetoothSocket::QDeclarativeBluetoothSocket(QDeclarativeBluetoothService *service, QObject *parent)
    : QObject(parent)
{
    d = new QDeclarativeBluetoothSocketPrivate(this);
    d->m_service = service;
}

QDeclarativeBluetoothSocket::QDeclarativeBluetoothSocket(QBluetoothSocket *socket, QDeclarativeBluetoothService *service, QObject *parent)
    : QObject(parent)
{
    d = new QDeclarativeBluetoothSocketPrivate(this);
    d->m_service = service;
    d->m_socket = socket;
    d->m_connected = true;
    d->m_componentCompleted = true;

    QObject::connect(socket, SIGNAL(connected()), this, SLOT(socket_connected()));
    QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(socket_disconnected()));
    QObject::connect(socket, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(socket_error(QBluetoothSocket::SocketError)));
    QObject::connect(socket, SIGNAL(stateChanged(QBluetoothSocket::SocketState)), this, SLOT(socket_state(QBluetoothSocket::SocketState)));
    QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(socket_readyRead()));

    d->m_stream = new QDataStream(socket);

}


QDeclarativeBluetoothSocket::~QDeclarativeBluetoothSocket()
{
    delete d;
}

void QDeclarativeBluetoothSocket::componentComplete()
{
    d->m_componentCompleted = true;

    if (d->m_connected && d->m_service)
        d->connect();
}

/*!
  \qmlproperty BluetoothService BluetoothSocket::service

  This property holds the details of the remote service to connect too. It can be
  set to a static BluetoothService with a fixed description, or a service returned
  by service discovery.
  */


QDeclarativeBluetoothService *QDeclarativeBluetoothSocket::service()
{
    return d->m_service;
}

void QDeclarativeBluetoothSocket::setService(QDeclarativeBluetoothService *service)
{
    d->m_service = service;

    if (!d->m_componentCompleted)
        return;

    if (d->m_connected)
        d->connect();
    emit serviceChanged();
}

/*!
  \qmlproperty bool BluetoothSocket::connected

  This property holds the connection state of the socket. If the socket is
  connected to peer it returns true. It can be set true of false to control the
  connection. When set true the property will not return true until the
  connection is established.

  */


bool QDeclarativeBluetoothSocket::connected()
{
    if (!d->m_socket)
        return false;

    return d->m_socket->state() == QBluetoothSocket::ConnectedState;
}

void QDeclarativeBluetoothSocket::setConnected(bool connected)
{
    d->m_connected = connected;
    if (connected && d->m_componentCompleted) {
        if (d->m_service) {
            d->connect();
        }
        else {
            qWarning() << "BluetoothSocket::setConnected called before a service was set";
        }
    }

    if (!connected && d->m_socket){
        d->m_socket->close();
    }
}

/*!
  \qmlproperty string BluetoothSocket::error

  This property holds the string for the last reported error
  This property is read only.
  */


QString QDeclarativeBluetoothSocket::error()
{
    return d->m_error;
}

void QDeclarativeBluetoothSocket::socket_connected()
{
    emit connectedChanged();
}

void QDeclarativeBluetoothSocket::socket_disconnected()
{
    d->m_socket->deleteLater();
    d->m_socket = 0;
    emit connectedChanged();
}

void QDeclarativeBluetoothSocket::socket_error(QBluetoothSocket::SocketError err)
{
    if (err == QBluetoothSocket::ConnectionRefusedError)
        d->m_error = QLatin1String("Connection Refused");
    else if (err == QBluetoothSocket::RemoteHostClosedError)
        d->m_error = QLatin1String("Connection Closed by Remote Host");
    else if (err == QBluetoothSocket::HostNotFoundError)
        d->m_error = QLatin1String("Host Not Found");
    else if (err == QBluetoothSocket::ServiceNotFoundError)
        d->m_error = QLatin1String("Could not find service at remote host");
    else
        d->m_error = QLatin1String("Unknown Error");

    emit errorChanged();
}

void QDeclarativeBluetoothSocket::socket_state(QBluetoothSocket::SocketState state)
{
    switch (state) {
    case QBluetoothSocket::UnconnectedState:
        d->m_state = QLatin1String("Unconnected");
        break;
    case QBluetoothSocket::ServiceLookupState:
        d->m_state = QLatin1String("Service Lookup");
        break;
    case QBluetoothSocket::ConnectingState:
        d->m_state = QLatin1String("Connecting");
        break;
    case QBluetoothSocket::ConnectedState:
        d->m_state = QLatin1String("Connected");
        break;
    case QBluetoothSocket::ClosingState:
        d->m_state = QLatin1String("Closing");
        break;
    case QBluetoothSocket::ListeningState:
        d->m_state = QLatin1String("Listening");
        break;
    case QBluetoothSocket::BoundState:
        d->m_state = QLatin1String("Bound");
        break;
    }

    emit stateChanged();
}

QString QDeclarativeBluetoothSocket::state()
{
    return d->m_state;
}

void QDeclarativeBluetoothSocket::socket_readyRead()
{
    emit dataAvailable();
}

/*!
  \qmlproperty string BluetoothSocket::stringData

  This property receives or sends data to remote bluetooth device. Arrival of
  data is signaled through the dataAvailable signal and can be read by
  stringData. Calling sendStringData will transmit the string to the far side.
  If excessive amounts of data are sent the function may block. Reading will
  never block.
  */


QString QDeclarativeBluetoothSocket::stringData()
{
    if (!d->m_socket|| !d->m_socket->bytesAvailable())
        return QString();

    QString data;
    *d->m_stream >> data;
    return data;
}

void QDeclarativeBluetoothSocket::sendStringData(QString data)
{
    if (!d->m_connected || !d->m_socket){
        qWarning() << "Writing data to unconnected socket";
        return;
    }

    QByteArray b;
    QDataStream s(&b, QIODevice::WriteOnly);
    s << data;
    d->m_socket->write(b);
}

void QDeclarativeBluetoothSocket::newSocket(QBluetoothSocket *socket, QDeclarativeBluetoothService *service)
{
    if (d->m_socket){
        delete d->m_socket;
    }

    d->m_service = service;
    d->m_socket = socket;
    d->m_connected = true;
    d->m_componentCompleted = true;
    d->m_error = QLatin1String("No Error");

    QObject::connect(socket, SIGNAL(connected()), this, SLOT(socket_connected()));
    QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(socket_disconnected()));
    QObject::connect(socket, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(socket_error(QBluetoothSocket::SocketError)));
    QObject::connect(socket, SIGNAL(stateChanged(QBluetoothSocket::SocketState)), this, SLOT(socket_state(QBluetoothSocket::SocketState)));
    QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(socket_readyRead()));

    d->m_stream = new QDataStream(socket);

    socket_state(socket->state());

    emit connectedChanged();
}

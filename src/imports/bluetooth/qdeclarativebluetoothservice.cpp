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

#include "qdeclarativebluetoothservice_p.h"

#include <qbluetoothdeviceinfo.h>
#include <qbluetoothsocket.h>
#include <qbluetoothaddress.h>
#include <ql2capserver.h>
#include <qrfcommserver.h>

#include <QObject>

/* ==================== QDeclarativeBluetoothService ======================= */

/*!
   \qmlclass BluetoothService QDeclarativeBluetoothService
   \brief Provides information about a particular Bluetooth service.

   \ingroup bluetooth-qml
   \inmodule QtBluetooth

   \sa QBluetoothAddress
   \sa QBluetoothSocket

    The BluetoothService element was introduced in \bold{QtBluetooth 5.0}.

    It allows a QML project to get information about a remote service, or describe a service
    for a BluetoothSocket to connect to.
 */


class QDeclarativeBluetoothServicePrivate
{
public:
    QDeclarativeBluetoothServicePrivate()
        : m_componentComplete(false),
          m_service(0), m_port(0),
          m_needsRegistration(false),
          m_listen(0)
    {

    }

    ~QDeclarativeBluetoothServicePrivate()
    {
        delete m_service;
    }

    int listen();

    bool m_componentComplete;
    QBluetoothServiceInfo *m_service;
    QString m_protocol;
    qint32 m_port;
    QString m_description;
    QString m_name;
    QString m_uuid;
    bool m_needsRegistration;
    QObject *m_listen;

};

QDeclarativeBluetoothService::QDeclarativeBluetoothService(QObject *parent) :
    QObject(parent)
{
    d = new QDeclarativeBluetoothServicePrivate;
}

QDeclarativeBluetoothService::QDeclarativeBluetoothService(const QBluetoothServiceInfo &service, QObject *parent)
    : QObject(parent)
{
    d = new QDeclarativeBluetoothServicePrivate;
    d->m_service = new QBluetoothServiceInfo(service);
}

QDeclarativeBluetoothService::~QDeclarativeBluetoothService()
{
    delete d;
}

void QDeclarativeBluetoothService::componentComplete()
{
    d->m_componentComplete = true;

    if (d->m_needsRegistration)
        setRegistered(true);
}


/*!
  \qmlproperty string BluetoothService::deviceName

  This property holds the name of the remote device.
  */

QString QDeclarativeBluetoothService::deviceName() const
{
    if (!d->m_service)
        return QString();

    return d->m_service->device().name();
}

/*!
  \qmlproperty string BluetoothService::deviceAddress

  This property holds the remote device MAc address. Must be valid if you to
  connect to a remote device with a BluetoothSocket.
  */

QString QDeclarativeBluetoothService::deviceAddress() const
{
    if (!d->m_service)
        return QString();

    return d->m_service->device().address().toString();
}

void QDeclarativeBluetoothService::setDeviceAddress(QString address)
{
    if (!d->m_service)
        d->m_service = new QBluetoothServiceInfo();

    QBluetoothAddress a(address);
    QBluetoothDeviceInfo device(a, QString(), QBluetoothDeviceInfo::ComputerDevice);
    d->m_service->setDevice(device);
}

/*!
  \qmlproperty string BluetoothService::serviceName

  This property holds the name of the remote service if available.
  */

QString QDeclarativeBluetoothService::serviceName() const
{
    if (!d->m_service)
        return QString();

    if (!d->m_name.isEmpty())
        return d->m_name;

    return d->m_service->serviceName();
}

void QDeclarativeBluetoothService::setServiceName(QString name)
{
    d->m_name = name;
}


/*!
  \qmlproperty string BluetoothService::serviceDescription

  This property holds the description provided by the remote service.
  */
QString QDeclarativeBluetoothService::serviceDescription() const
{
    if (!d->m_service)
        return QString();

    if (!d->m_description.isEmpty())
        return d->m_name;

    return d->m_service->serviceDescription();
}

void QDeclarativeBluetoothService::setServiceDescription(QString description)
{
    d->m_description = description;
    emit detailsChanged();
}

/*!
  \qmlproperty string BluetoothService::serviceProtocol

  This property holds the protocol used for the service. Can be the string
  "l2cap" or "rfcomm"
  */

QString QDeclarativeBluetoothService::serviceProtocol() const
{
    if (!d->m_protocol.isEmpty())
        return d->m_protocol;

    if (!d->m_service)
        return QString();

    if (d->m_service->socketProtocol() == QBluetoothServiceInfo::L2capProtocol)
        return QLatin1String("l2cap");
    if (d->m_service->socketProtocol() == QBluetoothServiceInfo::RfcommProtocol)
        return QLatin1String("rfcomm");

    return QLatin1String("unknown");
}

void QDeclarativeBluetoothService::setServiceProtocol(QString protocol)
{
    if (protocol != "rfcomm" && protocol != "l2cap")
        qWarning() << "Invalid protocol" << protocol;

    d->m_protocol = protocol;
    emit detailsChanged();
}

/*!
  \qmlproperty string BluetoothService::serviceUuid

  This property holds the UUID of the remote service. Service UUID or port, as
  well as the address must be set to connect to a remote service. If UUID and
  port are set, the port is used. The UUID takes longer to connect since
  service discovery must be initiated to discover the port value.
  */

QString QDeclarativeBluetoothService::serviceUuid() const
{
    if (!d->m_uuid.isEmpty())
        return d->m_uuid;

    if (!d->m_service)
        return QString();

    return d->m_service->attribute(QBluetoothServiceInfo::ServiceId).toString();
}

void QDeclarativeBluetoothService::setServiceUuid(QString uuid)
{
    d->m_uuid = uuid;
    if (!d->m_service)
        d->m_service = new QBluetoothServiceInfo();
    d->m_service->setAttribute(QBluetoothServiceInfo::ServiceId, QBluetoothUuid(uuid));

    emit detailsChanged();
}

/*!
  \qmlproperty int BluetoothService::servicePort

  This property holds the port value for the remote service. Bluetooth does not
  use well defined port values, so port values should not be stored and used
  later without care. Connecting via UUID is much more consistent.
  */
qint32 QDeclarativeBluetoothService::servicePort() const
{
    if (d->m_port > 0)
        return d->m_port;

    if (!d->m_service)
        return -1;

    if (d->m_service->serverChannel() > 0)
        return d->m_service->serverChannel();
    if (d->m_service->protocolServiceMultiplexer() > 0)
        return d->m_service->protocolServiceMultiplexer();

    return -1;
}

void QDeclarativeBluetoothService::setServicePort(qint32 port)
{
    if (d->m_port != port){
        d->m_port = port;
        if (isRegistered())
            setRegistered(true);
        emit detailsChanged();
    }
}

/*!
  \qmlproperty string BluetoothService::registered

  This property holds the registration/publication status of the service.  If true the service
  is published via service discovery.  Not implemented in 1.2.
  */

bool QDeclarativeBluetoothService::isRegistered() const
{
    if (!d->m_service)
        return false;

    return d->m_service->isRegistered();
}


int QDeclarativeBluetoothServicePrivate::listen() {

    if (m_protocol == "l2cap") {
        QL2capServer *server = new QL2capServer();

        server->setMaxPendingConnections(1);
        server->listen(QBluetoothAddress(), m_port);
        m_port = server->serverPort();
        m_listen = server;
    }
    else if (m_protocol == "rfcomm") {
        QRfcommServer *server = new QRfcommServer();

        server->setMaxPendingConnections(1);
        server->listen(QBluetoothAddress(), m_port);
        m_port = server->serverPort();
        m_listen = server;
    }
    else {
        qDebug() << "Unknown protocol, can't make service" << m_protocol;
    }

    return m_port;

}

void QDeclarativeBluetoothService::setRegistered(bool registered)
{

    d->m_needsRegistration = registered;

    if (!d->m_componentComplete) {
        return;
    }

    if (!registered) {
        if (!d->m_service)
            return;
        d->m_service->unregisterService();
        emit registeredChanged();
    }

    if (!d->m_service) {
        d->m_service = new QBluetoothServiceInfo();
    }


    delete d->m_listen;
    d->m_listen = 0;

    d->listen();
    connect(d->m_listen, SIGNAL(newConnection()), this, SLOT(new_connection()));


    d->m_service->setAttribute(QBluetoothServiceInfo::ServiceRecordHandle, (uint)0x00010010);

//    QBluetoothServiceInfo::Sequence classId;
////    classId << QVariant::fromVhttp://theunderstatement.com/alue(QBluetoothUuid(serviceUuid));
//    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
//    d->m_service->setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);

    d->m_service->setAttribute(QBluetoothServiceInfo::ServiceName, d->m_name);
    d->m_service->setAttribute(QBluetoothServiceInfo::ServiceDescription,
                             d->m_description);

    d->m_service->setServiceUuid(QBluetoothUuid(d->m_uuid));

    qDebug() << "name/uuid" << d->m_name << d->m_uuid << d->m_port;

    d->m_service->setAttribute(QBluetoothServiceInfo::BrowseGroupList,
                             QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));

    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;

    qDebug() << "Port" << d->m_port;

    if (d->m_protocol == "l2cap") {
        protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap))
                 << QVariant::fromValue(quint16(d->m_port));
        protocolDescriptorList.append(QVariant::fromValue(protocol));
    }
    else if (d->m_protocol == "rfcomm") {
        protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
                 << QVariant::fromValue(quint8(d->m_port));
        protocolDescriptorList.append(QVariant::fromValue(protocol));
    }
    else {
        qWarning() << "No protocol specified for bluetooth service";
    }
    d->m_service->setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                             protocolDescriptorList);

    if (d->m_service->registerService()) {
        qDebug() << "registered";
        emit registeredChanged();
    }
    else {
        qDebug() << "Failed";
    }
}

QBluetoothServiceInfo *QDeclarativeBluetoothService::serviceInfo() const
{
    return d->m_service;
}

void QDeclarativeBluetoothService::new_connection()
{
    emit newClient();
}

QDeclarativeBluetoothSocket *QDeclarativeBluetoothService::nextClient()
{
    QL2capServer *server = qobject_cast<QL2capServer *>(d->m_listen);
    if (server) {
        if (server->hasPendingConnections()) {
            QBluetoothSocket *socket = server->nextPendingConnection();
            return new QDeclarativeBluetoothSocket(socket, this, 0x0);
        }
        else {
            qDebug() << "Socket has no pending connection, failing";
            return 0x0;
        }
    }
    QRfcommServer *rserver = qobject_cast<QRfcommServer *>(d->m_listen);
    if (rserver) {
        if (rserver->hasPendingConnections()) {
            QBluetoothSocket *socket = rserver->nextPendingConnection();
            return new QDeclarativeBluetoothSocket(socket, this, 0x0);;
        }
        else {
            qDebug() << "Socket has no pending connection, failing";
            return 0x0;
        }
    }
    return 0x0;
}

void QDeclarativeBluetoothService::assignNextClient(QDeclarativeBluetoothSocket *dbs)
{
    QL2capServer *server = qobject_cast<QL2capServer *>(d->m_listen);
    if (server) {
        if (server->hasPendingConnections()) {
            QBluetoothSocket *socket = server->nextPendingConnection();
            dbs->newSocket(socket, this);
            return;
        }
        else {
            qDebug() << "Socket has no pending connection, failing";
            return;
        }
    }
    QRfcommServer *rserver = qobject_cast<QRfcommServer *>(d->m_listen);
    if (rserver) {
        if (rserver->hasPendingConnections()) {
            QBluetoothSocket *socket = rserver->nextPendingConnection();
            dbs->newSocket(socket, this);
            return;
        }
        else {
            qDebug() << "Socket has no pending connection, failing";
            return;
        }
    }
    return;
}


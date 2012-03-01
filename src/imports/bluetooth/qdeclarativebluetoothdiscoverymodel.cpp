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

#include "qdeclarativebluetoothdiscoverymodel_p.h"

#include <QPixmap>

#include <qbluetoothdeviceinfo.h>
#include <qbluetoothaddress.h>

#include "qdeclarativebluetoothservice_p.h"

/*!
    \qmlclass BluetoothDiscoveryModel QDeclarativeBluetoothDiscoveryModel
    \brief Enables you to search for the Bluetooth devices and services in range.

    \ingroup bluetooth-qml
    \inmodule QtBluetooth

    The BluetoothDiscoveryModel element was introduced in \b{QtBluetooth 5.0}.

    BluetoothDiscoveryModel provides a model of connectable services. The
    contents of the model can be filtered by UUID allowing discovery to be
    limited to a single service such as a game.

    The model roles provided by BluetoothDiscoveryModel are display, decoration and \c Service.
    Through the \c Service role the BluetoothService maybe accessed for more details.

    \sa QBluetoothServiceDiscoveryAgent

*/

class QDeclarativeBluetoothDiscoveryModelPrivate
{
public:
    QDeclarativeBluetoothDiscoveryModelPrivate()
        :m_agent(0),
        m_error(QBluetoothServiceDiscoveryAgent::NoError),
        m_minimal(true),
        m_working(false),
        m_componentCompleted(false),
        m_discovery(true)
    {
    }
    ~QDeclarativeBluetoothDiscoveryModelPrivate()
    {
        if (m_agent)
            delete m_agent;
    }

    QBluetoothServiceDiscoveryAgent *m_agent;

    QBluetoothServiceDiscoveryAgent::Error m_error;
//    QList<QBluetoothServiceInfo> m_services;
    QList<QDeclarativeBluetoothService *> m_services;
    bool m_minimal;
    bool m_working;
    bool m_componentCompleted;
    QString m_uuid;
    bool m_discovery;
};

QDeclarativeBluetoothDiscoveryModel::QDeclarativeBluetoothDiscoveryModel(QObject *parent) :
    QAbstractListModel(parent),
    d(new QDeclarativeBluetoothDiscoveryModelPrivate)
{

    QHash<int, QByteArray> roleNames;
    roleNames = QAbstractItemModel::roleNames();
    roleNames.insert(Qt::DisplayRole, "name");
    roleNames.insert(Qt::DecorationRole, "icon");
    roleNames.insert(ServiceRole, "service");
    setRoleNames(roleNames);

    d->m_agent = new QBluetoothServiceDiscoveryAgent(this);
    connect(d->m_agent, SIGNAL(serviceDiscovered(const QBluetoothServiceInfo&)), this, SLOT(serviceDiscovered(const QBluetoothServiceInfo&)));
    connect(d->m_agent, SIGNAL(finished()), this, SLOT(finishedDiscovery()));
    connect(d->m_agent, SIGNAL(canceled()), this, SLOT(finishedDiscovery()));
    connect(d->m_agent, SIGNAL(error(QBluetoothServiceDiscoveryAgent::Error)), this, SLOT(errorDiscovery(QBluetoothServiceDiscoveryAgent::Error)));

}
void QDeclarativeBluetoothDiscoveryModel::componentComplete()
{
    d->m_componentCompleted = true;
    setDiscovery(d->m_discovery);
}

/*!
  \qmlproperty bool BluetoothSocket::discovery

  This property starts or stops discovery.

  */

void QDeclarativeBluetoothDiscoveryModel::setDiscovery(bool discovery_)
{
    d->m_discovery = discovery_;

    if (!d->m_componentCompleted)
        return;

    d->m_working = false;

    d->m_agent->stop();

    if (!discovery_) {
        emit discoveryChanged();
        return;
    }

    if (!d->m_uuid.isEmpty())
        d->m_agent->setUuidFilter(QBluetoothUuid(d->m_uuid));

    d->m_working = true;

    if (d->m_minimal) {
        qDebug() << "Doing minimal";
        d->m_agent->start(QBluetoothServiceDiscoveryAgent::MinimalDiscovery);
    }
    else
        d->m_agent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);

    emit discoveryChanged();
}

void QDeclarativeBluetoothDiscoveryModel::errorDiscovery(QBluetoothServiceDiscoveryAgent::Error error)
{
    d->m_error = error;
    emit errorChanged();
}

/*!
  \qmlproperty string BluetoothDiscoveryModel::error

  This property holds the last error reported by discovery.

  This property is read only.
  */
QString QDeclarativeBluetoothDiscoveryModel::error() const
{
    switch (d->m_error){
    case QBluetoothServiceDiscoveryAgent::NoError:
        break;
    default:
        return QLatin1String("UnknownError");
    }
    return QLatin1String("NoError");

}

int QDeclarativeBluetoothDiscoveryModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->m_services.count();
}

QVariant QDeclarativeBluetoothDiscoveryModel::data(const QModelIndex &index, int role) const
{
    QDeclarativeBluetoothService *service = d->m_services.value(index.row());
    QBluetoothServiceInfo *info = service->serviceInfo();

    switch (role) {
        case Qt::DisplayRole:
        {
            QString label = info->device().name();
            if (label.isEmpty())
                label += info->device().address().toString();
            label += " " + info->serviceName();
            return label;
        }
        case Qt::DecorationRole:
            return QLatin1String("image://bluetoothicons/default");
        case ServiceRole:
        {
            return QVariant::fromValue(service);
        }
    }
    return QVariant();
}

/*!
  \qmlsignal BluetoothDiscoveryModel::newServiceDiscovered()

  This handler is called when a new service is discovered.
  */

void QDeclarativeBluetoothDiscoveryModel::serviceDiscovered(const QBluetoothServiceInfo &service)
{
    QDeclarativeBluetoothService *bs = new QDeclarativeBluetoothService(service, this);

    for (int i = 0; i < d->m_services.count(); i++) {
        if (bs->deviceAddress() == d->m_services.at(i)->deviceAddress()) {
            delete bs;
            return;
        }
    }

    beginResetModel(); // beginInsertRows(...) doesn't work for full discovery...
    d->m_services.append(bs);
    endResetModel();
    emit newServiceDiscovered(bs);
}

/*!
    \qmlsignal BluetoothDiscoveryModel::discoveryChanged()

    This handler is called when discovery has completed and no
    further results will be generated.
*/

void QDeclarativeBluetoothDiscoveryModel::finishedDiscovery()
{
    qDebug() << "Done!";
    d->m_working = false;
    emit discoveryChanged();
}

/*!
  \qmlproperty bool BluetoothDiscoveryModel::minimalDiscovery

  This property controls minimalDiscovery, which is faster than full discocvery but it
  only guarantees the device and UUID information to be correct.

  */

bool QDeclarativeBluetoothDiscoveryModel::minimalDiscovery()
{
    return d->m_minimal;
}

void QDeclarativeBluetoothDiscoveryModel::setMinimalDiscovery(bool minimalDiscovery_)
{
    d->m_minimal = minimalDiscovery_;
    emit minimalDiscoveryChanged();
}

bool QDeclarativeBluetoothDiscoveryModel::discovery()
{
    return d->m_working;
}

/*!
  \qmlproperty string BluetoothDiscoveryModel::uuidFilter

  This property holds an optional UUID filter.  A UUID can be used to return only
  matching services.  16 bit, 32 bit or 128 bit UUIDs maybe used.  The string format
  is the same format as QUuid.

  \sa QBluetoothUuid
  \sa QUuid
  */


QString QDeclarativeBluetoothDiscoveryModel::uuidFilter() const
{
    return d->m_uuid;
}

void QDeclarativeBluetoothDiscoveryModel::setUuidFilter(QString uuid)
{
    QBluetoothUuid qbuuid(uuid);
    if (qbuuid.isNull()) {
        qWarning() << "Invalid UUID providded " << uuid;
        return;
    }
    d->m_uuid = uuid;
    emit uuidFilterChanged();
}

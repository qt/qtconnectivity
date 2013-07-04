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

#include "qdeclarativebluetoothdiscoverymodel_p.h"

#include <QPixmap>

#include <qbluetoothdeviceinfo.h>
#include <QtBluetooth/QBluetoothAddress>

#include "qdeclarativebluetoothservice_p.h"

/*!
    \qmltype BluetoothDiscoveryModel
    \instantiates QDeclarativeBluetoothDiscoveryModel
    \inqmlmodule QtBluetooth 5.0
    \brief Enables you to search for the Bluetooth devices and services in
    range.

    The BluetoothDiscoveryModel type was introduced in \b{QtBluetooth 5.0}.

    BluetoothDiscoveryModel provides a model of connectable services. The
    contents of the model can be filtered by UUID allowing discovery to be
    limited to a single service such as a game.

    The model roles provided by BluetoothDiscoveryModel are display, decoration and \c Service.
    Through the \c Service role the BluetoothService can be accessed for more details.

    \sa QBluetoothServiceDiscoveryAgent

*/

class QDeclarativeBluetoothDiscoveryModelPrivate
{
public:
    QDeclarativeBluetoothDiscoveryModelPrivate()
        :m_agent(0),
        m_error(QBluetoothServiceDiscoveryAgent::NoError),
        m_minimal(true),
        m_componentCompleted(false),
        m_discovery(false),
        m_modelDataNeedsReset(false)
    {
    }
    ~QDeclarativeBluetoothDiscoveryModelPrivate()
    {
        if (m_agent)
            delete m_agent;

        qDeleteAll(m_services);
    }

    QBluetoothServiceDiscoveryAgent *m_agent;

    QBluetoothServiceDiscoveryAgent::Error m_error;
//    QList<QBluetoothServiceInfo> m_services;
    QList<QDeclarativeBluetoothService *> m_services;
    bool m_minimal;
    bool m_componentCompleted;
    QString m_uuid;
    bool m_discovery;
    bool m_modelDataNeedsReset;
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

QDeclarativeBluetoothDiscoveryModel::~QDeclarativeBluetoothDiscoveryModel()
{
    delete d;
}
void QDeclarativeBluetoothDiscoveryModel::componentComplete()
{
    d->m_componentCompleted = true;
    setDiscovery(true);
}

/*!
  \qmlproperty bool BluetoothDiscoveryModel::discovery

  This property starts or stops discovery. A restart of the discovery process
  requires setting this property to \c false and subsequemtly to \c true again.

*/
void QDeclarativeBluetoothDiscoveryModel::setDiscovery(bool discovery_)
{
    if (!d->m_componentCompleted)
        return;

    if (d->m_discovery == discovery_)
        return;

    d->m_discovery = discovery_;

    if (!discovery_) {
        d->m_agent->stop();
    } else {
        if (!d->m_uuid.isEmpty())
            d->m_agent->setUuidFilter(QBluetoothUuid(d->m_uuid));

        if (d->m_minimal)
            d->m_agent->start(QBluetoothServiceDiscoveryAgent::MinimalDiscovery);
        else
            d->m_agent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);

        d->m_modelDataNeedsReset = true;
    }

    emit discoveryChanged();
}

void QDeclarativeBluetoothDiscoveryModel::errorDiscovery(QBluetoothServiceDiscoveryAgent::Error error)
{
    d->m_error = error;
    emit errorChanged();
}

void QDeclarativeBluetoothDiscoveryModel::clearModelIfRequired()
{
    if (d->m_modelDataNeedsReset) {
        d->m_modelDataNeedsReset = false;

        beginResetModel();
        qDeleteAll(d->m_services);
        d->m_services.clear();
        endResetModel();
    }
}

/*!
  \qmlproperty string BluetoothDiscoveryModel::error

  This property holds the last error reported during discovery.

  This property is read-only.
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
    if (!index.isValid())
        return QVariant();

    QDeclarativeBluetoothService *service = d->m_services.value(index.row());

    switch (role) {
        case Qt::DisplayRole:
        {
            QString label = service->deviceName();
            if (label.isEmpty())
                label += service->deviceAddress();
            else
                label+= QStringLiteral(":");
            label += QStringLiteral(" ") + service->serviceName();
            return label;
        }
        case Qt::DecorationRole:
            return QLatin1String("image://bluetoothicons/default");
        case ServiceRole:
            return QVariant::fromValue(service);
    }
    return QVariant();
}

/*!
  \qmlsignal BluetoothDiscoveryModel::newServiceDiscovered()

  This handler is called when a new service is discovered.
  */

void QDeclarativeBluetoothDiscoveryModel::serviceDiscovered(const QBluetoothServiceInfo &service)
{
    clearModelIfRequired();

    QDeclarativeBluetoothService *bs = new QDeclarativeBluetoothService(service, this);
    QDeclarativeBluetoothService *current = 0;
    for (int i = 0; i < d->m_services.count(); i++) {
        current = d->m_services.at(i);
        if (bs->deviceAddress() == current->deviceAddress()
                && bs->serviceName() == current->serviceName()) {
            delete bs;
            return;
        }
    }

    beginInsertRows(QModelIndex(),d->m_services.count(), d->m_services.count());
    d->m_services.append(bs);
    endInsertRows();
    emit newServiceDiscovered(bs);
}

/*!
    \qmlsignal BluetoothDiscoveryModel::discoveryChanged()

    This handler is called when discovery has completed and no
    further results will be generated.
*/

void QDeclarativeBluetoothDiscoveryModel::finishedDiscovery()
{
    clearModelIfRequired();
    if (d->m_discovery) {
        d->m_discovery = false;
        emit discoveryChanged();
    }
}

/*!
  \qmlproperty bool BluetoothDiscoveryModel::minimalDiscovery

  This property controls minimalDiscovery, which is faster than full discocvery but it
  only guarantees the device and UUID information to be correct.

  */

bool QDeclarativeBluetoothDiscoveryModel::minimalDiscovery() const
{
    return d->m_minimal;
}

void QDeclarativeBluetoothDiscoveryModel::setMinimalDiscovery(bool minimalDiscovery_)
{
    d->m_minimal = minimalDiscovery_;
    emit minimalDiscoveryChanged();
}

bool QDeclarativeBluetoothDiscoveryModel::discovery() const
{
    return d->m_discovery;
}

/*!
  \qmlproperty string BluetoothDiscoveryModel::uuidFilter

  This property holds an optional UUID filter.  A UUID can be used to return only
  matching services.  16 bit, 32 bit or 128 bit UUIDs can be used.  The string format
  is same as the format of QUuid.

  \sa QBluetoothUuid
  \sa QUuid
  */


QString QDeclarativeBluetoothDiscoveryModel::uuidFilter() const
{
    return d->m_uuid;
}

void QDeclarativeBluetoothDiscoveryModel::setUuidFilter(QString uuid)
{
    if (uuid == d->m_uuid)
        return;

    QBluetoothUuid qbuuid(uuid);
    if (qbuuid.isNull()) {
        qWarning() << "Invalid UUID providded " << uuid;
        return;
    }
    d->m_uuid = uuid;
    emit uuidFilterChanged();
}

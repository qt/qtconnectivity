/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QDECLARATIVECONTACTMODEL_P_H
#define QDECLARATIVECONTACTMODEL_P_H

#include <QAbstractListModel>
#include <QDeclarativeParserStatus>
#include <QDeclarativeListProperty>
#include <QDeclarativeParserStatus>

#include <qbluetoothserviceinfo.h>
#include <qbluetoothservicediscoveryagent.h>

#include <qbluetoothglobal.h>

#include "qdeclarativebluetoothservice_p.h"

QTBLUETOOTH_USE_NAMESPACE

class QDeclarativeBluetoothDiscoveryModelPrivate;
class QDeclarativeBluetoothDiscoveryModel : public QAbstractListModel, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(bool minimalDiscovery READ minimalDiscovery WRITE setMinimalDiscovery NOTIFY minimalDiscoveryChanged)
    Q_PROPERTY(bool discovery READ discovery WRITE setDiscovery NOTIFY discoveryChanged)
    Q_PROPERTY(QString uuidFilter READ uuidFilter WRITE setUuidFilter NOTIFY uuidFilterChanged)
    Q_INTERFACES(QDeclarativeParserStatus)
public:
    explicit QDeclarativeBluetoothDiscoveryModel(QObject *parent = 0);

    enum {
        ServiceRole =  Qt::UserRole + 500,
        AddressRole,
        NameRole
    };

    QString error() const;

    // From QDeclarativeParserStatus
    virtual void classBegin() {}

    virtual void componentComplete();

    // From QAbstractListModel
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    bool minimalDiscovery();
    void setMinimalDiscovery(bool minimalDiscovery_);

    bool discovery();

    QString uuidFilter() const;
    void setUuidFilter(QString uuid);

signals:
    void errorChanged();
    void minimalDiscoveryChanged();
    void newServiceDiscovered(QDeclarativeBluetoothService *service);
    void discoveryChanged();
    void uuidFilterChanged();

public slots:
    void setDiscovery(bool discovery_);

private slots:

    void serviceDiscovered(const QBluetoothServiceInfo &service);
    void finishedDiscovery();
    void errorDiscovery(QBluetoothServiceDiscoveryAgent::Error error);

private:
    QDeclarativeBluetoothDiscoveryModelPrivate* d;
};

#endif // QDECLARATIVECONTACTMODEL_P_H

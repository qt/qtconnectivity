/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#include <QtCore/QCoreApplication>
#include <QtCore/QPointer>
#include <QtBluetooth/QLowEnergyService>
#include <QtBluetooth/QLowEnergyCharacteristicInfo>

#include "qlowenergycontrollernew_p.h"
#include "qlowenergyserviceprivate_p.h"

QT_BEGIN_NAMESPACE

/*!
  \internal

  QLowEnergyControllerNewPrivate creates instances of this class.
  The user gets access to class instances via
  \l QLowEnergyControllerNew::services().
 */
QLowEnergyService::QLowEnergyService(QSharedPointer<QLowEnergyServicePrivate> p,
                                     QObject *parent)
    : QObject(parent),
      d_ptr(p)
{
    connect(p.data(), SIGNAL(error(QLowEnergyService::ServiceError)),
            this, SIGNAL(error(QLowEnergyService::ServiceError)));
    connect(p.data(), SIGNAL(stateChanged(QLowEnergyService::ServiceState)),
            this, SIGNAL(stateChanged(QLowEnergyService::ServiceState)));
}


QLowEnergyService::~QLowEnergyService()
{
}

QList<QSharedPointer<QLowEnergyService> > QLowEnergyService::includedServices() const
{
    QList<QSharedPointer<QLowEnergyService > > results;
    //TODO implement secondary service support
    return results;
}

QLowEnergyService::ServiceState QLowEnergyService::state() const
{
    return d_ptr->state;
}


QLowEnergyService::ServiceType QLowEnergyService::type() const
{
    return d_ptr->type;
}


QList<QLowEnergyCharacteristicInfo> QLowEnergyService::characteristics() const
{
    //TODO implement
    QList<QLowEnergyCharacteristicInfo> results;
    return results;
}


QBluetoothUuid QLowEnergyService::serviceUuid() const
{
    return d_ptr->uuid;
}


QString QLowEnergyService::serviceName() const
{
    bool ok = false;
    quint16 clsId = d_ptr->uuid.toUInt16(&ok);
    if (ok) {
        QBluetoothUuid::ServiceClassUuid id
                = static_cast<QBluetoothUuid::ServiceClassUuid>(clsId);
        return QBluetoothUuid::serviceClassToString(id);
    }
    return qApp ?
           qApp->translate("QBluetoothServiceDiscoveryAgent", "Unknown Service") :
           QStringLiteral("Unknown Service");
}


void QLowEnergyService::discoverDetails()
{
    Q_D(QLowEnergyService);
    if (d->state != QLowEnergyService::DiscoveryRequired)
        return;

    if (!d->controller) {
        d->setError(QLowEnergyService::ServiceNotValidError);
        return;
    }

    d->controller->discoverServiceDetails(d->uuid);
}

QLowEnergyService::ServiceError QLowEnergyService::error() const
{
    return d_ptr->lastError;
}


QT_END_NAMESPACE

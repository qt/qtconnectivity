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

QT_BEGIN_NAMESPACE

class QLowEnergyServicePrivate {
public:
    QBluetoothUuid uuid;
    QLowEnergyService::ServiceType type;
    QLowEnergyService::ServiceState state;
    QLowEnergyService::ServiceError lastError;

    QPointer<QLowEnergyControllerNewPrivate> controller;
};

/*!
  \internal

  QLowEnergyControllerNewPrivate creates instances of this class.
  The user gets access to class instances via
  \l QLowEnergyControllerNew::services().
 */
QLowEnergyService::QLowEnergyService(const QBluetoothUuid &uuid,
                                     QObject *parent)
    : QObject(parent)
{
    d_ptr = new QLowEnergyServicePrivate();
    d_ptr->uuid = uuid;
    d_ptr->state = QLowEnergyService::InvalidService;
    d_ptr->type = QLowEnergyService::PrimaryService;
    d_ptr->lastError = QLowEnergyService::NoError;
}

/*!
  \internal

  Called by Controller right after construction.
 */
void QLowEnergyService::setController(QLowEnergyControllerNewPrivate *control)
{
    Q_D(QLowEnergyService);
    if (!control)
        return;

    d->state = QLowEnergyService::DiscoveryRequired;
    d->controller = control;
}

/*!
 \internal

 Called by Controller.
 */
void QLowEnergyService::setError(QLowEnergyService::ServiceError newError)
{
    Q_D(QLowEnergyService);
    d->lastError = newError;
    emit error(newError);
}

void QLowEnergyService::setState(QLowEnergyService::ServiceState newState)
{
    Q_D(QLowEnergyService);
    d->state = newState;
    emit stateChanged(newState);
}

QLowEnergyService::~QLowEnergyService()
{
    delete d_ptr;
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
        setError(QLowEnergyService::ServiceNotValidError);
        return;
    }

    d->controller->discoverServiceDetails(d->uuid);
}

QLowEnergyService::ServiceError QLowEnergyService::error() const
{
    return d_ptr->lastError;
}


QT_END_NAMESPACE

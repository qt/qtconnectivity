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

#include <algorithm>

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
    qRegisterMetaType<QLowEnergyService::ServiceState>("QLowEnergyService::ServiceState");
    qRegisterMetaType<QLowEnergyService::ServiceError>("QLowEnergyService::ServiceError");

    connect(p.data(), SIGNAL(error(QLowEnergyService::ServiceError)),
            this, SIGNAL(error(QLowEnergyService::ServiceError)));
    connect(p.data(), SIGNAL(stateChanged(QLowEnergyService::ServiceState)),
            this, SIGNAL(stateChanged(QLowEnergyService::ServiceState)));
    connect(p.data(), SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
            this, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)));
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


QList<QLowEnergyCharacteristic> QLowEnergyService::characteristics() const
{
    QList<QLowEnergyCharacteristic> results;
    QList<QLowEnergyHandle> handles = d_ptr->characteristicList.keys();
    std::sort(handles.begin(), handles.end());

    foreach (const QLowEnergyHandle &handle, handles) {
        QLowEnergyCharacteristic characteristic(d_ptr, handle);
        results.append(characteristic);
    }
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

    if (!d->controller || d->state == QLowEnergyService::InvalidService) {
        d->setError(QLowEnergyService::ServiceNotValidError);
        return;
    }

    if (d->state != QLowEnergyService::DiscoveryRequired)
        return;

    d->setState(QLowEnergyService::DiscoveringServices);

    d->controller->discoverServiceDetails(d->uuid);
}

QLowEnergyService::ServiceError QLowEnergyService::error() const
{
    return d_ptr->lastError;
}

bool QLowEnergyService::contains(const QLowEnergyCharacteristic &characteristic)
{
    if (characteristic.d_ptr.isNull() || !characteristic.data)
        return false;

    if (d_ptr == characteristic.d_ptr
        && d_ptr->characteristicList.contains(characteristic.attributeHandle())) {
        return true;
    }

    return false;
}

/*!
    Writes \a newValue as value for the \a characteristic. If the operation is successful
    the \l characteristicChanged() signal will be emitted. \a newValue must contain the
    hexadecimal representation of new value.

    A characteristic can only be written if this service is in the \l ServiceDiscovered state
    and \a characteristic is writable.
 */
void QLowEnergyService::writeCharacteristic(
        const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    //TODO check behavior when writing to WriteNoResponse characteristic
    //TODO check behavior when writing to WriteSigned characteristic
    //TODO add support for write long characteristic value (newValue.size() > MTU - 3)
    Q_D(QLowEnergyService);

    // not a characteristic of this service
    if (!contains(characteristic))
        return;

    // don't write if we don't have to
    if (characteristic.value() == newValue)
        return;

    // don't write write-protected or undiscovered characteristic
    if (!(characteristic.properties() & QLowEnergyCharacteristic::Write)
            || state() != ServiceDiscovered) {
        d->setError(QLowEnergyService::OperationError);
        return;
    }

    if (!d->controller)
        return;

    d->controller->writeCharacteristic(characteristic.d_ptr,
                                       characteristic.attributeHandle(),
                                       newValue);
}


QT_END_NAMESPACE

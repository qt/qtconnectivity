/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qlowenergycharacteristic.h"
#include "qlowenergydescriptor.h"
#include "qlowenergyservice.h"
#include "qbluetoothuuid.h"

#include <QtCore/qstring.h>
#include <QtCore/qlist.h>

QLowEnergyService::QLowEnergyService(QSharedPointer<QLowEnergyServicePrivate> d, QObject *parent)
    : QObject(parent),
      d_ptr(d)
{
}

QLowEnergyService::~QLowEnergyService()
{
}

QList<QBluetoothUuid> QLowEnergyService::includedServices() const
{
    return QList<QBluetoothUuid>();
}

QLowEnergyService::ServiceTypes QLowEnergyService::type() const
{
    return PrimaryService;
}

QLowEnergyService::ServiceState QLowEnergyService::state() const
{
    return InvalidService;
}

QLowEnergyCharacteristic QLowEnergyService::characteristic(const QBluetoothUuid &uuid) const
{
    Q_UNUSED(uuid)

    return QLowEnergyCharacteristic();
}

QList<QLowEnergyCharacteristic> QLowEnergyService::characteristics() const
{
    return QList<QLowEnergyCharacteristic>();
}

QBluetoothUuid QLowEnergyService::serviceUuid() const
{
    return QBluetoothUuid();
}

QString QLowEnergyService::serviceName() const
{
    return QString();
}

void QLowEnergyService::discoverDetails()
{
}

QLowEnergyService::ServiceError QLowEnergyService::error() const
{
    return NoError;
}

bool QLowEnergyService::contains(const QLowEnergyCharacteristic &characteristic) const
{
    Q_UNUSED(characteristic)

    return false;
}

void QLowEnergyService::writeCharacteristic(const QLowEnergyCharacteristic &characteristic,
                                            const QByteArray &newValue,
                                            WriteMode mode)
{
    Q_UNUSED(characteristic)
    Q_UNUSED(newValue)
    Q_UNUSED(mode)
}

bool QLowEnergyService::contains(const QLowEnergyDescriptor &descriptor) const
{
    Q_UNUSED(descriptor)
    return false;
}

void QLowEnergyService::writeDescriptor(const QLowEnergyDescriptor &descriptor,
                                        const QByteArray &newValue)
{
    Q_UNUSED(descriptor)
    Q_UNUSED(newValue)
}

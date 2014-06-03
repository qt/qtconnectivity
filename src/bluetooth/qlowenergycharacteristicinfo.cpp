/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
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

#include "qlowenergycharacteristicinfo.h"
#include "qlowenergycharacteristicinfo_p.h"
#include <QHash>

QT_BEGIN_NAMESPACE

/*!
    \class QLowEnergyCharacteristicInfo
    \inmodule QtBluetooth
    \brief The QLowEnergyCharacteristicInfo class stores information about a Bluetooth
    Low Energy service characteristic.

    \since 5.4

    QLowEnergyCharacteristicInfo provides information about a Bluetooth Low Energy
    service characteristic's name, UUID, value, permissions, handle and descriptors.
    To get the full characteristic specification and information it is necessary to
    connect to the service using QLowEnergyServiceInfo and QLowEnergyController classes.
    Some characteristics can contain none, one or more descriptors.
*/

/*!
    \enum QLowEnergyCharacteristicInfo::PropertyType

    This enum describes the properties of a characteristic.

    \value Unknown                  The type is not known.
    \value Broadcasting             Allow for the broadcasting of Generic Attributes (GATT) characteristic values.
    \value Read                     Allow the characteristic values to be read.
    \value WriteNoResponse          Allow characteristic values without responses to be written.
    \value Write                    Allow for characteristic values to be written.
    \value Notify                   Permits notification of characteristic values.
    \value Indicate                 Permits indications of characteristic values.
    \value WriteSigned              Permits signed writes of the GATT characteristic values.
    \value ExtendedProperty         Additional characteristic properties are defined in the characteristic
                                    extended properties descriptor.

    \sa properties()
*/

/*!
    Construct a new QLowEnergyCharacteristicInfo.
*/
QLowEnergyCharacteristicInfo::QLowEnergyCharacteristicInfo():
    d_ptr(new QLowEnergyCharacteristicInfoPrivate)
{

}

/*!
    Construct a new QLowEnergyCharacteristicInfo with given \a uuid.
*/
QLowEnergyCharacteristicInfo::QLowEnergyCharacteristicInfo(const QBluetoothUuid &uuid):
    d_ptr(new QLowEnergyCharacteristicInfoPrivate)
{
    d_ptr->uuid = uuid;
}

/*!
    Construct a new QLowEnergyCharacteristicInfo that is a copy of \a other.

    The two copies continue to share the same underlying data which does not detach
    upon write.
*/
QLowEnergyCharacteristicInfo::QLowEnergyCharacteristicInfo(const QLowEnergyCharacteristicInfo &other):
    d_ptr(other.d_ptr)
{

}

/*!
    Destroys the QLowEnergyCharacteristicInfo object.
*/
QLowEnergyCharacteristicInfo::~QLowEnergyCharacteristicInfo()
{

}

/*!
    Returns the name of the gatt characteristic.
*/
QString QLowEnergyCharacteristicInfo::name() const
{
    return QBluetoothUuid::characteristicToString(
                static_cast<QBluetoothUuid::CharacteristicType>(d_ptr->uuid.toUInt16()));
}

/*!
    Returns the UUID of the gatt characteristic.
*/
QBluetoothUuid QLowEnergyCharacteristicInfo::uuid() const
{
    return d_ptr->uuid;
}

/*!
    Returns the properties of the gatt characteristic.
*/
QLowEnergyCharacteristicInfo::PropertyTypes QLowEnergyCharacteristicInfo::properties() const
{
    return d_ptr->properties;
}

/*!
    Returns value of the gatt characteristic.
*/
QByteArray QLowEnergyCharacteristicInfo::value() const
{
    return d_ptr->value;
}

/*!
    Returns the handle of the gatt characteristic.
*/
QString QLowEnergyCharacteristicInfo::handle() const
{
    return d_ptr->handle;
}

/*!
    Returns the true or false statement depending whether the characteristic is notification
    (enables conctant updates when value is changed on LE device).
*/
bool QLowEnergyCharacteristicInfo::isNotificationCharacteristic() const
{
    return d_ptr->notification;
}

/*!
    Sets the value \a value of the characteristic. This only caches the value. To write
    a value directly to the device QLowEnergyController class must be used.

    \sa QLowEnergyController::writeCharacteristic()
*/
void QLowEnergyCharacteristicInfo::setValue(const QByteArray &value)
{
    d_ptr->value = value;
}

/*!
    Makes a copy of the \a other and assigns it to this QLowEnergyCharacteristicInfo object.
    The two copies continue to share the same service and registration details.
*/
QLowEnergyCharacteristicInfo &QLowEnergyCharacteristicInfo::operator=(const QLowEnergyCharacteristicInfo &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

/*!
    Returns true if the QLowEnergyCharacteristicInfo object is valid, otherwise returns false.
    If it returns false, it means that service that this characteristic belongs was not connected.
*/
bool QLowEnergyCharacteristicInfo::isValid() const
{
    if (d_ptr->uuid == QBluetoothUuid())
        return false;
    if (d_ptr->handle.toUShort(0,0) == 0)
        return false;
    if (!d_ptr->valid())
        return false;
    return true;
}

/*!
    Returns the list of characteristic descriptors.
*/
QList<QLowEnergyDescriptorInfo> QLowEnergyCharacteristicInfo::descriptors() const
{
    return d_ptr->descriptorsList;
}

QT_END_NAMESPACE

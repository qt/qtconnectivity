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

#include "qlowenergycharacteristic.h"
#include "qlowenergyserviceprivate_p.h"
#include <QHash>

QT_BEGIN_NAMESPACE

/*!
    \class QLowEnergyCharacteristic
    \inmodule QtBluetooth
    \brief The QLowEnergyCharacteristic class stores information about a Bluetooth
    Low Energy service characteristic.

    \since 5.4

    QLowEnergyCharacteristic provides information about a Bluetooth Low Energy
    service characteristic's name, UUID, value, permissions, handle and descriptors.
    To get the full characteristic specification and information it is necessary to
    connect to the service using QLowEnergyServiceInfo and QLowEnergyController classes.
    Some characteristics can contain none, one or more descriptors.
*/

/*!
    \enum QLowEnergyCharacteristic::PropertyType

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

struct QLowEnergyCharacteristicPrivate
{
    QLowEnergyHandle handle;
};

/*!
    Construct a new QLowEnergyCharacteristic.
*/
QLowEnergyCharacteristic::QLowEnergyCharacteristic():
    d_ptr(0), data(0)
{

}

/*!
    Construct a new QLowEnergyCharacteristic that is a copy of \a other.

    The two copies continue to share the same underlying data which does not detach
    upon write.
*/
QLowEnergyCharacteristic::QLowEnergyCharacteristic(const QLowEnergyCharacteristic &other):
    d_ptr(other.d_ptr)
{
    if (!other.data) {
        if (data) {
            delete data;
            data = 0;
        }
    } else {
        if (!data)
            data = new QLowEnergyCharacteristicPrivate();

        data->handle = other.data->handle;
    }
}

/*!
    \internal
*/
QLowEnergyCharacteristic::QLowEnergyCharacteristic(
        QSharedPointer<QLowEnergyServicePrivate> p, QLowEnergyHandle handle):
    d_ptr(p)
{
    data = new QLowEnergyCharacteristicPrivate();
    data->handle = handle;
}

/*!
    Destroys the QLowEnergyCharacteristic object.
*/
QLowEnergyCharacteristic::~QLowEnergyCharacteristic()
{
    delete data;
}

/*!
    Returns the name of the gatt characteristic type.
*/
QString QLowEnergyCharacteristic::name() const
{
    return QBluetoothUuid::characteristicToString(
                static_cast<QBluetoothUuid::CharacteristicType>(uuid().toUInt16()));
}

/*!
    Returns the UUID of the gatt characteristic.
*/
QBluetoothUuid QLowEnergyCharacteristic::uuid() const
{
    if (d_ptr.isNull() || !data
        || !d_ptr->characteristicList.contains(data->handle))
        return QBluetoothUuid();

    return d_ptr->characteristicList[data->handle].uuid;
}

/*!
    Returns the properties of the gatt characteristic.
*/
QLowEnergyCharacteristic::PropertyTypes QLowEnergyCharacteristic::properties() const
{
    if (d_ptr.isNull() || !data
        || !d_ptr->characteristicList.contains(data->handle))
        return QLowEnergyCharacteristic::Unknown;

    return d_ptr->characteristicList[data->handle].properties;
}

/*!
    Returns value of the gatt characteristic.
*/
QByteArray QLowEnergyCharacteristic::value() const
{
    if (d_ptr.isNull() || !data
        || !d_ptr->characteristicList.contains(data->handle))
        return QByteArray();

    return d_ptr->characteristicList[data->handle].value;
}

/*!
    Returns the handle of the GATT characteristic's value attribute;
    otherwise returns \c 0.
*/
QLowEnergyHandle QLowEnergyCharacteristic::handle() const
{
    if (d_ptr.isNull() || !data
        || !d_ptr->characteristicList.contains(data->handle))
        return 0;

    return d_ptr->characteristicList[data->handle].valueHandle;
}

/*!
    Sets the value \a value of the characteristic. This only caches the value. To write
    a value directly to the device QLowEnergyController class must be used.

    \sa QLowEnergyController::writeCharacteristic()
*/
void QLowEnergyCharacteristic::setValue(const QByteArray &value)
{
    //d_ptr->value = value;
}

/*!
    Makes a copy of \a other and assigns it to this QLowEnergyCharacteristic object.
    The two copies continue to share the same service and registration details.
*/
QLowEnergyCharacteristic &QLowEnergyCharacteristic::operator=(const QLowEnergyCharacteristic &other)
{
    d_ptr = other.d_ptr;

    if (!other.data) {
        if (data) {
            delete data;
            data = 0;
        }
    } else {
        if (!data)
            data = new QLowEnergyCharacteristicPrivate();

        data->handle = other.data->handle;
    }
    return *this;
}

/*!
    Returns true if the QLowEnergyCharacteristic object is valid, otherwise returns false.
    If it returns \c false, it means that this instance is not associated to any service
    or the associated service is no longer valid.
*/
bool QLowEnergyCharacteristic::isValid() const
{
    if (d_ptr.isNull() || !data)
        return false;

    if (d_ptr->state == QLowEnergyService::InvalidService)
        return false;

    return true;
}



/*!
    Returns the list of characteristic descriptors.
*/
QList<QLowEnergyDescriptorInfo> QLowEnergyCharacteristic::descriptors() const
{
    // TODO QLowEnergyCharacteristic::descriptors()
    QList<QLowEnergyDescriptorInfo> result;
    return result;

}

QT_END_NAMESPACE

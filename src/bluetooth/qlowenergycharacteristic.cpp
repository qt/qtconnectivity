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
    d_ptr(other.d_ptr), data(0)
{
    if (other.data) {
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

    The returned QByteArray contains the hex representation of the value.
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
    Returns \c true if \a other is equal to this QLowEnergyCharacteristic; otherwise \c false.

    Two QLowEnergyCharcteristic instances are considered to be equal if they refer to
    the same charcteristic on the same remote Bluetooth Low Energy device.
 */
bool QLowEnergyCharacteristic::operator==(const QLowEnergyCharacteristic &other) const
{
    if (d_ptr != other.d_ptr)
        return false;

    if ((data && !other.data) || (!data && other.data))
        return false;

    if (!data)
        return true;

    if (data->handle != other.data->handle)
        return false;

    return true;
}

/*!
    Returns \c true if \a other is not equal to this QLowEnergyCharacteristic; otherwise \c false.

    Two QLowEnergyCharcteristic instances are considered to be equal if they refer to
    the same charcteristic on the same remote Bluetooth Low Energy device.
 */

bool QLowEnergyCharacteristic::operator!=(const QLowEnergyCharacteristic &other) const
{
    return !(*this == other);
}

/*!
    Returns \c true if the QLowEnergyCharacteristic object is valid, otherwise returns \c false.

    An invalid characteristic object is not associated to any service
    or the associated service is no longer valid due to for example a disconnect from
    the underlying Bluetooth Low Energy device. Once the object is invalid
    it cannot become valid anymore.

    \note If a QLowEnergyCharacteristic instance turns invalid due to a disconnect
    from the underlying device, the information encapsulated by the current
    instance remains as it was at the time of the disconnect.
*/
bool QLowEnergyCharacteristic::isValid() const
{
    if (d_ptr.isNull() || !data)
        return false;

    if (d_ptr->state == QLowEnergyService::InvalidService)
        return false;

    return true;
}

QLowEnergyHandle QLowEnergyCharacteristic::attributeHandle() const
{
    if (d_ptr.isNull() || !data)
        return 0;

    return data->handle;
}


/*!
    Returns the descriptor with \a uuid; otherwise an invalid \c QLowEnergyDescriptor
    instance.

    \sa descriptors()
*/
QLowEnergyDescriptor QLowEnergyCharacteristic::descriptor(const QBluetoothUuid &uuid) const
{
    if (d_ptr.isNull() || !data)
        return QLowEnergyDescriptor();

    QList<QLowEnergyHandle> descriptorKeys = d_ptr->characteristicList[data->handle].
                                                    descriptorList.keys();
    foreach (const QLowEnergyHandle descHandle, descriptorKeys) {
        if (uuid == d_ptr->characteristicList[data->handle].descriptorList[descHandle].uuid)
            return QLowEnergyDescriptor(d_ptr, data->handle, descHandle);
    }

    return QLowEnergyDescriptor();
}

/*!
    Returns the list of descriptors belonging to this characteristic; otherwise
    an empty list.

    \sa descriptor()
*/
QList<QLowEnergyDescriptor> QLowEnergyCharacteristic::descriptors() const
{
    QList<QLowEnergyDescriptor> result;

    if (d_ptr.isNull() || !data
                       || !d_ptr->characteristicList.contains(data->handle))
        return result;

    QList<QLowEnergyHandle> descriptorKeys = d_ptr->characteristicList[data->handle].
                                                    descriptorList.keys();

    std::sort(descriptorKeys.begin(), descriptorKeys.end());

    foreach (const QLowEnergyHandle descHandle, descriptorKeys) {
        QLowEnergyDescriptor descriptor(d_ptr, data->handle, descHandle);
        result.append(descriptor);
    }

    return result;
}

QT_END_NAMESPACE

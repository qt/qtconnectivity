/***************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#include <QtBluetooth/QLowEnergyService>
#include "qlowenergyserviceprivate_p.h"
#include "qlowenergydescriptor.h"

QT_BEGIN_NAMESPACE

/*!
    \class QLowEnergyDescriptor
    \inmodule QtBluetooth
    \brief The QLowEnergyDescriptor class stores information about the Bluetooth
    Low Energy descriptor.
    \since 5.4

    QLowEnergyDescriptor provides information about a Bluetooth Low Energy
    descriptor's name, UUID, value and handle. Descriptors are contained in the
    Bluetooth Low Energy characteristic and they provide additional information
    about the characteristic (data format, notification activation, etc).
*/

struct QLowEnergyDescriptorPrivate
{
    QLowEnergyHandle charHandle;
    QLowEnergyHandle descHandle;
};

/*!
    Construct a new QLowEnergyDescriptor.
*/
QLowEnergyDescriptor::QLowEnergyDescriptor():
    d_ptr(0), data(0)
{
}

/*!
    Construct a new QLowEnergyDesxcriptor that is a copy of \a other.

    The two copies continue to share the same underlying data which does not detach
    upon write.
*/
QLowEnergyDescriptor::QLowEnergyDescriptor(const QLowEnergyDescriptor &other):
    d_ptr(other.d_ptr), data(0)
{
    if (other.data) {
        data = new QLowEnergyDescriptorPrivate();
        data->charHandle = other.data->charHandle;
        data->descHandle = other.data->descHandle;
    }
}

/*!
    \internal

*/
QLowEnergyDescriptor::QLowEnergyDescriptor(QSharedPointer<QLowEnergyServicePrivate> p,
                                           QLowEnergyHandle charHandle,
                                           QLowEnergyHandle descHandle):
    d_ptr(p)
{
    data = new QLowEnergyDescriptorPrivate();
    data->charHandle = charHandle;
    data->descHandle = descHandle;

}

/*!
    Destroys the QLowEnergyDescriptor object.
*/
QLowEnergyDescriptor::~QLowEnergyDescriptor()
{
    delete data;
}

/*!
    Makes a copy of \a other and assigns it to this QLowEnergyDescriptor object.
    The two copies continue to share the same service and registration details.
*/
QLowEnergyDescriptor &QLowEnergyDescriptor::operator=(const QLowEnergyDescriptor &other)
{
    d_ptr = other.d_ptr;

    if (!other.data) {
        if (data) {
            delete data;
            data = 0;
        }
    } else {
        if (!data)
            data = new QLowEnergyDescriptorPrivate();

        data->charHandle = other.data->charHandle;
        data->descHandle = other.data->descHandle;
    }

    return *this;
}

/*!
    Returns \c true if \a other is equal to this QLowEnergyCharacteristic; otherwise \c false.

    Two QLowEnergyDescriptor instances are considered to be equal if they refer to
    the same descriptor on the same remote Bluetooth Low Energy device.
 */
bool QLowEnergyDescriptor::operator==(const QLowEnergyDescriptor &other) const
{
    if (d_ptr != other.d_ptr)
        return false;

    if ((data && !other.data) || (!data && other.data))
        return false;

    if (!data)
        return true;

    if (data->charHandle != other.data->charHandle
        || data->descHandle != other.data->descHandle) {
        return false;
    }

    return true;
}

/*!
    Returns \c true if \a other is not equal to this QLowEnergyCharacteristic; otherwise \c false.

    Two QLowEnergyDescriptor instances are considered to be equal if they refer to
    the same descriptor on the same remote Bluetooth Low Energy device.
 */
bool QLowEnergyDescriptor::operator!=(const QLowEnergyDescriptor &other) const
{
    return !(*this == other);
}

/*!
    Returns \c true if the QLowEnergyDescriptor object is valid, otherwise returns \c false.

    An invalid descriptor instance is not associated to any service
    or the associated service is no longer valid due to for example a disconnect from
    the underlying Bluetooth Low Energy device. Once the object is invalid
    it cannot become valid anymore.

    \note If a QLowEnergyDescriptor instance turns invalid due to a disconnect
    from the underlying device, the information encapsulated by the current
    instance remains as it was at the time of the disconnect.
*/
bool QLowEnergyDescriptor::isValid() const
{
    if (d_ptr.isNull() || !data)
        return false;

    if (d_ptr->state == QLowEnergyService::InvalidService)
        return false;

    return true;
}

/*!
    Returns the UUID of this descriptor.
*/
QBluetoothUuid QLowEnergyDescriptor::uuid() const
{
    if (d_ptr.isNull() || !data
                       || !d_ptr->characteristicList.contains(data->charHandle)
                       || !d_ptr->characteristicList[data->charHandle].
                                        descriptorList.contains(data->descHandle)) {
        return QBluetoothUuid();
    }

    return d_ptr->characteristicList[data->charHandle].descriptorList[data->descHandle].uuid;
}

/*!
    Returns the handle of the descriptor.
*/
QLowEnergyHandle QLowEnergyDescriptor::handle() const
{
    if (!data)
        return 0;

    return data->descHandle;
}

/*!
    Returns the value of the descriptor.
*/
QByteArray QLowEnergyDescriptor::value() const
{
    if (d_ptr.isNull() || !data
                       || !d_ptr->characteristicList.contains(data->charHandle)
                       || !d_ptr->characteristicList[data->charHandle].
                                        descriptorList.contains(data->descHandle)) {
        return QByteArray();
    }

    return d_ptr->characteristicList[data->charHandle].descriptorList[data->descHandle].value;
}

/*!
    Returns the name of the descriptor type.

    \sa type()
*/

QString QLowEnergyDescriptor::name() const
{
    return QBluetoothUuid::descriptorToString(type());
}

/*!
    Returns the type of descriptor.
 */
QBluetoothUuid::DescriptorType QLowEnergyDescriptor::type() const
{
    const QBluetoothUuid u = uuid();
    bool ok = false;
    quint16 shortUuid = u.toUInt16(&ok);

    if (!ok)
        return QBluetoothUuid::UnknownDescriptorType;

    switch (shortUuid) {
    case QBluetoothUuid::CharacteristicExtendedProperties:
    case QBluetoothUuid::CharacteristicUserDescription:
    case QBluetoothUuid::ClientCharacteristicConfiguration:
    case QBluetoothUuid::ServerCharacteristicConfiguration:
    case QBluetoothUuid::CharacteristicPresentationFormat:
    case QBluetoothUuid::CharacteristicAggregateFormat:
    case QBluetoothUuid::ValidRange:
    case QBluetoothUuid::ExternalReportReference:
    case QBluetoothUuid::ReportReference:
        return (QBluetoothUuid::DescriptorType) shortUuid;
    default:
        break;
    }

    return QBluetoothUuid::UnknownDescriptorType;
}

/*!
    \internal

    Returns the handle of the characteristic to which this descriptor belongs
 */
QLowEnergyHandle QLowEnergyDescriptor::characteristicHandle() const
{
    if (d_ptr.isNull() || !data)
        return 0;

    return data->charHandle;
}

QT_END_NAMESPACE

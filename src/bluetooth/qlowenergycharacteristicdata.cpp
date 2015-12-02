/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
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
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qlowenergycharacteristicdata.h"

#include "qlowenergydescriptordata.h"

#include <QtCore/qbytearray.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT)

struct QLowEnergyCharacteristicDataPrivate : public QSharedData
{
    QLowEnergyCharacteristicDataPrivate() : properties(QLowEnergyCharacteristic::Unknown) {}

    QBluetoothUuid uuid;
    QLowEnergyCharacteristic::PropertyTypes properties;
    QList<QLowEnergyDescriptorData> descriptors;
    QByteArray value;
    QBluetooth::AttAccessConstraints readConstraints;
    QBluetooth::AttAccessConstraints writeConstraints;
};

/*!
    \since 5.7
    \class QLowEnergyCharacteristicData
    \brief The QLowEnergyCharacteristicData class is used to set up GATT service data.
    \inmodule QtBluetooth
    \ingroup shared

    An Object of this class provides a characteristic to be added to a
    \l QLowEnergyServiceData object via \l QLowEnergyServiceData::addCharacteristic().

    \sa QLowEnergyServiceData
    \sa QLowEnergyController::addService
*/

/*! Creates a new invalid object of this class. */
QLowEnergyCharacteristicData::QLowEnergyCharacteristicData()
    : d(new QLowEnergyCharacteristicDataPrivate)
{
}

/*! Constructs a new object of this class that is a copy of \a other. */
QLowEnergyCharacteristicData::QLowEnergyCharacteristicData(const QLowEnergyCharacteristicData &other)
    : d(other.d)
{
}

/*! Destroys this object. */
QLowEnergyCharacteristicData::~QLowEnergyCharacteristicData()
{
}

/*! Makes this object a copy of \a other and returns the new value of this object. */
QLowEnergyCharacteristicData &QLowEnergyCharacteristicData::operator=(const QLowEnergyCharacteristicData &other)
{
    d = other.d;
    return *this;
}

/*! Returns the UUID of this characteristic. */
QBluetoothUuid QLowEnergyCharacteristicData::uuid() const
{
    return d->uuid;
}

/*! Sets the UUID of this characteristic to \a uuid. */
void QLowEnergyCharacteristicData::setUuid(const QBluetoothUuid &uuid)
{
    d->uuid = uuid;
}

/*! Returns the value of this characteristic. */
QByteArray QLowEnergyCharacteristicData::value() const
{
    return d->value;
}

/*! Sets the value of this characteristic to \a value. */
void QLowEnergyCharacteristicData::setValue(const QByteArray &value)
{
    d->value = value;
}

/*! Returns the properties of this characteristic. */
QLowEnergyCharacteristic::PropertyTypes QLowEnergyCharacteristicData::properties() const
{
    return d->properties;
}

/*! Sets the properties of this characteristic to \a properties. */
void QLowEnergyCharacteristicData::setProperties(QLowEnergyCharacteristic::PropertyTypes properties)
{
    d->properties = properties;
}

/*! Returns the descriptors of this characteristic. */
QList<QLowEnergyDescriptorData> QLowEnergyCharacteristicData::descriptors() const
{
    return d->descriptors;
}

/*!
  Sets the descriptors of this characteristic to \a descriptors. Only valid descriptors
  are considered.
  \sa addDescriptor()
  */
void QLowEnergyCharacteristicData::setDescriptors(const QList<QLowEnergyDescriptorData> &descriptors)
{
    foreach (const QLowEnergyDescriptorData &desc, descriptors)
        addDescriptor(desc);
}

/*!
  Adds \a descriptor to the list of descriptors of this characteristic, if it is valid.
  \sa setDescriptors()
 */
void QLowEnergyCharacteristicData::addDescriptor(const QLowEnergyDescriptorData &descriptor)
{
    if (descriptor.isValid())
        d->descriptors << descriptor;
    else
        qCWarning(QT_BT) << "not adding invalid descriptor to characteristic";
}

/*!
  Specifies that clients need to fulfill \a constraints to read the value of this characteristic.
 */
void QLowEnergyCharacteristicData::setReadConstraints(QBluetooth::AttAccessConstraints constraints)
{
    d->readConstraints = constraints;
}

/*!
  Returns the constraints needed for a client to read the value of this characteristic.
  If \l properties() does not include \l QLowEnergyCharacteristic::Read, this value is irrelevant.
  By default, there are no read constraints.
 */
QBluetooth::AttAccessConstraints QLowEnergyCharacteristicData::readConstraints() const
{
    return d->readConstraints;
}

/*!
  Specifies that clients need to fulfill \a constraints to write the value of this characteristic.
 */
void QLowEnergyCharacteristicData::setWriteConstraints(QBluetooth::AttAccessConstraints constraints)
{
    d->writeConstraints = constraints;
}

/*!
  Returns the constraints needed for a client to write the value of this characteristic.
  If \l properties() does not include either of \l QLowEnergyCharacteristic::Write,
  \l QLowEnergyCharacteristic::WriteNoResponse and \l QLowEnergyCharacteristic::WriteSigned,
  this value is irrelevant.
  By default, there are no write constraints.
 */
QBluetooth::AttAccessConstraints QLowEnergyCharacteristicData::writeConstraints() const
{
    return d->writeConstraints;
}

/*!
  Returns true if and only if this characteristic is valid, that is, it has a non-null UUID.
 */
bool QLowEnergyCharacteristicData::isValid() const
{
    return !uuid().isNull();
}

/*!
   \fn void QLowEnergyCharacteristicData::swap(QLowEnergyCharacteristicData &other)
    Swaps this object with \a other.
 */

/*!
   Returns \c true if \a cd1 and \a cd2 are equal with respect to their public state,
   otherwise returns \c false.
 */
bool operator==(const QLowEnergyCharacteristicData &cd1, const QLowEnergyCharacteristicData &cd2)
{
    return cd1.d == cd2.d || (
                cd1.uuid() == cd2.uuid()
                && cd1.properties() == cd2.properties()
                && cd1.descriptors() == cd2.descriptors()
                && cd1.value() == cd2.value()
                && cd1.readConstraints() == cd2.readConstraints()
                && cd1.writeConstraints() == cd2.writeConstraints());
}

/*!
   \fn bool operator!=(const QLowEnergyCharacteristicData &cd1,
                       const QLowEnergyCharacteristicData &cd2)
   Returns \c true if \a cd1 and \a cd2 are not equal with respect to their public state,
   otherwise returns \c false.
 */

QT_END_NAMESPACE

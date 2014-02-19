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

    QLowEnergyCharacteristicInfo provides information about a Bluetooth Low Energy
    service characteristic's name, UUID, value, permissions, handle and descriptors.
    To get the full characteristic specification and information it is necessary to
    connect to the service using QLowEnergyServiceInfo and QLowEnergyController classes.
    Some characteristics can contain none, one or more descriptors.
*/

/*!
    \enum QLowEnergyCharacteristicInfo::Error

    This enum describes the type of an error that can appear.

    \value NotificationFail         Could not subscribe or could not get notifications from wanted characteristic.
    \value NotConnected             GATT Service is not connected.
    \value UnknownError             Unknown error.
*/

/*!
    \enum QLowEnergyCharacteristicInfo::Property

    This enum describes the properties of a characteristic.

    \value Broadcasting             Allow for the broadcasting of Generic Attributes (GATT) characteristic values.
    \value Read                     Allow the characteristic values to be read.
    \value WriteNoResponse          Allow characteristic values without responses to be written.
    \value Write                    Allow for characteristic values to be written.
    \value Notify                   Permits notification of characteristic values.
    \value Indicate                 Permits indications of characteristic values.
    \value WriteSigned              Permits signed writes of the GATT characteristic values.
    \value ExtendedProperty         Additional characteristic properties are defined in the characteristic
                                    extended properties descriptor.
*/

/*!
    Method for parsing the characteristic name with given \a uuid.
 * \brief parseUuid
 * \param uuid
 * \return
 */
QString parseUuid(QBluetoothUuid uuid) {
    static QHash<int, QString> uuidnames;
    if ( uuidnames.isEmpty() ) {
        uuidnames[0x2a43] = QStringLiteral("Alert Category ID");
        uuidnames[0x2A42] = QStringLiteral("Alert Category ID Bit Mask");
        uuidnames[0x2A06] = QStringLiteral("Alert Level");
        uuidnames[0x2A44] = QStringLiteral("Alert Notification Control Point");
        uuidnames[0x2A3F] = QStringLiteral("Alert Status");
        uuidnames[0x2A01] = QStringLiteral("GAP Appearance");
        uuidnames[0x2A19] = QStringLiteral("Battery Level");
        uuidnames[0x2A49] = QStringLiteral("Blood Pressure Feature");
        uuidnames[0x2A35] = QStringLiteral("Blood Pressure Measurement");
        uuidnames[0x2A38] = QStringLiteral("Body Sensor Location");
        uuidnames[0x2A22] = QStringLiteral("Boot Keyboard Input Report");
        uuidnames[0x2A32] = QStringLiteral("Boot Keyboard Output Report");
        uuidnames[0x2A33] = QStringLiteral("Boot Mouse Input Report");
        uuidnames[0x2A2B] = QStringLiteral("Current Time");
        uuidnames[0x2A08] = QStringLiteral("Date Time");
        uuidnames[0x2A0A] = QStringLiteral("Day Date Time");
        uuidnames[0x2A09] = QStringLiteral("Day of Week");
        uuidnames[0x2A00] = QStringLiteral("GAP Device Name");
        uuidnames[0x2A0D] = QStringLiteral("DST Offset");
        uuidnames[0x2A0C] = QStringLiteral("Exact Time 256");
        uuidnames[0x2A26] = QStringLiteral("Firmware Revision String");
        uuidnames[0x2A51] = QStringLiteral("Glucose Feature");
        uuidnames[0x2A18] = QStringLiteral("Glucose Measurement");
        uuidnames[0x2A34] = QStringLiteral("Glucose Measurement Context");
        uuidnames[0x2A27] = QStringLiteral("Hardware Revision String");
        uuidnames[0x2A39] = QStringLiteral("Heart Rate Control Point");
        uuidnames[0x2A37] = QStringLiteral("Heart Rate Measurement");
        uuidnames[0x2A4C] = QStringLiteral("HID Control Point");
        uuidnames[0x2A4A] = QStringLiteral("HID Information");
        uuidnames[0x2A2A] = QStringLiteral("IEEE 11073 20601 Regulatory Certification Data List");
        uuidnames[0x2A36] = QStringLiteral("Intermediate Blood Pressure");
        uuidnames[0x2A1E] = QStringLiteral("Iintermediate Temperature");
        uuidnames[0x2A0F] = QStringLiteral("Local Time Information");
        uuidnames[0x2A29] = QStringLiteral("Manufacturer Name String");
        uuidnames[0x2A21] = QStringLiteral("Measurement Interval");
        uuidnames[0x2A24] = QStringLiteral("Model Number String");
        uuidnames[0x2A46] = QStringLiteral("New Alert");
        uuidnames[0x2A04] = QStringLiteral("GAP Peripheral Preferred Connection Parameters");
        uuidnames[0x2A02] = QStringLiteral("GAP Peripheral Privacy Flag");
        uuidnames[0x2A50] = QStringLiteral("PNP ID");
        uuidnames[0x2A4E] = QStringLiteral("Protocol Mode");
        uuidnames[0x2A03] = QStringLiteral("GAP Reconnection Address");
        uuidnames[0x2A52] = QStringLiteral("Record Access Control Point");
        uuidnames[0x2A14] = QStringLiteral("Reference Time Information");
        uuidnames[0x2A4D] = QStringLiteral("Report");
        uuidnames[0x2A4B] = QStringLiteral("Report Map");
        uuidnames[0x2A40] = QStringLiteral("Ringer Control Point");
        uuidnames[0x2A41] = QStringLiteral("Ringer Setting");
        uuidnames[0x2A4F] = QStringLiteral("Scan Interval Window");
        uuidnames[0x2A31] = QStringLiteral("Scan Refresh");
        uuidnames[0x2A25] = QStringLiteral("Serial Number String");
        uuidnames[0x2A05] = QStringLiteral("GATT Service Changed");
        uuidnames[0x2A28] = QStringLiteral("Software Revision String");
        uuidnames[0x2A47] = QStringLiteral("Supported New Alert Category");
        uuidnames[0x2A48] = QStringLiteral("Supported Unread Alert Category");
        uuidnames[0x2A23] = QStringLiteral("System ID");
        uuidnames[0x2A1C] = QStringLiteral("Temperature Measurement");
        uuidnames[0x2A1D] = QStringLiteral("Temperature Type");
        uuidnames[0x2A12] = QStringLiteral("Time Accuracy");
        uuidnames[0x2A13] = QStringLiteral("Time Source");
        uuidnames[0x2A16] = QStringLiteral("Time Update Control Point");
        uuidnames[0x2A17] = QStringLiteral("Time Update State");
        uuidnames[0x2A11] = QStringLiteral("Time With DST");
        uuidnames[0x2A0E] = QStringLiteral("Time Zone");
        uuidnames[0x2A07] = QStringLiteral("TX Power");
        uuidnames[0x2A45] = QStringLiteral("Unread Alert Status");
    }
    QString name = uuidnames.value(uuid.toUInt16(), QStringLiteral("Unknown Characteristic"));
    return name;

}

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
    d_ptr->name = parseUuid(d_ptr->uuid);
    return d_ptr->name;
}

/*!
    Returns the UUID of the gatt characteristic.
*/
QBluetoothUuid QLowEnergyCharacteristicInfo::uuid() const
{
    return d_ptr->uuid;
}

/*!
    Returns the properties value of the gatt characteristic.
*/
QVariantMap QLowEnergyCharacteristicInfo::properties() const
{
    return d_ptr->properties;
}

/*!
    Returns permissions of the gatt characteristic.
*/
int QLowEnergyCharacteristicInfo::permissions() const
{
    return d_ptr->permission;
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
    Writes the value \a value directly to LE device. If the value was not written successfully
    an error will be emitted with an error string.

    \sa errorString()
*/
void QLowEnergyCharacteristicInfo::writeValue(const QByteArray &value)
{
    d_ptr->setValue(value);
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

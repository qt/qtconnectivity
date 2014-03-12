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

#include "qlowenergydescriptorinfo.h"
#include "qlowenergydescriptorinfo_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QLowEnergyDescriptorInfo
    \inmodule QtBluetooth
    \brief The QLowEnergyDescriptorInfo class stores information about the Bluetooth
    Low Energy descriptor.

    QLowEnergyDescriptorInfo provides information about a Bluetooth Low Energy
    descriptor's name, UUID, value and handle. Descriptors are contained in the
    Bluetooth Low Energy characteristic and they provide additional information
    about the characteristic (data format, notification activation, etc).
*/

QString parseDescriptorUuid(const QBluetoothUuid &uuid)
{
    static QHash<int, QString> uuidnames;
    if ( uuidnames.isEmpty() ) {
        uuidnames[0x2905] = QStringLiteral("Characteristic Aggregate Format");
        uuidnames[0x2900] = QStringLiteral("Characteristic Extended Properties");
        uuidnames[0x2904] = QStringLiteral("Characteristic Presentation Format");
        uuidnames[0x2901] = QStringLiteral("Characteristic User Description");
        uuidnames[0x2902] = QStringLiteral("Client Characteristic Configuration");
        uuidnames[0x2907] = QStringLiteral("External Report Reference");
        uuidnames[0x2908] = QStringLiteral("Report Reference");
        uuidnames[0x2903] = QStringLiteral("Server Characteristic Configuration");
        uuidnames[0x2906] = QStringLiteral("Valid Range");
    }
    QString name = uuidnames.value(uuid.toUInt16(), QStringLiteral("Unknow Descriptor"));
    return name;
}

QLowEnergyDescriptorInfoPrivate::QLowEnergyDescriptorInfoPrivate(const QBluetoothUuid &uuid, const QString &handle):
    m_value(QByteArray()), m_uuid(uuid), m_handle(handle), m_properties(QVariantMap()), m_name(QStringLiteral(""))
{
#ifdef QT_QNX_BLUETOOTH
    instance = -1;
#endif
}

QLowEnergyDescriptorInfoPrivate::~QLowEnergyDescriptorInfoPrivate()
{

}

/*!
    Construct a new QLowEnergyCharacteristicInfo with given \a uuid.
*/
QLowEnergyDescriptorInfo::QLowEnergyDescriptorInfo(const QBluetoothUuid &uuid):
    d_ptr(new QLowEnergyDescriptorInfoPrivate(uuid, QStringLiteral("0x0000")))
{

}

/*!
    Construct a new QLowEnergyDescriptorInfo with given \a uuid and the \a handle.
*/
QLowEnergyDescriptorInfo::QLowEnergyDescriptorInfo(const QBluetoothUuid &uuid, const QString &handle):
    d_ptr(new QLowEnergyDescriptorInfoPrivate(uuid, handle))
{
     d_ptr->m_name = parseDescriptorUuid(uuid);
}

/*!
    Destroys the QLowEnergyDescriptorInfo object.
*/
QLowEnergyDescriptorInfo::~QLowEnergyDescriptorInfo()
{

}

/*!
    Makes a copy of the \a other and assigns it to this QLowEnergyDescriptorInfo object.
    The two copies continue to share the same service and registration details.
*/
QLowEnergyDescriptorInfo &QLowEnergyDescriptorInfo::operator=(const QLowEnergyDescriptorInfo &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

/*!
    Returns the UUID of the descriptor.
*/
QBluetoothUuid QLowEnergyDescriptorInfo::uuid() const
{
    return d_ptr->m_uuid;
}

/*!
    Returns the handle of the descriptor.
*/
QString QLowEnergyDescriptorInfo::handle() const
{
    return d_ptr->m_handle;
}

/*!
    Returns the value of the descriptor.
*/
QByteArray QLowEnergyDescriptorInfo::value() const
{
    return d_ptr->m_value;
}

/*!
    Returns the properties of the descriptor.
*/
QVariantMap QLowEnergyDescriptorInfo::properties() const
{
    return d_ptr->m_properties;
}

/*!
    Returns the name of the descriptor.
*/
QString QLowEnergyDescriptorInfo::name() const
{
    return d_ptr->m_name;
}

/*!
    Sets the value \a value of the descriptor. This only caches the value. To write
    a value directly to the device QLowEnergyController class must be used.

    \sa QLowEnergyController::writeDescriptor()
*/
void QLowEnergyDescriptorInfo::setValue(const QByteArray &value)
{
    d_ptr->m_value = value;
}

QT_END_NAMESPACE

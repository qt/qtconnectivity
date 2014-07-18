/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited all rights reserved
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

#include "qlowenergyserviceinfo.h"
#include "qlowenergyserviceinfo_p.h"
#include <QtCore/QCoreApplication>

QT_BEGIN_NAMESPACE

/*!
    \class QLowEnergyServiceInfo
    \inmodule QtBluetooth
    \brief The QLowEnergyServiceInfo class stores information about the Bluetooth
    Low Energy service.
    \since 5.4

    QLowEnergyServiceInfo provides information about a Bluetooth Low Energy
    service's name, device, UUID, connection status, service type, handle
    and characteristics. A Bluetooth Low Energy device can have one or more
    low energy services. Each low energy service contains one or more
    characteristics. The class is used with the QLowEnergyController
    class. It is necessary to connect to the service first in order
    to get the all service information and characteristics.
*/

/*!
    \enum QLowEnergyServiceInfo::ServiceType

    This enum describes the type of the service. One LE device can have one or more primary services.

    \value PrimaryService           The primary service. The primary service can have one or
                                    more included services.
    \value IncludedService          The included service by primary services.
*/

/*!
    Construct a new QLowEnergyServiceInfo.
*/
QLowEnergyServiceInfo::QLowEnergyServiceInfo():
    d_ptr(QSharedPointer<QLowEnergyServiceInfoPrivate>(new QLowEnergyServiceInfoPrivate))
{

}

/*!
    Construct a new QLowEnergyServiceInfo object with the given \a uuid.

    Based on uuid, corresponsing service name is given.
*/
QLowEnergyServiceInfo::QLowEnergyServiceInfo(const QBluetoothUuid &uuid):
    d_ptr(QSharedPointer<QLowEnergyServiceInfoPrivate>(new QLowEnergyServiceInfoPrivate))
{
    d_ptr->uuid = QBluetoothUuid(uuid);
}

/*!
    Construct a new QLowEnergyServiceInfo that is a copy of \a other.

    The two copies continue to share the same underlying data which does not detach
    upon write.
*/
QLowEnergyServiceInfo::QLowEnergyServiceInfo(const QLowEnergyServiceInfo &other):
    d_ptr(other.d_ptr)
{

}

/*!
    Destroys the QLowEnergyServiceInfo object.
*/
QLowEnergyServiceInfo::~QLowEnergyServiceInfo()
{

}

/*!
    Returns the gatt service uuid.
*/
QBluetoothUuid QLowEnergyServiceInfo::serviceUuid() const
{
    return d_ptr->uuid;
}

/*!
    Returns the service name.
*/
QString QLowEnergyServiceInfo::serviceName() const
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

/*!
    Sets service type with \a type.
*/
void QLowEnergyServiceInfo::setServiceType(QLowEnergyServiceInfo::ServiceType type)
{
    d_ptr->serviceType = type;
}

/*!
    Returns the service type. If setServiceType is not called default service type
    (PrimaryService) is returned.
*/
QLowEnergyServiceInfo::ServiceType QLowEnergyServiceInfo::serviceType() const
{
    return d_ptr->serviceType;
}

/*!
    Returns a copy of \a other and assigns it to this QLowEnergyServiceInfo object.
    The two copies continue to share the same service and registration details.
*/
QLowEnergyServiceInfo &QLowEnergyServiceInfo::operator=(const QLowEnergyServiceInfo &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

/*!
    Returns \c true if this service is connected; otherwise returns \c false.
 */
bool QLowEnergyServiceInfo::isConnected() const
{
    return d_ptr->connected;
}

/*!
    Returns the address of the Bluetooth device that provides this service.
*/
QBluetoothDeviceInfo QLowEnergyServiceInfo::device() const
{
    return d_ptr->deviceInfo;
}

/*!
    Sets the Bluetooth device that provides this service to \a device.
*/
void QLowEnergyServiceInfo::setDevice(const QBluetoothDeviceInfo &device)
{
    d_ptr->deviceInfo = device;
}

/*!
    Returns true if the QLowEnergyServiceInfo object is valid, otherwise returns false.
*/
bool QLowEnergyServiceInfo::isValid() const
{
    if (d_ptr->uuid == QBluetoothUuid())
        return false;
    if (!d_ptr->deviceInfo.isValid())
        return false;
    return true;
}

QT_END_NAMESPACE



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

#include "qlowenergycontroller.h"
#include "qlowenergycontroller_p.h"
#include "qlowenergyserviceinfo_p.h"
#include "qlowenergycharacteristicinfo_p.h"

QT_BEGIN_NAMESPACE
/*!
    \class QLowEnergyController
    \inmodule QtBluetooth
    \brief The QLowEnergyController class is used for connecting and reading data from LE device
    services.

    To read data contained in LE service characteristics, the LE device must be paired and turned on.
    First, connect necessary signals from the class and then connect to service by passing the wanted service
    to \l connectoToService() function. \l connected() signal will be emitted on success or \l error()
    signal in case of failure. It is possible to get error message by calling errorString function
    in QLowEnergyServiceInfo class.

    In case of disconnecting from service, pass the wanted service to \l disconnectFromService()
    function and \l disconnected signal will be emitted.

    It is enough to use one instance of this class to connect to more service on more devices,
    but it is not possible to create more QLowEnergyController classes and connect to same
    service on the same LE device.


    \sa QLowEnergyServiceInfo, QLowEnergyCharacteristicInfo
*/

/*!
    \fn void QLowEnergyController::connected(const QLowEnergyServiceInfo &)

    This signal is emitted when a LE service is connected, passing connected QLowEnergyServiceInfo
    instance.

    \sa connectToService(const QLowEnergyServiceInfo &leService)
*/

/*!
    \fn void QLowEnergyController::error(const QLowEnergyServiceInfo &)

    This signal is emitted when the service error occurs.

    \sa errorString()
*/

/*!
    \fn void QLowEnergyController::error(const QLowEnergyCharacteristicInfo &)

    This signal is emitted when the characteristic error occurs.

    \sa errorString()
*/

/*!
    \fn void QLowEnergyController::disconnected(const QLowEnergyServiceInfo &)

    Emits disconnected signal with disconnected LE service.
*/

/*!
    \fn QLowEnergyController::valueChanged(const QLowEnergyCharacteristicInfo &)

    Emits a signal when the characteristic value is changed. This signal is emitted
    after calling \l enableNotifications(const QLowEnergyCharacteristicInfo &characteristic)
    function.

    \sa enableNotifications(const QLowEnergyCharacteristicInfo &characteristic)
*/


/*!
    Construct a new QLowEnergyController object with \a parent.
*/
QLowEnergyController::QLowEnergyController(QObject *parent)
    : QObject(parent), d_ptr(new QLowEnergyControllerPrivate)
{
    d_ptr->q_ptr = this;
}

/*!
   Constructs a new QLowEnergyController for \a localAdapter and with \a parent.

   It uses \a localAdapter for the device connection. If \a localAdapter is default constructed
   the resulting object will use the local default Bluetooth adapter.
*/
QLowEnergyController::QLowEnergyController(const QBluetoothAddress &localAdapter, QObject *parent)
    : QObject(parent), d_ptr(new QLowEnergyControllerPrivate)
{
    d_ptr->q_ptr = this;
    d_ptr->localAdapter = localAdapter;
}

/*!
    Destroys the QLowEnergyController instance.
*/
QLowEnergyController::~QLowEnergyController()
{

}

/*!
    Returns the list of all Bluetooth LE Services that were added.
*/
QList<QLowEnergyServiceInfo> QLowEnergyController::services() const
{
    return d_ptr->m_leServices;
}

/*!
    Connects to the given \a leService. If the given service is not valid, or if
    the given service is already connected, nothing will happen.

*/
void QLowEnergyController::connectToService(const QLowEnergyServiceInfo &leService)
{
    d_ptr->connectService(leService);
}


/*!
    Disconnects the given \a leService. If the given service was not connected, nothing will be done.

    If the \a leService is not passed or it is invalid, all connected services will be disconnected.

*/
void QLowEnergyController::disconnectFromService(const QLowEnergyServiceInfo &leService)
{
    d_ptr->disconnectService(leService);
}


/*!
    Enables receiving notifications from the given \a characteristic. If the service characteristic
    does not belong to one of the services or characteristic permissions do not allow notifications,
    the function will return false.
*/
bool QLowEnergyController::enableNotifications(const QLowEnergyCharacteristicInfo &characteristic)
{
    return d_ptr->enableNotification(characteristic);
}

/*!
    Disables receiving notifications from the given \a characteristic. If the service characteristic
    does not belong to one of the services, nothing wil lbe done.

    \sa addLeService
*/
void QLowEnergyController::disableNotifications(const QLowEnergyCharacteristicInfo &characteristic)
{
    d_ptr->disableNotification(characteristic);
}

/*!
    Returns a human-readable description of the last error that occurred.

    \sa error(const QLowEnergyServiceInfo &), error(const QLowEnergyCharacteristicInfo &)
*/
QString QLowEnergyController::errorString() const
{
    return d_ptr->errorString;
}

/*!
    This method is called for the Linux platform if a device has a random device address that
    is used for connecting.
 */
void QLowEnergyController::setRandomAddress()
{
    d_ptr->m_randomAddress = true;
}

/*!
    This method writes the wanted \a characteristic taking its value. This value is written directly
    to the Bluetooth Low Energy device. In case wanted characteristic is not connected or does not
    have write permission, it will return false with the corresponding error string.

    \sa QLowEnergyCharacteristicInfo::setValue(), errorString(), error()
 */
bool QLowEnergyController::writeCharacteristic(const QLowEnergyCharacteristicInfo &characteristic)
{
    return d_ptr->write(characteristic);
}

/*!
    This method writes the wanted \a descriptor taking its value. This value is written directly
    to the Bluetooth Low Energy device. In case wanted descriptor is not connected it will return
    false with the corresponding error string.

    \sa QLowEnergyDescriptorInfo::setValue(), errorString(), error()
 */
bool QLowEnergyController::writeDescriptor(const QLowEnergyDescriptorInfo &descriptor)
{
    return d_ptr->write(descriptor);
}

QT_END_NAMESPACE

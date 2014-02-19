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
#include "moc_qlowenergycontroller.cpp"

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

QLowEnergyControllerPrivate::QLowEnergyControllerPrivate()
{

}

QLowEnergyControllerPrivate::~QLowEnergyControllerPrivate()
{

}

void QLowEnergyControllerPrivate::_q_serviceConnected(const QBluetoothUuid &uuid)
{
    for (int i = 0; i < m_leServices.size(); i++) {
        if (((QLowEnergyServiceInfo)m_leServices.at(i)).serviceUuid() == uuid)
            emit q_ptr->connected((QLowEnergyServiceInfo)m_leServices.at(i));

    }
}

void QLowEnergyControllerPrivate::_q_serviceError(const QBluetoothUuid &uuid)
{
    for (int i = 0; i < m_leServices.size(); i++) {
        if (((QLowEnergyServiceInfo)m_leServices.at(i)).serviceUuid() == uuid) {
            QLowEnergyServiceInfo service((QLowEnergyServiceInfo)m_leServices.at(i));
            errorString = service.d_ptr->errorString;
            emit q_ptr->error(service);
        }
    }
}

void QLowEnergyControllerPrivate::_q_characteristicError(const QBluetoothUuid &uuid)
{
    for (int i = 0; i < m_leServices.size(); i++) {
        QList<QLowEnergyCharacteristicInfo> characteristics = m_leServices.at(i).characteristics();
        for (int j = 0; j < characteristics.size(); j++) {
            if (characteristics.at(j).uuid() == uuid) {
                errorString = characteristics.at(j).d_ptr->errorString;
                emit q_ptr->error(characteristics.at(j));
            }
        }
    }
}


void QLowEnergyControllerPrivate::_q_valueReceived(const QBluetoothUuid &uuid)
{
     for (int i = 0; i < m_leServices.size(); i++) {
         QList<QLowEnergyCharacteristicInfo> characteristics = m_leServices.at(i).characteristics();
         for (int j = 0; j < characteristics.size(); j++) {
             if (characteristics.at(j).uuid() == uuid)
                 emit q_ptr->valueChanged(characteristics.at(j));
         }
     }
}



void QLowEnergyControllerPrivate::_q_serviceDisconnected(const QBluetoothUuid &uuid)
{
    for (int i = 0; i < m_leServices.size(); i++) {
        if (((QLowEnergyServiceInfo)m_leServices.at(i)).serviceUuid() == uuid) {
            QObject::disconnect(((QLowEnergyServiceInfo)m_leServices.at(i)).d_ptr.data(), SIGNAL(connectedToService(QBluetoothUuid)), q_ptr, SLOT(_q_serviceConnected(QBluetoothUuid)));
            QObject::disconnect(((QLowEnergyServiceInfo)m_leServices.at(i)).d_ptr.data(), SIGNAL(error(QBluetoothUuid)), q_ptr, SLOT(_q_serviceError(QBluetoothUuid)));
            QObject::disconnect(((QLowEnergyServiceInfo)m_leServices.at(i)).d_ptr.data(), SIGNAL(disconnectedFromService(QBluetoothUuid)), q_ptr, SLOT(_q_serviceDisconnected(QBluetoothUuid)));
            emit q_ptr->disconnected((QLowEnergyServiceInfo)m_leServices.at(i));
        }
    }
}

void QLowEnergyControllerPrivate::connectService(const QLowEnergyServiceInfo &service)
{
    bool in = false;
    if (service.isValid()) {
        for (int i = 0; i < m_leServices.size(); i++) {
            if (((QLowEnergyServiceInfo)m_leServices.at(i)).serviceUuid() == service.serviceUuid() && !((QLowEnergyServiceInfo)m_leServices.at(i)).isConnected()) {
                in = true;
                QObject::connect(m_leServices.at(i).d_ptr.data(), SIGNAL(connectedToService(QBluetoothUuid)), q_ptr, SLOT(_q_serviceConnected(QBluetoothUuid)));
                QObject::connect(((QLowEnergyServiceInfo)m_leServices.at(i)).d_ptr.data(), SIGNAL(error(QBluetoothUuid)), q_ptr, SLOT(_q_serviceError(QBluetoothUuid)));
                QObject::connect(((QLowEnergyServiceInfo)m_leServices.at(i)).d_ptr.data(), SIGNAL(disconnectedFromService(QBluetoothUuid)), q_ptr, SLOT(_q_serviceDisconnected(QBluetoothUuid)));
                ((QLowEnergyServiceInfo)m_leServices.at(i)).d_ptr->registerServiceWatcher();
                break;
            }
        }
        if (!in) {
            m_leServices.append(service);
            QObject::connect(((QLowEnergyServiceInfo)m_leServices.last()).d_ptr.data(), SIGNAL(connectedToService(QBluetoothUuid)), q_ptr, SLOT(_q_serviceConnected(QBluetoothUuid)));
            QObject::connect(((QLowEnergyServiceInfo)m_leServices.last()).d_ptr.data(), SIGNAL(error(QBluetoothUuid)), q_ptr, SLOT(_q_serviceError(QBluetoothUuid)));
            QObject::connect(((QLowEnergyServiceInfo)m_leServices.last()).d_ptr.data(), SIGNAL(disconnectedFromService(QBluetoothUuid)), q_ptr, SLOT(_q_serviceDisconnected(QBluetoothUuid)));
            ((QLowEnergyServiceInfo)m_leServices.last()).d_ptr->registerServiceWatcher();
        }
    }
}

void QLowEnergyControllerPrivate::disconnectService(const QLowEnergyServiceInfo &service)
{
    if (service.isValid()) {
        for (int i = 0; i < m_leServices.size(); i++) {
            if (((QLowEnergyServiceInfo)m_leServices.at(i)).serviceUuid() == service.serviceUuid() && service.isConnected()) {
                ((QLowEnergyServiceInfo)m_leServices.at(i)).d_ptr->unregisterServiceWatcher();
                break;
            }
        }
    }
    else {
        for (int i = 0; i < m_leServices.size(); i++) {
            if (((QLowEnergyServiceInfo)m_leServices.at(i)).isConnected()) {
                ((QLowEnergyServiceInfo)m_leServices.at(i)).d_ptr->unregisterServiceWatcher();
                break;
            }
        }
    }
}

/*!
    Construct a new QLowEnergyInfo object with the \a parent.
*/
QLowEnergyController::QLowEnergyController(QObject *parent):
    QObject(parent), d_ptr(new QLowEnergyControllerPrivate)
{
    d_ptr->q_ptr = this;
}

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
    bool enable = false;
    for (int i = 0; i < d_ptr->m_leServices.size(); i++) {
        for (int j = 0; j < d_ptr->m_leServices.at(i).characteristics().size(); j++) {
            if (d_ptr->m_leServices.at(i).characteristics().at(j).uuid() == characteristic.uuid()) {
                connect(d_ptr->m_leServices.at(i).characteristics().at(j).d_ptr.data(), SIGNAL(notifyValue(QBluetoothUuid)), this, SLOT(_q_valueReceived(QBluetoothUuid)));
                connect(d_ptr->m_leServices.at(i).characteristics().at(j).d_ptr.data(), SIGNAL(error(QBluetoothUuid)), this, SLOT(_q_characteristicError(QBluetoothUuid)));
                enable = d_ptr->m_leServices.at(i).characteristics().at(j).d_ptr->enableNotification();
            }
        }
    }
    return enable;
}

/*!
    Disables receiving notifications from the given \a characteristic. If the service characteristic
    does not belong to one of the services, nothing wil lbe done.

    \sa addLeService
*/
void QLowEnergyController::disableNotifications(const QLowEnergyCharacteristicInfo &characteristic)
{
    for (int i = 0; i < d_ptr->m_leServices.size(); i++) {
        for (int j = 0; j < d_ptr->m_leServices.at(i).characteristics().size(); j++) {
            if (d_ptr->m_leServices.at(i).characteristics().at(j).uuid() == characteristic.uuid()){
                disconnect(d_ptr->m_leServices.at(i).characteristics().at(j).d_ptr.data(), SIGNAL(notifyValue(QBluetoothUuid)), this, SLOT(_q_valueReceived(QBluetoothUuid)));
                disconnect(d_ptr->m_leServices.at(i).characteristics().at(j).d_ptr.data(), SIGNAL(error(QBluetoothUuid)), this, SLOT(_q_characteristicError(QBluetoothUuid)));
                d_ptr->m_leServices.at(i).characteristics().at(j).d_ptr->disableNotification();
            }
        }
    }
}

/*!
    Returns a human-readable description of the last error that occurred.

    \sa error(const QLowEnergyServiceInfo &), error(const QLowEnergyCharacteristicInfo &)
*/
QString QLowEnergyController::errorString() const
{
    return d_ptr->errorString;
}
QT_END_NAMESPACE

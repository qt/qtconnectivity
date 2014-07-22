/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#include <QtBluetooth/QBluetoothLocalDevice>

#include <algorithm>

QT_BEGIN_NAMESPACE

void QLowEnergyControllerPrivate::setError(
        QLowEnergyController::Error newError)
{
    Q_Q(QLowEnergyController);
    error = newError;

    switch (newError) {
    case QLowEnergyController::UnknownRemoteDeviceError:
        errorString = QLowEnergyController::tr("Remote device cannot be found");
        break;
    case QLowEnergyController::InvalidBluetoothAdapterError:
        errorString = QLowEnergyController::tr("Cannot find local adapter");
        break;
    case QLowEnergyController::NetworkError:
        errorString = QLowEnergyController::tr("Error occurred during connection I/O");
        break;
    case QLowEnergyController::UnknownError:
    default:
        errorString = QLowEnergyController::tr("Unknown Error");
        break;
    }

    emit q->error(newError);
}

bool QLowEnergyControllerPrivate::isValidLocalAdapter()
{
    if (localAdapter.isNull())
        return false;

    const QList<QBluetoothHostInfo> foundAdapters = QBluetoothLocalDevice::allDevices();
    bool adapterFound = false;

    foreach (const QBluetoothHostInfo &info, foundAdapters) {
        if (info.address() == localAdapter) {
            adapterFound = true;
            break;
        }
    }

    return adapterFound;
}

void QLowEnergyControllerPrivate::setState(
        QLowEnergyController::ControllerState newState)
{
    Q_Q(QLowEnergyController);
    if (state == newState)
        return;

    state = newState;
    emit q->stateChanged(state);
}

void QLowEnergyControllerPrivate::invalidateServices()
{
    foreach (const QSharedPointer<QLowEnergyServicePrivate> service, serviceList.values()) {
        service->setController(0);
        service->setState(QLowEnergyService::InvalidService);
    }

    serviceList.clear();
}

QSharedPointer<QLowEnergyServicePrivate> QLowEnergyControllerPrivate::serviceForHandle(
        QLowEnergyHandle handle)
{
    foreach (QSharedPointer<QLowEnergyServicePrivate> service, serviceList.values())
        if (service->startHandle <= handle && handle <= service->endHandle)
            return service;

    return QSharedPointer<QLowEnergyServicePrivate>();
}

/*!
    Returns a valid characteristic if the given handle is the
    handle of the characteristic itself or one of its descriptors
 */
QLowEnergyCharacteristic QLowEnergyControllerPrivate::characteristicForHandle(
        QLowEnergyHandle handle)
{
    QSharedPointer<QLowEnergyServicePrivate> service = serviceForHandle(handle);
    if (service.isNull())
        return QLowEnergyCharacteristic();

    if (service->characteristicList.isEmpty())
        return QLowEnergyCharacteristic();

    // check whether it is the handle of a characteristic header
    if (service->characteristicList.contains(handle))
        return QLowEnergyCharacteristic(service, handle);

    // check whether it is the handle of the characteristic value or its descriptors
    QList<QLowEnergyHandle> charHandles = service->characteristicList.keys();
    std::sort(charHandles.begin(), charHandles.end());
    for (int i = charHandles.size() - 1; i >= 0; i--) {
        if (charHandles.at(i) > handle)
            continue;

        return QLowEnergyCharacteristic(service, charHandles.at(i));
    }

    return QLowEnergyCharacteristic();
}

/*!
    Returns a valid descriptor if \a handle blongs to a descriptor;
    otherwise an invalid one.
 */
QLowEnergyDescriptor QLowEnergyControllerPrivate::descriptorForHandle(
        QLowEnergyHandle handle)
{
    const QLowEnergyCharacteristic matchingChar = characteristicForHandle(handle);
    if (!matchingChar.isValid())
        return QLowEnergyDescriptor();

    const QLowEnergyServicePrivate::CharData charData = matchingChar.
            d_ptr->characteristicList[matchingChar.attributeHandle()];

    if (charData.descriptorList.contains(handle))
        return QLowEnergyDescriptor(matchingChar.d_ptr, matchingChar.attributeHandle(),
                                    handle);

    return QLowEnergyDescriptor();
}

void QLowEnergyControllerPrivate::updateValueOfCharacteristic(
        QLowEnergyHandle charHandle, const QByteArray &value)
{
    QSharedPointer<QLowEnergyServicePrivate> service = serviceForHandle(charHandle);
    if (!service.isNull() && service->characteristicList.contains(charHandle))
        service->characteristicList[charHandle].value = value;
}

void QLowEnergyControllerPrivate::updateValueOfDescriptor(
        QLowEnergyHandle charHandle, QLowEnergyHandle descriptorHandle,
        const QByteArray &value)
{
    QSharedPointer<QLowEnergyServicePrivate> service = serviceForHandle(charHandle);
    if (service.isNull() || !service->characteristicList.contains(charHandle))
        return;

    if (!service->characteristicList[charHandle].descriptorList.contains(descriptorHandle))
        return;

    service->characteristicList[charHandle].descriptorList[descriptorHandle].value = value;
}

QLowEnergyController::QLowEnergyController(
                            const QBluetoothAddress &remoteDevice,
                            QObject *parent)
    : QObject(parent), d_ptr(new QLowEnergyControllerPrivate())
{
    Q_D(QLowEnergyController);
    d->q_ptr = this;
    d->remoteDevice = remoteDevice;
    d->localAdapter = QBluetoothLocalDevice().address();
}

QLowEnergyController::QLowEnergyController(
                            const QBluetoothAddress &remoteDevice,
                            const QBluetoothAddress &localDevice,
                            QObject *parent)
    : QObject(parent), d_ptr(new QLowEnergyControllerPrivate())
{
    Q_D(QLowEnergyController);
    d->q_ptr = this;
    d->remoteDevice = remoteDevice;
    d->localAdapter = localDevice;
}

QLowEnergyController::~QLowEnergyController()
{
    disconnectFromDevice(); //in case we were connected
    delete d_ptr;
}

/*!
  Returns the address of the local Bluetooth adapter being used for the
  communication.

  If this class instance was requested to use the default adapter
  but there was no default adapter when creating this
  class instance, the returned \l QBluetoothAddress will be null.

  \sa QBluetoothAddress::isNull()
 */
QBluetoothAddress QLowEnergyController::localAddress() const
{
    return d_ptr->localAdapter;
}

QBluetoothAddress QLowEnergyController::remoteAddress() const
{
    return d_ptr->remoteDevice;
}

QLowEnergyController::ControllerState QLowEnergyController::state() const
{
    return d_ptr->state;
}

void QLowEnergyController::connectToDevice()
{
    Q_D(QLowEnergyController);

    if (!d->isValidLocalAdapter()) {
        d->setError(QLowEnergyController::InvalidBluetoothAdapterError);
        return;
    }

    if (state() != QLowEnergyController::UnconnectedState)
        return;

    d->connectToDevice();
}

void QLowEnergyController::disconnectFromDevice()
{
    Q_D(QLowEnergyController);

    if (state() == QLowEnergyController::UnconnectedState)
        return;

    d->invalidateServices();
    d->disconnectFromDevice();
}

void QLowEnergyController::discoverServices()
{
    Q_D(QLowEnergyController);

    if (d->state != QLowEnergyController::ConnectedState)
        return;

    d->discoverServices();
}

/*!
    Returns the list of services offered by the remote device.

    The list contains all primary and secondary services.

    \sa createServiceObject()
 */
QList<QBluetoothUuid> QLowEnergyController::services() const
{
    return d_ptr->serviceList.keys();
}

/*!
    Creates an instance of the service represented by \a serviceUuid.
    The \a serviceUuid parameter must have been obtained via
    \l services().

    The caller takes ownership of the returned pointer and may pass
    a \a parent parameter as default owner.

    This function returns a null pointer if no service with
    \a serviceUUid can be found on the remote device.

    This function can return instances for secondary services
    too. The include relationships between services can be expressed
    via \l QLowEnergyService::includedServices().

    \sa services()
 */
QLowEnergyService *QLowEnergyController::createServiceObject(
        const QBluetoothUuid &serviceUuid, QObject *parent)
{
    Q_D(QLowEnergyController);
    if (!d->serviceList.contains(serviceUuid))
        return 0;

    QLowEnergyService *service = new QLowEnergyService(
                d->serviceList.value(serviceUuid), parent);

    return service;
}

QLowEnergyController::Error QLowEnergyController::error() const
{
    return d_ptr->error;
}

QString QLowEnergyController::errorString() const
{
    return d_ptr->errorString;
}

QT_END_NAMESPACE

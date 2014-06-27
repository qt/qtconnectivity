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

#include "qlowenergycontrollernew.h"
#include "qlowenergycontrollernew_p.h"

#include <QtBluetooth/QBluetoothLocalDevice>

QT_BEGIN_NAMESPACE

void QLowEnergyControllerNewPrivate::setError(
        QLowEnergyControllerNew::Error newError)
{
    Q_Q(QLowEnergyControllerNew);
    error = newError;

    switch (newError) {
    case QLowEnergyControllerNew::UnknownRemoteDeviceError:
        errorString = QLowEnergyControllerNew::tr("Remote device cannot be found");
        break;
    case QLowEnergyControllerNew::InvalidBluetoothAdapterError:
        errorString = QLowEnergyControllerNew::tr("Cannot find local adapter");
        break;
    case QLowEnergyControllerNew::NetworkError:
        errorString = QLowEnergyControllerNew::tr("Error occurred during connection I/O");
        break;
    case QLowEnergyControllerNew::UnknownError:
    default:
        errorString = QLowEnergyControllerNew::tr("Unknown Error");
        break;
    }

    emit q->error(newError);
}

bool QLowEnergyControllerNewPrivate::isValidLocalAdapter()
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

void QLowEnergyControllerNewPrivate::setState(
        QLowEnergyControllerNew::ControllerState newState)
{
    Q_Q(QLowEnergyControllerNew);
    if (state == newState)
        return;

    state = newState;
    emit q->stateChanged(state);
}

void QLowEnergyControllerNewPrivate::invalidateServices()
{
    foreach (const QSharedPointer<QLowEnergyServicePrivate> service, serviceList.values()) {
        service->setController(0);
        service->setState(QLowEnergyService::InvalidService);
    }

    serviceList.clear();
}

QSharedPointer<QLowEnergyServicePrivate> QLowEnergyControllerNewPrivate::serviceForHandle(
        QLowEnergyHandle handle)
{
    foreach (QSharedPointer<QLowEnergyServicePrivate> service, serviceList.values())
        if (service->startHandle <= handle && handle <= service->endHandle)
            return service;

    return QSharedPointer<QLowEnergyServicePrivate>();
}

void QLowEnergyControllerNewPrivate::updateValueOfCharacteristic(
        QLowEnergyHandle charHandle, const QByteArray &value)
{
    QSharedPointer<QLowEnergyServicePrivate> service = serviceForHandle(charHandle);
    if (!service.isNull() && service->characteristicList.contains(charHandle))
        service->characteristicList[charHandle].value = value;
}

void QLowEnergyControllerNewPrivate::updateValueOfDescriptor(
        QLowEnergyHandle charHandle, QLowEnergyHandle descriptorHandle,
        const QByteArray &value)
{
    QSharedPointer<QLowEnergyServicePrivate> service = serviceForHandle(charHandle);
    if (service.isNull() || !service->characteristicList.contains(charHandle))
        return;

    QHash<QLowEnergyHandle, QLowEnergyServicePrivate::DescData> descData;
    descData = service->characteristicList[charHandle].descriptorList;

    if (!service->characteristicList[charHandle].descriptorList.contains(descriptorHandle))
        return;

    service->characteristicList[charHandle].descriptorList[descriptorHandle].value = value;
}

QLowEnergyControllerNew::QLowEnergyControllerNew(
                            const QBluetoothAddress &remoteDevice,
                            QObject *parent)
    : QObject(parent), d_ptr(new QLowEnergyControllerNewPrivate())
{
    Q_D(QLowEnergyControllerNew);
    d->q_ptr = this;
    d->remoteDevice = remoteDevice;
    d->localAdapter = QBluetoothLocalDevice().address();
}

QLowEnergyControllerNew::QLowEnergyControllerNew(
                            const QBluetoothAddress &remoteDevice,
                            const QBluetoothAddress &localDevice,
                            QObject *parent)
    : QObject(parent), d_ptr(new QLowEnergyControllerNewPrivate())
{
    Q_D(QLowEnergyControllerNew);
    d->q_ptr = this;
    d->remoteDevice = remoteDevice;
    d->localAdapter = localDevice;
}

QLowEnergyControllerNew::~QLowEnergyControllerNew()
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
QBluetoothAddress QLowEnergyControllerNew::localAddress() const
{
    return d_ptr->localAdapter;
}

QBluetoothAddress QLowEnergyControllerNew::remoteAddress() const
{
    return d_ptr->remoteDevice;
}

QLowEnergyControllerNew::ControllerState QLowEnergyControllerNew::state() const
{
    return d_ptr->state;
}

void QLowEnergyControllerNew::connectToDevice()
{
    Q_D(QLowEnergyControllerNew);

    if (!d->isValidLocalAdapter()) {
        d->setError(QLowEnergyControllerNew::InvalidBluetoothAdapterError);
        return;
    }

    if (state() != QLowEnergyControllerNew::UnconnectedState)
        return;

    d->connectToDevice();
}

void QLowEnergyControllerNew::disconnectFromDevice()
{
    Q_D(QLowEnergyControllerNew);

    if (state() == QLowEnergyControllerNew::UnconnectedState)
        return;

    d->invalidateServices();
    d->disconnectFromDevice();
}

void QLowEnergyControllerNew::discoverServices()
{
    Q_D(QLowEnergyControllerNew);

    if (d->state != QLowEnergyControllerNew::ConnectedState)
        return;

    d->discoverServices();
}

QList<QBluetoothUuid> QLowEnergyControllerNew::services() const
{
    return d_ptr->serviceList.keys();
}

QLowEnergyService *QLowEnergyControllerNew::createServiceObject(
        const QBluetoothUuid &serviceUuid, QObject *parent)
{
    Q_D(QLowEnergyControllerNew);
    if (!d->serviceList.contains(serviceUuid))
        return 0;

    QLowEnergyService *service = new QLowEnergyService(
                d->serviceList.value(serviceUuid), parent);

    return service;
}

QLowEnergyControllerNew::Error QLowEnergyControllerNew::error() const
{
    return d_ptr->error;
}

QString QLowEnergyControllerNew::errorString() const
{
    return d_ptr->errorString;
}

QT_END_NAMESPACE

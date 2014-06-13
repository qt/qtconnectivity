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

void QLowEnergyControllerNewPrivate::validateLocalAdapter()
{
    const QList<QBluetoothHostInfo> foundAdapters = QBluetoothLocalDevice::allDevices();
    bool adapterFound = false;

    if (!localAdapter.isNull()) {
        foreach (const QBluetoothHostInfo &info, foundAdapters) {
            if (info.address() == localAdapter) {
                adapterFound = true;
                break;
            }
        }

        if (!adapterFound)
            setError(QLowEnergyControllerNew::InvalidBluetoothAdapterError);
    } else {
        adapterFound = true;
        if (!foundAdapters.isEmpty())
            localAdapter = foundAdapters[0].address();
    }

    if (localAdapter.isNull() || !adapterFound)
        setError(QLowEnergyControllerNew::InvalidBluetoothAdapterError);
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

QLowEnergyControllerNew::QLowEnergyControllerNew(
                            const QBluetoothAddress &remoteDevice,
                            QObject *parent)
    : QObject(parent), d_ptr(new QLowEnergyControllerNewPrivate())
{
    Q_D(QLowEnergyControllerNew);
    d->q_ptr = this;
    d->remoteDevice = remoteDevice;

    d->validateLocalAdapter();
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

    d->validateLocalAdapter();
}

QLowEnergyControllerNew::~QLowEnergyControllerNew()
{
    delete d_ptr;
}

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

    if (state() != QLowEnergyControllerNew::UnconnectedState)
        return;

    d->connectToDevice();
}

void QLowEnergyControllerNew::disconnectFromDevice()
{
    Q_D(QLowEnergyControllerNew);

    if (state() == QLowEnergyControllerNew::UnconnectedState)
        return;

    d->disconnectFromDevice();
}

void QLowEnergyControllerNew::discoverServices()
{
    Q_D(QLowEnergyControllerNew);

    if (d->state != QLowEnergyControllerNew::ConnectedState)
        return;

    d->serviceList.clear();
    d->discoverServices();
}

QList<QBluetoothUuid> QLowEnergyControllerNew::services() const
{
    return d_ptr->serviceList.keys();
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

/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Copyright (C) 2014 Denis Shienkov <denis.shienkov@gmail.com>
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

#include "qbluetoothlocaldevice.h"
#include "qbluetoothaddress.h"

#include "qbluetoothlocaldevice_p.h"

#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

QBluetoothLocalDevice::QBluetoothLocalDevice(QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothLocalDevicePrivate(this))
{
}

QBluetoothLocalDevice::QBluetoothLocalDevice(
        const QBluetoothAddress &address, QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothLocalDevicePrivate(this, address))
{
}

QString QBluetoothLocalDevice::name() const
{
    Q_D(const QBluetoothLocalDevice);
    return d->deviceName;
}

QBluetoothAddress QBluetoothLocalDevice::address() const
{
    Q_D(const QBluetoothLocalDevice);
    return d->deviceAddress;
}

void QBluetoothLocalDevice::powerOn()
{
    if (hostMode() != HostPoweredOff)
        return;

    setHostMode(HostConnectable);
}

void QBluetoothLocalDevice::setHostMode(
        QBluetoothLocalDevice::HostMode requestedMode)
{
    if (!isValid()) {
        qCWarning(QT_BT_WINDOWS) << "The local device is not initialized correctly";
        return;
    }

    if (requestedMode == HostDiscoverableLimitedInquiry)
        requestedMode = HostDiscoverable;

    if (requestedMode == hostMode())
        return;

    if (requestedMode == QBluetoothLocalDevice::HostPoweredOff) {
        if (::BluetoothIsDiscoverable(NULL)
                && !::BluetoothEnableDiscovery(NULL, FALSE)) {
            qCWarning(QT_BT_WINDOWS) << "Unable to disable the discoverable mode";
            emit error(QBluetoothLocalDevice::UnknownError);
            return;
        }
        if (::BluetoothIsConnectable(NULL)
                && !::BluetoothEnableIncomingConnections(NULL, FALSE)) {
            qCWarning(QT_BT_WINDOWS) << "Unable to disable the connectable mode";
            emit error(QBluetoothLocalDevice::UnknownError);
            return;
        }
    } else if (requestedMode == QBluetoothLocalDevice::HostConnectable) {
        if (::BluetoothIsDiscoverable(NULL)) {
            if (!::BluetoothEnableDiscovery(NULL, FALSE)) {
                qCWarning(QT_BT_WINDOWS) << "Unable to disable the discoverable mode";
                emit error(QBluetoothLocalDevice::UnknownError);
                return;
            }
        } else if (!::BluetoothEnableIncomingConnections(NULL, TRUE)) {
            qCWarning(QT_BT_WINDOWS) << "Unable to enable the connectable mode";
            emit error(QBluetoothLocalDevice::UnknownError);
            return;
        }
    } else if (requestedMode == QBluetoothLocalDevice::HostDiscoverable
               || requestedMode == QBluetoothLocalDevice::HostDiscoverableLimitedInquiry) {
        if (!::BluetoothIsConnectable(NULL)
                && !::BluetoothEnableIncomingConnections(NULL, TRUE)) {
            qCWarning(QT_BT_WINDOWS) << "Unable to enable the connectable mode";
            emit error(QBluetoothLocalDevice::UnknownError);
            return;
        }
        if (!::BluetoothEnableDiscovery(NULL, TRUE)) {
            qCWarning(QT_BT_WINDOWS) << "Unable to enable the discoverable mode";
            emit error(QBluetoothLocalDevice::UnknownError);
            return;
        }
    }

    emit hostModeStateChanged(requestedMode);
}

QBluetoothLocalDevice::HostMode QBluetoothLocalDevice::hostMode() const
{
    if (!isValid()) {
        qCWarning(QT_BT_WINDOWS) << "The local device is not initialized correctly";
        return HostPoweredOff;
    }

    if (::BluetoothIsDiscoverable(NULL))
        return HostDiscoverable;
    if (::BluetoothIsConnectable(NULL))
        return HostConnectable;
    return HostPoweredOff;
}

QList<QBluetoothAddress> QBluetoothLocalDevice::connectedDevices() const
{
    return QList<QBluetoothAddress>();
}

QList<QBluetoothHostInfo> QBluetoothLocalDevice::allDevices()
{
    const WinClassicBluetooth::LocalRadiosDiscoveryResult result =
            WinClassicBluetooth::enumerateLocalRadios();

    QList<QBluetoothHostInfo> devices;
    foreach (const BLUETOOTH_RADIO_INFO &radio, result.radios) {
        QBluetoothHostInfo device;
        device.setAddress(QBluetoothAddress(radio.address.ullLong));
        device.setName(QString::fromWCharArray(radio.szName));
        devices.append(device);
    }
    return devices;
}

void QBluetoothLocalDevice::requestPairing(const QBluetoothAddress &address, Pairing pairing)
{
    Q_UNUSED(address);
    Q_UNUSED(pairing);
}

QBluetoothLocalDevice::Pairing QBluetoothLocalDevice::pairingStatus(
    const QBluetoothAddress &address) const
{
    Q_UNUSED(address);
    return Unpaired;
}

void QBluetoothLocalDevice::pairingConfirmation(bool confirmation)
{
    Q_UNUSED(confirmation);
}

QBluetoothLocalDevicePrivate::QBluetoothLocalDevicePrivate(
        QBluetoothLocalDevice *q, const QBluetoothAddress &address)
    : deviceValid(false)
    , q_ptr(q)
{
    initialize(address);
}

QBluetoothLocalDevicePrivate::~QBluetoothLocalDevicePrivate()
{
}

bool QBluetoothLocalDevicePrivate::isValid() const
{
    return deviceValid;
}

void QBluetoothLocalDevicePrivate::initialize(const QBluetoothAddress &address)
{
    Q_Q(QBluetoothLocalDevice);

    const WinClassicBluetooth::LocalRadiosDiscoveryResult result =
            WinClassicBluetooth::enumerateLocalRadios();

    if (result.error != NO_ERROR
            && result.error != ERROR_NO_MORE_ITEMS) {
        qCWarning(QT_BT_WINDOWS) << qt_error_string(result.error);
        QMetaObject::invokeMethod(q, "error", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothLocalDevice::Error,
                                        QBluetoothLocalDevice::UnknownError));
        return;
    } else if (result.radios.isEmpty()) {
        qCWarning(QT_BT_WINDOWS) << "No any classic local radio found";
        QMetaObject::invokeMethod(q, "error", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothLocalDevice::Error,
                                        QBluetoothLocalDevice::UnknownError));
        return;
    }

    foreach (const BLUETOOTH_RADIO_INFO &radio, result.radios) {
        if (address == QBluetoothAddress()
             || address == QBluetoothAddress(radio.address.ullLong)) {
            deviceAddress = QBluetoothAddress(radio.address.ullLong);
            deviceName = QString::fromWCharArray(radio.szName);
            deviceValid = true;
            return;
        }
    }

    qCWarning(QT_BT_WINDOWS) << "Unable to find classic local radio: " << address;
    QMetaObject::invokeMethod(q, "error", Qt::QueuedConnection,
                              Q_ARG(QBluetoothLocalDevice::Error,
                                    QBluetoothLocalDevice::UnknownError));
}

QT_END_NAMESPACE

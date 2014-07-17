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

#include <bluetoothapis.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

QBluetoothLocalDevice::QBluetoothLocalDevice(QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothLocalDevicePrivate(this, QBluetoothAddress()))
{
}

QBluetoothLocalDevice::QBluetoothLocalDevice(const QBluetoothAddress &address, QObject *parent) :
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

void QBluetoothLocalDevice::setHostMode(QBluetoothLocalDevice::HostMode requestedMode)
{
    Q_D(QBluetoothLocalDevice);

    if (!isValid()) {
        qCWarning(QT_BT_WINDOWS) << "The local device is not initialized correctly";
        return;
    }

    if (requestedMode == HostDiscoverableLimitedInquiry)
        requestedMode = HostDiscoverable;

    if (requestedMode == hostMode())
        return;

    if (requestedMode == QBluetoothLocalDevice::HostPoweredOff) {
        if (::BluetoothIsDiscoverable(d->deviceHandle)
                && !::BluetoothEnableDiscovery(d->deviceHandle, FALSE)) {
            qCWarning(QT_BT_WINDOWS) << "Unable to disable the discoverable mode";
            emit error(QBluetoothLocalDevice::UnknownError);
            return;
        }
        if (::BluetoothIsConnectable(d->deviceHandle)
                && !::BluetoothEnableIncomingConnections(d->deviceHandle, FALSE)) {
            qCWarning(QT_BT_WINDOWS) << "Unable to disable the connectable mode";
            emit error(QBluetoothLocalDevice::UnknownError);
            return;
        }
    } else if (requestedMode == QBluetoothLocalDevice::HostConnectable) {
        if (::BluetoothIsDiscoverable(d->deviceHandle)) {
            if (!::BluetoothEnableDiscovery(d->deviceHandle, FALSE)) {
                qCWarning(QT_BT_WINDOWS) << "Unable to disable the discoverable mode";
                emit error(QBluetoothLocalDevice::UnknownError);
                return;
            }
        } else if (!::BluetoothEnableIncomingConnections(d->deviceHandle, TRUE)) {
            qCWarning(QT_BT_WINDOWS) << "Unable to enable the connectable mode";
            emit error(QBluetoothLocalDevice::UnknownError);
            return;
        }
    } else if (requestedMode == QBluetoothLocalDevice::HostDiscoverable
               || requestedMode == QBluetoothLocalDevice::HostDiscoverableLimitedInquiry) {
        if (!::BluetoothIsConnectable(d->deviceHandle)
                && !::BluetoothEnableIncomingConnections(d->deviceHandle, TRUE)) {
            qCWarning(QT_BT_WINDOWS) << "Unable to enable the connectable mode";
            emit error(QBluetoothLocalDevice::UnknownError);
            return;
        }
        if (!::BluetoothEnableDiscovery(d->deviceHandle, TRUE)) {
            qCWarning(QT_BT_WINDOWS) << "Unable to enable the discoverable mode";
            emit error(QBluetoothLocalDevice::UnknownError);
            return;
        }
    }

    emit hostModeStateChanged(requestedMode);
}

QBluetoothLocalDevice::HostMode QBluetoothLocalDevice::hostMode() const
{
    Q_D(const QBluetoothLocalDevice);

    if (!isValid()) {
        qCWarning(QT_BT_WINDOWS) << "The local device is not initialized correctly";
        return HostPoweredOff;
    }

    if (::BluetoothIsDiscoverable(d->deviceHandle))
        return HostDiscoverable;
    if (::BluetoothIsConnectable(d->deviceHandle))
        return HostConnectable;
    return HostPoweredOff;
}

QList<QBluetoothAddress> QBluetoothLocalDevice::connectedDevices() const
{
    return QList<QBluetoothAddress>();
}

QList<QBluetoothHostInfo> QBluetoothLocalDevice::allDevices()
{
    BLUETOOTH_FIND_RADIO_PARAMS findRadioParams;
    ::ZeroMemory(&findRadioParams, sizeof(findRadioParams));
    findRadioParams.dwSize = sizeof(findRadioParams);

    HANDLE radioHandle = NULL;
    const HBLUETOOTH_RADIO_FIND radioFindHandle = ::BluetoothFindFirstRadio(&findRadioParams,
                                                                            &radioHandle);
    if (!radioFindHandle)
        return QList<QBluetoothHostInfo>();

    QList<QBluetoothHostInfo> localDevices;

    forever {
        BLUETOOTH_RADIO_INFO radioInfo;
        ::ZeroMemory(&radioInfo, sizeof(radioInfo));
        radioInfo.dwSize = sizeof(radioInfo);

        const DWORD retval = ::BluetoothGetRadioInfo(radioHandle, &radioInfo);
        ::CloseHandle(radioHandle);

        if (retval != ERROR_SUCCESS)
            break;

        QBluetoothHostInfo localDevice;
        localDevice.setAddress(QBluetoothAddress(radioInfo.address.ullLong));
        localDevice.setName(QString::fromWCharArray(radioInfo.szName));
        localDevices.append(localDevice);

        if (!::BluetoothFindNextRadio(radioFindHandle, &radioHandle))
            break;
    }

    ::BluetoothFindRadioClose(radioFindHandle);
    return localDevices;
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

QBluetoothLocalDevicePrivate::QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *q,
                                                           const QBluetoothAddress &address)
    : q_ptr(q)
    , deviceHandle(NULL)
{
    initialize(address);
}


QBluetoothLocalDevicePrivate::~QBluetoothLocalDevicePrivate()
{
    if (isValid())
        ::CloseHandle(deviceHandle);
}

void QBluetoothLocalDevicePrivate::initialize(const QBluetoothAddress &address)
{
    BLUETOOTH_FIND_RADIO_PARAMS findRadioParams;
    ::ZeroMemory(&findRadioParams, sizeof(findRadioParams));
    findRadioParams.dwSize = sizeof(findRadioParams);

    HANDLE radioHandle = NULL;
    const HBLUETOOTH_RADIO_FIND radioFindHandle = ::BluetoothFindFirstRadio(&findRadioParams,
                                                                            &radioHandle);
    if (!radioFindHandle) {
        qCWarning(QT_BT_WINDOWS) << qt_error_string(::GetLastError());
        return;
    }

    forever {
        BLUETOOTH_RADIO_INFO radioInfo;
        ::ZeroMemory(&radioInfo, sizeof(radioInfo));
        radioInfo.dwSize = sizeof(radioInfo);

        const DWORD retval = ::BluetoothGetRadioInfo(radioHandle, &radioInfo);
        if (retval != ERROR_SUCCESS) {
            ::CloseHandle(radioHandle);
            qCWarning(QT_BT_WINDOWS) << qt_error_string(retval);
            break;
        }

        if (address.isNull() || (address == QBluetoothAddress(radioInfo.address.ullLong))) {
            deviceAddress = QBluetoothAddress(radioInfo.address.ullLong);
            deviceName = QString::fromWCharArray(radioInfo.szName);
            deviceHandle = radioHandle;
            break;
        }

        ::CloseHandle(radioHandle);

        if (!::BluetoothFindNextRadio(radioFindHandle, &radioHandle)) {
            const DWORD nativeError = ::GetLastError();
            if (nativeError != ERROR_NO_MORE_ITEMS)
                qCWarning(QT_BT_WINDOWS) << qt_error_string(nativeError);
            break;
        }
    }

    if (!::BluetoothFindRadioClose(radioFindHandle))
        qCWarning(QT_BT_WINDOWS) << qt_error_string(::GetLastError());
}

bool QBluetoothLocalDevicePrivate::isValid() const
{
    return deviceHandle && (deviceHandle != INVALID_HANDLE_VALUE);
}

QT_END_NAMESPACE

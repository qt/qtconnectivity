/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#include <private/qbluetoothutils_winrt_p.h>

#include "qbluetoothlocaldevice.h"
#include "qbluetoothaddress.h"

#include "qbluetoothlocaldevice_p.h"
#include "qbluetoothutils_winrt_p.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <wrl.h>

#include <QtCore/private/qfunctions_winrt_p.h>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Devices::Enumeration;
using namespace winrt::Windows::Devices::Bluetooth;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

QT_BEGIN_NAMESPACE

QBluetoothLocalDevice::QBluetoothLocalDevice(QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothLocalDevicePrivate(this, QBluetoothAddress()))
{
    registerQBluetoothLocalDeviceMetaType();
}

QBluetoothLocalDevice::QBluetoothLocalDevice(const QBluetoothAddress &address, QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothLocalDevicePrivate(this, address))
{
    registerQBluetoothLocalDeviceMetaType();
}

QBluetoothLocalDevicePrivate::QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *q, QBluetoothAddress)
    : q_ptr(q)
{
}

QBluetoothLocalDevicePrivate::~QBluetoothLocalDevicePrivate() = default;

bool QBluetoothLocalDevicePrivate::isValid() const
{
    return true;
}

void QBluetoothLocalDevice::requestPairing(const QBluetoothAddress &address, Pairing pairing)
{
    Q_UNUSED(address);
    Q_UNUSED(pairing);
    QMetaObject::invokeMethod(this, "errorOccurred", Qt::QueuedConnection,
                              Q_ARG(QBluetoothLocalDevice::Error,
                                    QBluetoothLocalDevice::PairingError));
}

QBluetoothLocalDevice::Pairing QBluetoothLocalDevice::pairingStatus(
    const QBluetoothAddress &address) const
{
    if (!isValid() || address.isNull())
        return Unpaired;

    auto qtPairingFromPairingInfo = [](DeviceInformationPairing pairingInfo) {
        if (!pairingInfo.IsPaired())
            return Unpaired;

        const DevicePairingProtectionLevel protection = pairingInfo.ProtectionLevel();
        if (protection == DevicePairingProtectionLevel::Encryption
                || protection == DevicePairingProtectionLevel::EncryptionAndAuthentication)
            return AuthorizedPaired;
        return Paired;
    };

    const quint64 addr64 = address.toUInt64();
    BluetoothLEDevice leDevice(nullptr);
    bool res = await(BluetoothLEDevice::FromBluetoothAddressAsync(addr64), leDevice, 5000);
    if (res && leDevice && leDevice.BluetoothAddress() != 0)
        return qtPairingFromPairingInfo(leDevice.DeviceInformation().Pairing());

    BluetoothDevice device(nullptr);
    res = await(BluetoothDevice::FromBluetoothAddressAsync(addr64), device, 5000);
    if (res && device && device.BluetoothAddress() != 0)
        return qtPairingFromPairingInfo(device.DeviceInformation().Pairing());

    return Unpaired;
}

void QBluetoothLocalDevice::setHostMode(QBluetoothLocalDevice::HostMode mode)
{
    Q_UNUSED(mode);
}

QBluetoothLocalDevice::HostMode QBluetoothLocalDevice::hostMode() const
{
    return HostConnectable;
}

QList<QBluetoothAddress> QBluetoothLocalDevice::connectedDevices() const
{
    return QList<QBluetoothAddress>();
}

void QBluetoothLocalDevice::powerOn()
{
}

QString QBluetoothLocalDevice::name() const
{
    return QString();
}

QBluetoothAddress QBluetoothLocalDevice::address() const
{
    return QBluetoothAddress();
}

QList<QBluetoothHostInfo> QBluetoothLocalDevice::allDevices()
{
    QList<QBluetoothHostInfo> localDevices;
    return localDevices;
}

QT_END_NAMESPACE

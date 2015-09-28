/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2014 Denis Shienkov <denis.shienkov@gmail.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothdevicediscoveryagent_p.h"
#include "qbluetoothaddress.h"
#include "qbluetoothuuid.h"
#include "qbluetoothlocaldevice_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(
        const QBluetoothAddress &deviceAdapter,
        QBluetoothDeviceDiscoveryAgent *parent)
    : inquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry)
    , lastError(QBluetoothDeviceDiscoveryAgent::NoError)
    , classicDiscoveryWatcher(0)
    , lowEnergyDiscoveryWatcher(0)
    , pendingCancel(false)
    , pendingStart(false)
    , isClassicActive(false)
    , isClassicValid(false)
    , isLowEnergyActive(false)
    , isLowEnergyValid(false)
    , q_ptr(parent)
{
    initialize(deviceAdapter);
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
    if (isClassicActive || isLowEnergyActive)
        stop();

    if (classicDiscoveryWatcher)
        classicDiscoveryWatcher->waitForFinished();

    if (lowEnergyDiscoveryWatcher)
        lowEnergyDiscoveryWatcher->waitForFinished();
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    if (pendingStart)
        return true;
    if (pendingCancel)
        return false;

    return isClassicActive || isLowEnergyActive;
}

void QBluetoothDeviceDiscoveryAgentPrivate::start()
{
    if (!isClassicValid && !isLowEnergyValid) {
        setError(ERROR_INVALID_HANDLE,
                 QBluetoothDeviceDiscoveryAgent::tr("Passed address is not a local device."));
        return;
    }

    if (pendingCancel == true) {
        pendingStart = true;
        return;
    }

    discoveredDevices.clear();

    if (isClassicValid)
        startDiscoveryForFirstClassicDevice();

    if (isLowEnergyValid)
        startDiscoveryForLowEnergyDevices();
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    if (!isClassicActive && !isLowEnergyActive)
        return;

    pendingCancel = true;
    pendingStart = false;
}

void QBluetoothDeviceDiscoveryAgentPrivate::classicDeviceDiscovered()
{
    const WinClassicBluetooth::RemoteDeviceDiscoveryResult result =
            classicDiscoveryWatcher->result();

    if (isDiscoveredSuccessfully(result.error)) {

        if (canBeCanceled()) {
            cancel();
        } else if (canBePendingStarted()) {
            prepareToPendingStart();
        } else {
            if (result.error != ERROR_NO_MORE_ITEMS) {
                acceptDiscoveredClassicDevice(result.device);
                startDiscoveryForNextClassicDevice(result.hSearch);
                return;
            }
        }

    } else {
        drop(result.error);
    }

    completeClassicDiscovery(result.hSearch);
}

void QBluetoothDeviceDiscoveryAgentPrivate::lowEnergyDeviceDiscovered()
{
    const WinLowEnergyBluetooth::DeviceDiscoveryResult result =
            lowEnergyDiscoveryWatcher->result();

    if (isDiscoveredSuccessfully(result.error)) {

        if (canBeCanceled()) {
            cancel();
        } else if (canBePendingStarted()) {
            prepareToPendingStart();
        } else {
            foreach (const WinLowEnergyBluetooth::DeviceInfo &deviceInfo, result.devices)
                acceptDiscoveredLowEnergyDevice(deviceInfo);
        }

    } else {
        drop(result.error);
    }

    completeLowEnergyDiscovery();
}

void QBluetoothDeviceDiscoveryAgentPrivate::initialize(
        const QBluetoothAddress &deviceAdapter)
{
    isClassicValid = isClassicAdapterValid(deviceAdapter);
    isLowEnergyValid = isLowEnergyAdapterValid(deviceAdapter);
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isClassicAdapterValid(
        const QBluetoothAddress &deviceAdapter)
{
    foreach (const QBluetoothHostInfo &adapterInfo, QBluetoothLocalDevicePrivate::localAdapters()) {
        if (deviceAdapter == QBluetoothAddress()
                || deviceAdapter == adapterInfo.address()) {
            return true;
        }
    }

    qCWarning(QT_BT_WINDOWS) << "No matching for classic local radio:" << deviceAdapter;
    return false;
}

void QBluetoothDeviceDiscoveryAgentPrivate::startDiscoveryForFirstClassicDevice()
{
    isClassicActive = true;

    if (!classicDiscoveryWatcher) {
        classicDiscoveryWatcher = new QFutureWatcher<
                WinClassicBluetooth::RemoteDeviceDiscoveryResult>(this);
        connect(classicDiscoveryWatcher, &QFutureWatcher<WinClassicBluetooth::RemoteDeviceDiscoveryResult>::finished,
                this, &QBluetoothDeviceDiscoveryAgentPrivate::classicDeviceDiscovered);
    }

    const QFuture<WinClassicBluetooth::RemoteDeviceDiscoveryResult> future =
            QtConcurrent::run(WinClassicBluetooth::startDiscoveryOfFirstRemoteDevice);
    classicDiscoveryWatcher->setFuture(future);
}

void QBluetoothDeviceDiscoveryAgentPrivate::startDiscoveryForNextClassicDevice(
        HBLUETOOTH_DEVICE_FIND hSearch)
{
    Q_ASSERT(classicDiscoveryWatcher);

    const QFuture<WinClassicBluetooth::RemoteDeviceDiscoveryResult> future =
            QtConcurrent::run(WinClassicBluetooth::startDiscoveryOfNextRemoteDevice, hSearch);
    classicDiscoveryWatcher->setFuture(future);
}

void QBluetoothDeviceDiscoveryAgentPrivate::completeClassicDiscovery(
        HBLUETOOTH_DEVICE_FIND hSearch)
{
    WinClassicBluetooth::cancelRemoteDevicesDiscovery(hSearch);
    isClassicActive = false;
    finalize();
}

void QBluetoothDeviceDiscoveryAgentPrivate::acceptDiscoveredClassicDevice(
        const BLUETOOTH_DEVICE_INFO &device)
{
    QBluetoothDeviceInfo deviceInfo(
                QBluetoothAddress(device.Address.ullLong),
                QString::fromWCharArray(device.szName),
                device.ulClassofDevice);

    if (device.fRemembered)
        deviceInfo.setCached(true);

    processDuplicates(deviceInfo);
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isLowEnergyAdapterValid(
        const QBluetoothAddress &deviceAdapter)
{
    Q_UNUSED(deviceAdapter);

    // We can not detect an address of local BLE adapter,
    // but we can detect that some BLE adapter is present.
    return WinLowEnergyBluetooth::hasLocalRadio();
}

void QBluetoothDeviceDiscoveryAgentPrivate::startDiscoveryForLowEnergyDevices()
{
    isLowEnergyActive = true;

    if (!lowEnergyDiscoveryWatcher) {
        lowEnergyDiscoveryWatcher = new QFutureWatcher<
                WinLowEnergyBluetooth::DeviceDiscoveryResult>(this);
        connect(lowEnergyDiscoveryWatcher, &QFutureWatcher<WinLowEnergyBluetooth::DeviceDiscoveryResult>::finished,
                this, &QBluetoothDeviceDiscoveryAgentPrivate::lowEnergyDeviceDiscovered);
    }

    const QFuture<WinLowEnergyBluetooth::DeviceDiscoveryResult> future =
            QtConcurrent::run(WinLowEnergyBluetooth::startDiscoveryOfRemoteDevices);
    lowEnergyDiscoveryWatcher->setFuture(future);
}

void QBluetoothDeviceDiscoveryAgentPrivate::completeLowEnergyDiscovery()
{
    isLowEnergyActive = false;
    finalize();
}

void QBluetoothDeviceDiscoveryAgentPrivate::acceptDiscoveredLowEnergyDevice(
        const WinLowEnergyBluetooth::DeviceInfo &device)
{
    QBluetoothDeviceInfo deviceInfo(device.address, device.name, 0);
    deviceInfo.setCoreConfigurations(QBluetoothDeviceInfo::LowEnergyCoreConfiguration);
    deviceInfo.setCached(true);

    processDuplicates(deviceInfo);
}

void QBluetoothDeviceDiscoveryAgentPrivate::processDuplicates(
        const QBluetoothDeviceInfo &foundDevice)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    for (int i = 0; i < discoveredDevices.size(); i++) {
        QBluetoothDeviceInfo mergedDevice = discoveredDevices[i];
        if (mergedDevice.address() == foundDevice.address()) {
            if (mergedDevice == foundDevice
                    || mergedDevice.coreConfigurations() == foundDevice.coreConfigurations()) {
                qCDebug(QT_BT_WINDOWS) << "Duplicate: " << foundDevice.address();
                return;
            }

            // We assume that if the existing device it is low energy, it means that
            // the found device should be as classic, because it is impossible to get
            // same low energy device.
            if (mergedDevice.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
                mergedDevice = foundDevice;

            // We assume that it is impossible to have multiple devices with same core
            // configurations, which have one address. This possible only in case a device
            // provided both low energy and classic features at the same time.
            mergedDevice.setCoreConfigurations(QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration);
            mergedDevice.setCached(foundDevice.isCached());

            discoveredDevices.replace(i, mergedDevice);
            Q_Q(QBluetoothDeviceDiscoveryAgent);
            qCDebug(QT_BT_WINDOWS) << "Updated: " << mergedDevice.address();

            emit q->deviceDiscovered(mergedDevice);
            return;
        }
    }

    qCDebug(QT_BT_WINDOWS) << "Emit: " << foundDevice.address();
    discoveredDevices.append(foundDevice);
    emit q->deviceDiscovered(foundDevice);
}

void QBluetoothDeviceDiscoveryAgentPrivate::setError(DWORD error, const QString &str)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    lastError = (error == ERROR_INVALID_HANDLE) ?
                QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError
              : QBluetoothDeviceDiscoveryAgent::InputOutputError;
    errorString = str.isEmpty() ? qt_error_string(error) : str;
    emit q->error(lastError);
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isDiscoveredSuccessfully(
        int systemError) const
{
    return systemError == NO_ERROR || systemError == ERROR_NO_MORE_ITEMS;
}

bool QBluetoothDeviceDiscoveryAgentPrivate::canBeCanceled() const
{
    if (isClassicActive || isLowEnergyActive)
        return false;
    return pendingCancel && !pendingStart;
}

void QBluetoothDeviceDiscoveryAgentPrivate::cancel()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    emit q->canceled();
    pendingCancel = false;
}

bool QBluetoothDeviceDiscoveryAgentPrivate::canBePendingStarted() const
{
    if (isClassicActive || isLowEnergyActive)
        return false;
    return pendingStart;
}

void QBluetoothDeviceDiscoveryAgentPrivate::prepareToPendingStart()
{
    pendingCancel = false;
    pendingStart = false;
    start();
}

void QBluetoothDeviceDiscoveryAgentPrivate::finalize()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    if (isClassicActive || isLowEnergyActive)
        return;

    emit q->finished();
}

void QBluetoothDeviceDiscoveryAgentPrivate::drop(int systemError)
{
    setError(systemError);
    pendingCancel = false;
    pendingStart = false;
}

QT_END_NAMESPACE

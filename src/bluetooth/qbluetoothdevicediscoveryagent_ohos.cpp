// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothdevicediscoveryagent_p.h"

#include <QtBluetooth/qbluetoothaddress.h>
#include <QtBluetooth/qbluetoothdevicediscoveryagent.h>
#include <QtBluetooth/private/qohosbluetoothaccess_p.h>

#include <QtCore/qloggingcategory.h>

#include <algorithm>
#include <optional>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_OHOS)

namespace {

std::optional<QBluetoothDeviceInfo> tryMakeDeviceInfo(
    const QtOhosBluetooth::DiscoveryResult &device)
{
    const QBluetoothAddress address(QString::fromStdString(device.deviceId));
    if (address.isNull()) {
        qCWarning(
            QT_BT_OHOS, "%s: ignoring the device with an unexpected address: '%s'", Q_FUNC_INFO,
            device.deviceId.c_str());
        return std::nullopt;
    }

    QBluetoothDeviceInfo deviceInfo(
        address, QString::fromStdString(device.deviceName), device.classOfDevice);
    deviceInfo.setCoreConfigurations(QBluetoothDeviceInfo::BaseRateCoreConfiguration);

    return deviceInfo;
}

std::vector<QBluetoothDeviceInfo> processFoundDevices(
    const std::vector<QtOhosBluetooth::DiscoveryResult> &discoveredDevices)
{
    std::vector<QBluetoothDeviceInfo> foundDevicesInfos;
    foundDevicesInfos.reserve(discoveredDevices.size());

    for (const auto &discoveredDevice : discoveredDevices) {
        auto optDeviceInfo = tryMakeDeviceInfo(discoveredDevice);
        if (!optDeviceInfo)
            continue;

        optDeviceInfo->setRssi(discoveredDevice.rssi);

        foundDevicesInfos.push_back(std::move(*optDeviceInfo));
    }

    return foundDevicesInfos;
}

std::vector<QBluetoothDeviceInfo> processPairedDevices(
    const std::vector<QtOhosBluetooth::DiscoveryResult> &pairedDevices)
{
    std::vector<QBluetoothDeviceInfo> pairedDevicesInfos;
    pairedDevicesInfos.reserve(pairedDevices.size());

    for (const auto &pairedDevice : pairedDevices) {
        auto optDeviceInfo = tryMakeDeviceInfo(pairedDevice);
        if (!optDeviceInfo)
            continue;

        optDeviceInfo->setCached(true);

        pairedDevicesInfos.push_back(std::move(*optDeviceInfo));
    }

    return pairedDevicesInfos;
}

QBluetoothDeviceInfo::Fields updateDeviceInfoIfNeeded(
    QBluetoothDeviceInfo &existingDevice, const QBluetoothDeviceInfo &newDevice)
{
    QBluetoothDeviceInfo::Fields updatedFields = QBluetoothDeviceInfo::Field::None;

    if (!newDevice.isCached() && existingDevice.rssi() != newDevice.rssi()) {
        existingDevice.setRssi(newDevice.rssi());
        updatedFields.setFlag(QBluetoothDeviceInfo::Field::RSSI);
    }

    if (existingDevice.isCached() && !newDevice.isCached())
        existingDevice.setCached(false);

    if (existingDevice.name().isEmpty() && !newDevice.name().isEmpty())
        existingDevice.setName(newDevice.name());

    return updatedFields;
}

}

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(
    const QBluetoothAddress &deviceAdapter, QBluetoothDeviceDiscoveryAgent *parent)
    : lowEnergySearchTimeout(-1)
    , q_ptr(parent)
{
    if (!deviceAdapter.isNull()) {
        qCWarning(
            QT_BT_OHOS,
            "%s: HarmonyOS does not support device adapter setting. Ignoring device adapter.",
            Q_FUNC_INFO);
        lastError = QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Device adapter setting not supported");
        emitAsyncError();
        return;
    }

    m_discoveryAgentProxy = QtOhosBluetooth::QOhosBluetoothDeviceDiscoveryAgentProxy::instance();

    QObject::connect(
        m_discoveryAgentProxy.get(), &QtOhosBluetooth::QOhosBluetoothDeviceDiscoveryAgentProxy::discoveryStopped,
        this,
        [this]() {
            reportDiscoveryStopped(DiscoveryStopReason::Finished);
        });

    QObject::connect(
        m_discoveryAgentProxy.get(), &QtOhosBluetooth::QOhosBluetoothDeviceDiscoveryAgentProxy::discoveryStartFailed,
        this,
        [this]() {
            if (!m_discoveryRequested)
                return;

            clearDiscoveryState();
            lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
            errorString = QBluetoothDeviceDiscoveryAgent::tr("Unable to start the device discovery");
            emitAsyncError();
        });

    QObject::connect(
        m_discoveryAgentProxy.get(), &QtOhosBluetooth::QOhosBluetoothDeviceDiscoveryAgentProxy::discoveryStopFailed,
        this,
        [this]() {
            if (!m_discoveryRequested)
                return;

            clearDiscoveryState();
            lastError = QBluetoothDeviceDiscoveryAgent::UnknownError;
            errorString = QBluetoothDeviceDiscoveryAgent::tr("Timed out waiting for device discovery to stop");
            emitAsyncError();
        });

    QObject::connect(
        m_discoveryAgentProxy.get(), &QtOhosBluetooth::QOhosBluetoothDeviceDiscoveryAgentProxy::bluetoothPoweredOff,
        this,
        [this]() {
            reportDiscoveryStopped(DiscoveryStopReason::BluetoothPoweredOff);
        });

    QObject::connect(
        m_discoveryAgentProxy.get(), &QtOhosBluetooth::QOhosBluetoothDeviceDiscoveryAgentProxy::missingPermission,
        this,
        [this]() {
            if (!m_discoveryRequested)
                return;

            clearDiscoveryState();
            lastError = QBluetoothDeviceDiscoveryAgent::MissingPermissionsError;
            errorString = QBluetoothDeviceDiscoveryAgent::tr("Missing permissions for device discovery");
            emitAsyncError();
        });

    QObject::connect(
        m_discoveryAgentProxy.get(), &QtOhosBluetooth::QOhosBluetoothDeviceDiscoveryAgentProxy::bluetoothDevicesFound,
        this,
        [this](const std::vector<QtOhosBluetooth::DiscoveryResult> &alreadyDiscoveredDevices) {
            if (!m_discoveryRequested)
                return;

            reportDiscoveredDevices(processFoundDevices(alreadyDiscoveredDevices));
        });
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
    if (m_discoveryRequested && m_discoveryAgentProxy)
        m_discoveryAgentProxy->stopBluetoothDiscovery();
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    if (m_pendingStart)
        return true;

    if (m_pendingCancel)
        return false;

    return m_discoveryRequested;
}

QBluetoothDeviceDiscoveryAgent::DiscoveryMethods QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods()
{
    return QBluetoothDeviceDiscoveryAgent::ClassicMethod;
}

void QBluetoothDeviceDiscoveryAgentPrivate::start(QBluetoothDeviceDiscoveryAgent::DiscoveryMethods methods)
{
    requestedMethods = methods;

    if (!m_discoveryAgentProxy) {
        lastError = QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Device adapter setting not supported");
        clearDiscoveryState();
        emitAsyncError();
        return;
    }

    auto accessProxy = QtOhosBluetooth::QOhosBluetoothAccessProxy::instance();
    if (accessProxy->tryGetBluetoothState() != QtOhosBluetooth::QOhosBluetoothAccessProxy::BluetoothState::STATE_ON) {
        lastError = QBluetoothDeviceDiscoveryAgent::PoweredOffError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Local bluetooth device is powered off");
        clearDiscoveryState();
        emitAsyncError();
        return;
    }

    if (m_pendingCancel) {
        m_pendingStart = true;
        return;
    }

    lastError = QBluetoothDeviceDiscoveryAgent::NoError;
    errorString.clear();
    discoveredDevices.clear();

    m_discoveryRequested = true;

    if (m_discoveryAgentProxy->isForeignDiscoveryOngoing()) {
        m_pendingStart = true;
        m_discoveryAgentProxy->stopBluetoothDiscovery();
        return;
    }

    reportPairedDevicesAsync();

    m_discoveryAgentProxy->startBluetoothDiscovery();
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    if (!m_discoveryAgentProxy) {
        lastError = QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Device adapter setting not supported");
        clearDiscoveryState();
        emitAsyncError();
        return;
    }

    m_pendingCancel = true;
    m_pendingStart = false;
    m_discoveryAgentProxy->stopBluetoothDiscovery();
}

void QBluetoothDeviceDiscoveryAgentPrivate::reportDiscoveryStopped(DiscoveryStopReason reason)
{
    if (!m_discoveryRequested)
        return;

    if (m_pendingStart) {
        m_pendingStart = false;
        m_pendingCancel = false;
        start(requestedMethods);
    } else if (m_pendingCancel) {
        m_discoveryRequested = false;
        m_pendingCancel = false;
        Q_EMIT q_ptr->canceled();
    } else if (reason == DiscoveryStopReason::BluetoothPoweredOff) {
        m_discoveryRequested = false;
        lastError = QBluetoothDeviceDiscoveryAgent::PoweredOffError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Local bluetooth device is powered off");
        emitAsyncError();
    } else {
        m_discoveryRequested = false;
        Q_EMIT q_ptr->finished();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::reportPairedDevicesAsync()
{
    QMetaObject::invokeMethod(
        this,
        [this]() {
            if (!m_discoveryRequested)
                return;

            reportDiscoveredDevices(processPairedDevices(m_discoveryAgentProxy->getPairedDevices()));
        },
        Qt::QueuedConnection);
}

void QBluetoothDeviceDiscoveryAgentPrivate::reportDiscoveredDevices(
    const std::vector<QBluetoothDeviceInfo> &bluetoothDevicesInfos)
{
    for (const auto &device : bluetoothDevicesInfos) {
        auto alreadyDiscoveredDevice = std::find_if(
            discoveredDevices.begin(), discoveredDevices.end(),
            [&](const auto &alreadyDiscoveredDevice) {
                return alreadyDiscoveredDevice.address() == device.address();
            });

        if (alreadyDiscoveredDevice != discoveredDevices.end()) {
            const auto updatedFields = updateDeviceInfoIfNeeded(*alreadyDiscoveredDevice, device);
            if (updatedFields != QBluetoothDeviceInfo::Field::None)
                Q_EMIT q_ptr->deviceUpdated(*alreadyDiscoveredDevice, updatedFields);
        } else {
            discoveredDevices.append(device);
            Q_EMIT q_ptr->deviceDiscovered(device);
        }
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::clearDiscoveryState()
{
    m_discoveryRequested = false;
    m_pendingStart = false;
    m_pendingCancel = false;
}

void QBluetoothDeviceDiscoveryAgentPrivate::emitAsyncError()
{
    QMetaObject::invokeMethod(
        q_ptr,
        [this, reportedError = lastError]() {
            Q_EMIT q_ptr->errorOccurred(reportedError);
        },
        Qt::QueuedConnection);
}

QT_END_NAMESPACE

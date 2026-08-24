// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qohosbluetoothdevicediscovery_p.h"

#include <QtBluetooth/private/qohosbluetoothcommon_p.h>
#include <QtBluetooth/private/qohosbluetoothremotedevice_p.h>

#include <QtCore/qloggingcategory.h>
#include <QtCore/private/qnapi_p.h>
#include <QtCore/private/qohosjstools_p.h>
#include <QtCore/private/qohoslogger_p.h>

#include <chrono>
#include <optional>

using namespace std::chrono_literals;

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_OHOS)

namespace QtOhosBluetooth {

namespace {

constexpr qint16 unknownRssi = 0;

std::optional<bool> tryReadBluetoothDiscoveringState(QOhosJsState &jsState)
{
    if (!isBluetoothEnabled(jsState))
        return false;

    try {
        return jsState.eval<QNapi::Boolean>("@ohos.bluetooth.connection.isBluetoothDiscovering()").Value();
    } catch (const Napi::Error &error) {
        if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
            qOhosPrintfWarning(
                "%s: cannot read the bluetooth discovering state, error code: %u", Q_FUNC_INFO,
                qToUnderlying(*optErrorCode));
        }
        return std::nullopt;
    }
}

bool requestBluetoothDiscoveryStop(QOhosJsState &jsState)
{
    const auto optDiscovering = tryReadBluetoothDiscoveringState(jsState);
    if (optDiscovering && !*optDiscovering)
        return true;

    try {
        jsState.eval("@ohos.bluetooth.connection.stopBluetoothDiscovery()");
    } catch (const Napi::Error &error) {
        if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
            qOhosPrintfWarning(
                "%s: cannot stop the bluetooth discovery, error code: %u", Q_FUNC_INFO, qToUnderlying(*optErrorCode));
        }
        return false;
    }

    return true;
}

std::vector<std::string> readPairedDeviceIds(QOhosJsState &jsState)
{
    if (!isBluetoothEnabled(jsState))
        return {};

    try {
        return QNapi::getArrayElements<std::vector<std::string>, QNapi::String>(
            jsState.eval<QNapi::Array>("@ohos.bluetooth.connection.getPairedDevices()"),
            [](QNapi::String &&pairedDeviceId) {
                return pairedDeviceId.Utf8Value();
            });
    } catch (const Napi::Error &error) {
        if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
            qOhosPrintfWarning(
                "%s: cannot read the paired devices, error code: %u", Q_FUNC_INFO, qToUnderlying(*optErrorCode));
        }
        return {};
    }
}

std::vector<DiscoveryResult> readPairedDevices(QOhosJsState &jsState)
{
    auto pairedDeviceIds = readPairedDeviceIds(jsState);

    std::vector<DiscoveryResult> pairedDevices;
    pairedDevices.reserve(pairedDeviceIds.size());
    for (auto &pairedDeviceId : pairedDeviceIds) {
        if (isRemoteDeviceLowEnergyOnly(jsState, pairedDeviceId))
            continue;

        auto pairedDeviceName = readRemoteDeviceName(jsState, pairedDeviceId);
        const auto optPairedDeviceClass = readRemoteDeviceClass(jsState, pairedDeviceId);

        pairedDevices.push_back(
            DiscoveryResult {
                .deviceId = std::move(pairedDeviceId),
                .deviceName = pairedDeviceName
                    ? *pairedDeviceName
                    : std::string(),
                .classOfDevice = optPairedDeviceClass.value_or(0),
                .rssi = unknownRssi,
            });
    }

    return pairedDevices;
}

}

DiscoveryResult DiscoveryResult::makeFromOhosDiscoveryResultObject(
    QOhosJsState &jsState, QNapi::Object discoveryResultObject)
{
    auto deviceId = discoveryResultObject.get<QNapi::String>("deviceId").Utf8Value();

    auto deviceName = discoveryResultObject.get<QNapi::String>("deviceName").Utf8Value();
    if (deviceName.empty()) {
        if (auto optRemoteDeviceName = readRemoteDeviceName(jsState, deviceId))
            deviceName = std::move(*optRemoteDeviceName);
    }

    return DiscoveryResult {
        .deviceId = std::move(deviceId),
        .deviceName = std::move(deviceName),
        .classOfDevice = discoveryResultObject.get<QNapi::Number>("deviceClass.classOfDevice").Uint32Value(),
        .rssi = static_cast<qint16>(discoveryResultObject.get<QNapi::Number>("rssi").Int32Value()),
    };
}

QOhosBluetoothDeviceDiscoveryAgentProxy::QOhosBluetoothDeviceDiscoveryAgentProxy()
    : m_accessProxy(QOhosBluetoothAccessProxy::instance())
{
    QObject::connect(
        m_accessProxy.get(), &QOhosBluetoothAccessProxy::stateChanged, this,
        [this](QOhosBluetoothAccessProxy::BluetoothState bluetoothState) {
            if (bluetoothState == QOhosBluetoothAccessProxy::BluetoothState::STATE_ON)
                return;

            if (!m_discoveryStoppedPollTimer.isActive())
                return;

            resetDiscoveryTracking();
            Q_EMIT bluetoothPoweredOff();
        });

    constexpr auto discoveryStoppedPollInterval = 500ms;
    m_discoveryStoppedPollTimer.setInterval(discoveryStoppedPollInterval);
    QObject::connect(
        &m_discoveryStoppedPollTimer, &QTimer::timeout, this,
        [this]() {
            if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO)) {
                resetDiscoveryTracking();
                m_discoveredDevicesConsumerHandle.reset();
                Q_EMIT missingPermission();
                return;
            }

            if (tryCheckBluetoothDiscovering().value_or(true))
                return;

            resetDiscoveryTracking();
            Q_EMIT discoveryStopped();
        });

    constexpr auto discoveryStoppedTimeout = 30s;
    m_discoveryStoppedTimeoutTimer.setSingleShot(true);
    m_discoveryStoppedTimeoutTimer.setInterval(discoveryStoppedTimeout);
    QObject::connect(
        &m_discoveryStoppedTimeoutTimer, &QTimer::timeout, this,
        [this]() {
            resetDiscoveryTracking();
            Q_EMIT discoveryStopFailed();
        });
}

std::shared_ptr<QOhosBluetoothDeviceDiscoveryAgentProxy> QOhosBluetoothDeviceDiscoveryAgentProxy::instance()
{
    static std::weak_ptr<QOhosBluetoothDeviceDiscoveryAgentProxy> weakInstance;

    auto instance = weakInstance.lock();
    if (!instance) {
        instance = std::shared_ptr<QOhosBluetoothDeviceDiscoveryAgentProxy>(
            new QOhosBluetoothDeviceDiscoveryAgentProxy());
        weakInstance = instance;
    }

    return instance;
}

bool QOhosBluetoothDeviceDiscoveryAgentProxy::ensureDiscoveredDevicesConsumerRegistered()
{
    if (m_discoveredDevicesConsumerHandle)
        return true;

    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO))
        return false;

    auto selfRef = QtOhos::makeQThreadSafeRef(this);
    m_discoveredDevicesConsumerHandle = QOhosJsThreadGateway::eval(
        [&](QOhosJsState &jsState) -> std::shared_ptr<void> {
            auto discoveredDevicesConsumerHandle = registerQOhosOnOffMethodsBasedEventHandler(
                jsState.eval<QNapi::Object>("@ohos.bluetooth.connection"), "discoveryResult",
                [selfRef](const QOhosCallbackInfo &cbInfo) {
                    auto resultArray = cbInfo.getFirstArg<QNapi::Array>(Q_FUNC_INFO);
                    auto discoveredDevices =
                        QNapi::getArrayElements<std::vector<DiscoveryResult>, QNapi::Object>(
                            resultArray,
                            [&](QNapi::Object &&discoveryResultObject) {
                                return DiscoveryResult::makeFromOhosDiscoveryResultObject(
                                    cbInfo.jsState(), std::move(discoveryResultObject));
                            });

                    selfRef.visitInQtThreadIfAlive(
                        [discoveredDevices = std::move(discoveredDevices)](auto &self) {
                            Q_EMIT self.bluetoothDevicesFound(discoveredDevices);
                        });
                },
                {
                    .optOnCallExceptionHandler = [](const Napi::Error &error) {
                        if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
                            qOhosPrintfWarning(
                                "%s: cannot register the discovery result consumer, error code: %u",
                                Q_FUNC_INFO, qToUnderlying(*optErrorCode));
                        }
                    },
                });

            if (!discoveredDevicesConsumerHandle)
                return {};

            return QtOhos::makeProxyWithJsThreadDeleter(
                std::move(discoveredDevicesConsumerHandle));
        },
        Q_FUNC_INFO);

    return m_discoveredDevicesConsumerHandle != nullptr;
}

std::optional<bool> QOhosBluetoothDeviceDiscoveryAgentProxy::tryCheckBluetoothDiscovering()
{
    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO))
        return std::nullopt;

    return QOhosJsThreadGateway::eval(tryReadBluetoothDiscoveringState, Q_FUNC_INFO);
}

void QOhosBluetoothDeviceDiscoveryAgentProxy::resetDiscoveryTracking()
{
    m_discoveryStoppedPollTimer.stop();
    m_discoveryStoppedTimeoutTimer.stop();
    m_discoveryStartedByProxy = false;
}

bool QOhosBluetoothDeviceDiscoveryAgentProxy::isForeignDiscoveryOngoing()
{
    return !m_discoveryStartedByProxy && tryCheckBluetoothDiscovering().value_or(false);
}

void QOhosBluetoothDeviceDiscoveryAgentProxy::startBluetoothDiscovery()
{
    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO)) {
        Q_EMIT missingPermission();
        return;
    }

    if (!ensureDiscoveredDevicesConsumerRegistered()) {
        qCCritical(
            QT_BT_OHOS,
            "%s: discovery result consumer not registered. Found devices would not be reported.",
            Q_FUNC_INFO);
        Q_EMIT discoveryStartFailed();
        return;
    }

    const auto discoveryOngoing = QOhosJsThreadGateway::eval(
        [](QOhosJsState &jsState) {
            const auto optDiscovering = tryReadBluetoothDiscoveringState(jsState);
            if (optDiscovering && *optDiscovering)
                return true;

            try {
                jsState.eval("@ohos.bluetooth.connection.startBluetoothDiscovery()");
            } catch (const Napi::Error &error) {
                if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
                    qOhosPrintfWarning(
                        "%s: cannot start the bluetooth discovery, error code: %u", Q_FUNC_INFO,
                        qToUnderlying(*optErrorCode));
                }
                return false;
            }

            return true;
        },
        Q_FUNC_INFO);

    if (!discoveryOngoing) {
        Q_EMIT discoveryStartFailed();
        return;
    }

    m_discoveryStartedByProxy = true;
    m_discoveryStoppedTimeoutTimer.stop();
    m_discoveryStoppedPollTimer.start();
}

void QOhosBluetoothDeviceDiscoveryAgentProxy::stopBluetoothDiscovery()
{
    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO)) {
        Q_EMIT missingPermission();
        return;
    }

    if (!QOhosJsThreadGateway::eval(requestBluetoothDiscoveryStop, Q_FUNC_INFO)) {
        Q_EMIT discoveryStopFailed();
        return;
    }

    m_discoveryStoppedPollTimer.start();
    m_discoveryStoppedTimeoutTimer.start();
}

std::vector<DiscoveryResult> QOhosBluetoothDeviceDiscoveryAgentProxy::getPairedDevices()
{
    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO))
        return {};

    return QOhosJsThreadGateway::eval(readPairedDevices, Q_FUNC_INFO);
}

}

QT_END_NAMESPACE

#include "moc_qohosbluetoothdevicediscovery_p.cpp"

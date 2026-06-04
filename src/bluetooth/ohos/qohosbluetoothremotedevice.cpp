// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qohosbluetoothremotedevice_p.h"

#include <QtBluetooth/private/qohosbluetoothcommon_p.h>

#include <QtCore/qloggingcategory.h>
#include <QtCore/private/qcore_ohos_p.h>
#include <QtCore/private/qnapi_p.h>
#include <QtCore/private/qohosjstools_p.h>
#include <QtCore/private/qohoslogger_p.h>

#include <optional>
#include <string>
#include <vector>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_OHOS)

namespace QtOhosBluetooth {

namespace {

std::shared_ptr<void> registerBondStateChangeConsumer(
    QOhosJsState &jsState,
    QOhosConsumer<std::string, std::optional<QOhosBluetoothRemoteDeviceProxy::BondState>, std::optional<QOhosBluetoothRemoteDeviceProxy::UnbondCause>> bondStateChangeConsumer)
{
    return registerQOhosOnOffMethodsBasedEventHandler(
        jsState.eval<QNapi::Object>("@ohos.bluetooth.connection"), "bondStateChange",
        [bondStateChangeConsumer = std::move(bondStateChangeConsumer)](const QOhosCallbackInfo &cbInfo) {
            auto bondStateObject = cbInfo.getFirstArg<QNapi::Object>(Q_FUNC_INFO);

            auto deviceId = bondStateObject.get<QNapi::String>("deviceId").Utf8Value();
            const auto optBondState = cbInfo.jsState().tryMapOhosEnumFromJs<QOhosBluetoothRemoteDeviceProxy::BondState>(
                bondStateObject.get<QNapi::Number>("state"));
            const auto unbondCauseValue =
                QNapi::getOptionalPropOrEmpty<QNapi::Number>(bondStateObject, "cause");
            const auto optUnbondCause = unbondCauseValue.IsEmpty()
                ? std::optional<QOhosBluetoothRemoteDeviceProxy::UnbondCause>()
                : cbInfo.jsState().tryMapOhosEnumFromJs<QOhosBluetoothRemoteDeviceProxy::UnbondCause>(
                    unbondCauseValue);

            bondStateChangeConsumer(deviceId, optBondState, optUnbondCause);
        },
        {
            .optOnCallExceptionHandler = [](const Napi::Error &error) {
                if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
                    qOhosPrintfWarning(
                        "%s: cannot register the bond state change consumer, error code: %u",
                        Q_FUNC_INFO, qToUnderlying(*optErrorCode));
                }
            },
        });
}

std::optional<QOhosBluetoothRemoteDeviceProxy::BondState> readPairState(
    QOhosJsState &jsState, const std::string &deviceId)
{
    if (!isBluetoothEnabled(jsState))
        return std::nullopt;

    try {
        return jsState.tryMapOhosEnumFromJs<QOhosBluetoothRemoteDeviceProxy::BondState>(
            jsState.eval<QNapi::Number>(
                "@ohos.bluetooth.connection.getPairState(*)", {deviceId}));
    } catch (const Napi::Error &error) {
        if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
            qOhosPrintfWarning(
                "%s: cannot read the pair state of the device %s, error code: %u", Q_FUNC_INFO,
                deviceId.c_str(), qToUnderlying(*optErrorCode));
        }
        return std::nullopt;
    }
}

}

std::optional<std::string> readRemoteDeviceName(QOhosJsState &jsState, const std::string &deviceId)
{
    try {
        return jsState.eval<QNapi::String>(
            "@ohos.bluetooth.connection.getRemoteDeviceName(*)", {deviceId}).Utf8Value();
    } catch (const Napi::Error &error) {
        if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
            qOhosPrintfWarning(
                "%s: cannot read the name of the device %s, error code: %u", Q_FUNC_INFO,
                deviceId.c_str(), qToUnderlying(*optErrorCode));
        }
        return std::nullopt;
    }
}

std::optional<quint32> readRemoteDeviceClass(QOhosJsState &jsState, const std::string &deviceId)
{
    try {
        return jsState.eval<QNapi::Object>(
            "@ohos.bluetooth.connection.getRemoteDeviceClass(*)", {deviceId})
            .get<QNapi::Number>("classOfDevice").Uint32Value();
    } catch (const Napi::Error &error) {
        if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
            qOhosPrintfWarning(
                "%s: cannot read the class of the device %s, error code: %u", Q_FUNC_INFO,
                deviceId.c_str(), qToUnderlying(*optErrorCode));
        }
        return std::nullopt;
    }
}

std::optional<QOhosBluetoothRemoteDeviceProxy::BluetoothTransport> readRemoteDeviceTransport(
    QOhosJsState &jsState, const std::string &deviceId)
{
    try {
        return jsState.tryMapOhosEnumFromJs<QOhosBluetoothRemoteDeviceProxy::BluetoothTransport>(
            jsState.eval<QNapi::Number>(
                "@ohos.bluetooth.connection.getRemoteDeviceTransport(*)", {deviceId}));
    } catch (const Napi::Error &error) {
        if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
            qOhosPrintfWarning(
                "%s: cannot read the transport of the device %s, error code: %u", Q_FUNC_INFO,
                deviceId.c_str(), qToUnderlying(*optErrorCode));
        }
        return std::nullopt;
    }
}

bool isRemoteDeviceLowEnergyOnly(QOhosJsState &jsState, const std::string &deviceId)
{
    return readRemoteDeviceTransport(jsState, deviceId)
        == QOhosBluetoothRemoteDeviceProxy::BluetoothTransport::TRANSPORT_LE;
}

QOhosBluetoothRemoteDeviceProxy::QOhosBluetoothRemoteDeviceProxy()
{
    ensureBondStateChangeConsumerRegistered();
}

bool QOhosBluetoothRemoteDeviceProxy::ensureBondStateChangeConsumerRegistered()
{
    if (m_bondStateChangeConsumerHandle)
        return true;

    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO))
        return false;

    auto selfRef = QtOhos::makeQThreadSafeRef(this);
    m_bondStateChangeConsumerHandle = QOhosJsThreadGateway::eval(
        [&](QOhosJsState &jsState) -> std::shared_ptr<void> {
            auto bondStateChangeConsumerHandle = registerBondStateChangeConsumer(
                jsState,
                [selfRef](std::string deviceId, std::optional<BondState> optBondState, std::optional<UnbondCause> optUnbondCause) {
                    if (!optBondState)
                        return;

                    selfRef.visitInQtThreadIfAlive(
                        [deviceId = QString::fromStdString(deviceId), bondState = *optBondState, optUnbondCause](auto &self) {
                            Q_EMIT self.bondStateChanged(deviceId, bondState, optUnbondCause);
                        });
                });

            if (!bondStateChangeConsumerHandle)
                return {};

            return QtOhos::makeProxyWithJsThreadDeleter(std::move(bondStateChangeConsumerHandle));
        },
        Q_FUNC_INFO);

    return m_bondStateChangeConsumerHandle != nullptr;
}

std::shared_ptr<QOhosBluetoothRemoteDeviceProxy> QOhosBluetoothRemoteDeviceProxy::instance()
{
    static std::weak_ptr<QOhosBluetoothRemoteDeviceProxy> weakInstance;

    auto instance = weakInstance.lock();
    if (!instance) {
        instance = std::shared_ptr<QOhosBluetoothRemoteDeviceProxy>(
            new QOhosBluetoothRemoteDeviceProxy());
        weakInstance = instance;
    }

    return instance;
}

std::optional<QOhosBluetoothRemoteDeviceProxy::BondState> QOhosBluetoothRemoteDeviceProxy::tryGetPairState(
    const QString &deviceId)
{
    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO))
        return std::nullopt;

    return QOhosJsThreadGateway::eval(
        [&](QOhosJsState &jsState) {
            return readPairState(jsState, deviceId.toStdString());
        },
        Q_FUNC_INFO);
}

std::optional<QOhosBluetoothErrorCode> QOhosBluetoothRemoteDeviceProxy::requestRemoteDeviceServices(
    const QString &deviceId)
{
    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO))
        return QOhosBluetoothErrorCode::PermissionDenied;

    auto selfRef = QtOhos::makeQThreadSafeRef(this);
    return QOhosJsThreadGateway::eval(
        [&](QOhosJsState &jsState) -> std::optional<QOhosBluetoothErrorCode> {
            if (!isBluetoothEnabled(jsState))
                return QOhosBluetoothErrorCode::BluetoothDisabled;

            RemoteDeviceServices remoteDeviceServices;
            remoteDeviceServices.deviceId = deviceId.toStdString();
            remoteDeviceServices.optDeviceName =
                readRemoteDeviceName(jsState, remoteDeviceServices.deviceId);
            remoteDeviceServices.optClassOfDevice =
                readRemoteDeviceClass(jsState, remoteDeviceServices.deviceId);
            remoteDeviceServices.lowEnergyOnly =
                isRemoteDeviceLowEnergyOnly(jsState, remoteDeviceServices.deviceId);

            auto pendingServices = QtOhos::moveToSharedPtr(std::move(remoteDeviceServices));
            auto reportRemoteDeviceServices =
                [selfRef, pendingServices]() {
                    selfRef.visitInQtThreadIfAlive(
                        [pendingServices](auto &self) {
                            Q_EMIT self.remoteDeviceServicesRead(*pendingServices);
                        });
                };

            try {
                jsState.evalToPromiseOrRejectOnThrow(
                    "@ohos.bluetooth.connection.getRemoteProfileUuids(*)",
                    {pendingServices->deviceId})
                .onThen(
                    [reportRemoteDeviceServices, pendingServices](
                        const QOhosCallbackInfo &cbInfo) {
                        auto resultArray = cbInfo.getFirstArg<QNapi::Array>(Q_FUNC_INFO);
                        pendingServices->optProfileUuids =
                            QNapi::getArrayElements<std::vector<std::string>, QNapi::String>(
                                resultArray,
                                [](QNapi::String &&profileUuid) {
                                    return profileUuid.Utf8Value();
                                });
                        reportRemoteDeviceServices();
                    },
                    [reportRemoteDeviceServices](const QOhosCallbackInfo &cbInfo) {
                        QtOhos::logJsCallbackError(cbInfo, "Got error from getRemoteProfileUuids()");
                        reportRemoteDeviceServices();
                    });
                return std::nullopt;
            } catch (const Napi::Error &error) {
                const auto errorCode =
                    tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)
                        .value_or(QOhosBluetoothErrorCode::OperationFailed);
                qOhosPrintfWarning(
                    "%s: cannot read the remote profile uuids, error code: %u", Q_FUNC_INFO,
                    qToUnderlying(errorCode));
                return errorCode;
            }
        },
        Q_FUNC_INFO);
}

bool QOhosBluetoothRemoteDeviceProxy::pairDevice(const QString &deviceId)
{
    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO)) {
        Q_EMIT missingPermission();
        return false;
    }

    if (!ensureBondStateChangeConsumerRegistered()) {
        qCCritical(
            QT_BT_OHOS,
            "%s: bond state change consumer not registered. Pairing result will not be reported.",
            Q_FUNC_INFO);
    }

    auto selfRef = QtOhos::makeQThreadSafeRef(this);
    const auto optErrorCode = QOhosJsThreadGateway::eval(
        [&](QOhosJsState &jsState) -> std::optional<QOhosBluetoothErrorCode> {
            try {
                jsState.evalToPromiseOrRejectOnThrow(
                    "@ohos.bluetooth.connection.pairDevice(*)", {deviceId.toStdString()})
                .onCatch(
                    [selfRef, pairedDeviceId = deviceId.toStdString()](
                        const QOhosCallbackInfo &cbInfo) {
                        QtOhos::logJsCallbackError(cbInfo, "Got error from pairDevice()");
                        selfRef.visitInQtThreadIfAlive(
                            [pairedDeviceId](auto &self) {
                                Q_EMIT self.pairDeviceFailed(QString::fromStdString(pairedDeviceId));
                            });
                    });
                return std::nullopt;
            } catch (const Napi::Error &error) {
                return tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO);
            }
        },
        Q_FUNC_INFO);

    if (!optErrorCode)
        return true;

    qCWarning(
        QT_BT_OHOS, "%s: cannot start pairing with the device %ls, error code: %u", Q_FUNC_INFO,
        qUtf16Printable(deviceId), qToUnderlying(*optErrorCode));

    return false;
}

}

QT_END_NAMESPACE

#include "moc_qohosbluetoothremotedevice_p.cpp"

// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qohosbluetoothaccess_p.h"

#include <QtBluetooth/private/qohosbluetoothcommon_p.h>

#include <QtCore/qloggingcategory.h>
#include <QtCore/private/qnapi_p.h>
#include <QtCore/private/qohoscommon_p.h>
#include <QtCore/private/qohosjstools_p.h>
#include <QtCore/private/qohoslogger_p.h>

#include <memory>
#include <optional>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_OHOS)

namespace QtOhosBluetooth {

namespace {

using BluetoothState = QOhosBluetoothAccessProxy::BluetoothState;

std::shared_ptr<void> registerBluetoothStateChangeConsumer(
    QOhosJsState &jsState, QOhosConsumer<std::optional<BluetoothState>> stateChangeConsumer)
{
    return registerQOhosOnOffMethodsBasedEventHandler(
        jsState.eval<QNapi::Object>("@ohos.bluetooth.access"), "stateChange",
        [stateChangeConsumer = std::move(stateChangeConsumer)](const QOhosCallbackInfo &cbInfo) {
            auto ohosBluetoothState = cbInfo.getFirstArg<QNapi::Number>(Q_FUNC_INFO);
            stateChangeConsumer(
                cbInfo.jsState().tryMapOhosEnumFromJs<BluetoothState>(ohosBluetoothState));
        },
        {
            .optOnCallExceptionHandler = [](const Napi::Error &error) {
                if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
                    qOhosPrintfWarning(
                        "%s: cannot register the bluetooth state change consumer, error code: %u",
                        Q_FUNC_INFO, qToUnderlying(*optErrorCode));
                }
            },
        });
}

QOhosSupplier<std::optional<BluetoothState>> makeBluetoothStateSupplier(
    QOhosConsumer<std::optional<BluetoothState>> bluetoothStateChangeConsumer)
{
    bool stateChangeEventRegistered = false;

    auto cachedBluetoothStateSupplier = makeQOhosDataSource<std::optional<BluetoothState>>(
        readBluetoothState,
        [&](QOhosJsState &jsState, QOhosConsumer<std::optional<BluetoothState>> stateChangeConsumer) {
            auto stateChangeConsumerHandle =
                registerBluetoothStateChangeConsumer(jsState, std::move(stateChangeConsumer));
            stateChangeEventRegistered = stateChangeConsumerHandle != nullptr;

            return stateChangeConsumerHandle;
        },
        std::move(bluetoothStateChangeConsumer),
        QtOhos::invokeInQtThread,
        Q_FUNC_INFO);

    return [stateChangeEventRegistered,
            cachedBluetoothStateSupplier = std::move(cachedBluetoothStateSupplier)]() {
        return stateChangeEventRegistered
            ? cachedBluetoothStateSupplier()
            : QOhosJsThreadGateway::eval(readBluetoothState, Q_FUNC_INFO);
    };
}

}

QOhosBluetoothAccessProxy::QOhosBluetoothAccessProxy()
{
    m_ohosBluetoothStateSupplier = makeBluetoothStateSupplier(
        [this](std::optional<BluetoothState> optBluetoothState) {
            if (optBluetoothState)
                Q_EMIT stateChanged(*optBluetoothState);
        });
}

std::shared_ptr<QOhosBluetoothAccessProxy> QOhosBluetoothAccessProxy::instance()
{
    static auto instance = std::shared_ptr<QOhosBluetoothAccessProxy>(new QOhosBluetoothAccessProxy());
    return instance;
}

std::optional<QOhosBluetoothAccessProxy::BluetoothState> QOhosBluetoothAccessProxy::tryGetBluetoothState() const
{
    return m_ohosBluetoothStateSupplier();
}

std::optional<QOhosBluetoothErrorCode> QOhosBluetoothAccessProxy::trySetBluetoothEnabled(bool enabled)
{
    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO)) {
        Q_EMIT missingPermission();
        return QOhosBluetoothErrorCode::PermissionDenied;
    }

    auto selfRef = QtOhos::makeQThreadSafeRef(this);
    const auto optErrorCode = QOhosJsThreadGateway::eval(
        [&](QOhosJsState &jsState) -> std::optional<QOhosBluetoothErrorCode> {
            try {
                jsState.evalToPromiseOrRejectOnThrow(
                    enabled ? "@ohos.bluetooth.access.enableBluetoothAsync()"
                            : "@ohos.bluetooth.access.disableBluetoothAsync()")
                .onCatch(
                    [selfRef](const QOhosCallbackInfo &cbInfo) {
                        const auto errorValue = cbInfo.getFirstArg<QNapi::Value>(Q_FUNC_INFO);
                        const auto errorCode =
                            tryGetKnownErrorCode(errorValue, Q_FUNC_INFO)
                                .value_or(QOhosBluetoothErrorCode::OperationFailed);
                        qOhosPrintfWarning(
                            "%s: the bluetooth switch request was rejected, error code: %u",
                            Q_FUNC_INFO, qToUnderlying(errorCode));
                        selfRef.visitInQtThreadIfAlive(
                            [errorCode](auto &self) {
                                Q_EMIT self.setBluetoothEnabledFailed(errorCode);
                            });
                    });
                return std::nullopt;
            } catch (const Napi::Error &error) {
                return tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO);
            }
        },
        Q_FUNC_INFO);

    if (!optErrorCode)
        return std::nullopt;

    if (*optErrorCode == QOhosBluetoothErrorCode::PermissionDenied) {
        qCWarning(
            QT_BT_OHOS, "%s: no permission to set the bluetooth enabled state %d", Q_FUNC_INFO,
            enabled);
        Q_EMIT missingPermission();
        return optErrorCode;
    }

    qCWarning(
        QT_BT_OHOS, "%s: cannot set the bluetooth enabled state %d, error code: %u", Q_FUNC_INFO,
        enabled, qToUnderlying(*optErrorCode));

    return optErrorCode;
}

}

QT_END_NAMESPACE

#include "moc_qohosbluetoothaccess_p.cpp"

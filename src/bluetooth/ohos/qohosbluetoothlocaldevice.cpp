// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qohosbluetoothlocaldevice_p.h"

#include <QtBluetooth/private/qohosbluetoothcommon_p.h>

#include <QtCore/private/qcore_ohos_p.h>
#include <QtCore/private/qnapi_p.h>
#include <QtCore/private/qohosjstools_p.h>
#include <QtCore/private/qohoslogger_p.h>

#include <memory>

QT_BEGIN_NAMESPACE

namespace QtOhosBluetooth {

namespace {

constexpr int permanentlyDiscoverableDuration = 0;

template<typename T>
QOhosSupplier<T> makeQOhosLazySupplier(
    T fallbackValue, QOhosSupplier<std::optional<QOhosSupplier<T>>> supplierFactory)
{
    struct Context
    {
        QOhosSupplier<T> supplier;
        QOhosSupplier<std::optional<QOhosSupplier<T>>> optSupplierFactory;
    };

    auto context = std::make_shared<Context>();
    if (auto optSupplier = supplierFactory()) {
        context->supplier = std::move(*optSupplier);
    } else {
        context->supplier = [fallbackValue = std::move(fallbackValue)]() { return fallbackValue; };
        context->optSupplierFactory = std::move(supplierFactory);
    }

    return [context]() {
        if (context->optSupplierFactory) {
            if (auto optSupplier = context->optSupplierFactory()) {
                context->supplier = std::move(*optSupplier);
                context->optSupplierFactory = nullptr;
            }
        }

        return context->supplier();
    };
}

std::optional<QOhosBluetoothLocalDeviceProxy::ScanMode> readScanMode(QOhosJsState &jsState)
{
    if (!isBluetoothEnabled(jsState))
        return std::nullopt;

    try {
        return jsState.tryMapOhosEnumFromJs<QOhosBluetoothLocalDeviceProxy::ScanMode>(
            jsState.eval<QNapi::Number>("@ohos.bluetooth.connection.getBluetoothScanMode()"));
    } catch (const Napi::Error &error) {
        if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
            qOhosPrintfWarning(
                "%s: cannot read the bluetooth scan mode, error code: %u", Q_FUNC_INFO, qToUnderlying(*optErrorCode));
        }
        return std::nullopt;
    }
}

std::shared_ptr<void> registerScanModeChangeConsumer(
    QOhosJsState &jsState,
    QOhosConsumer<std::optional<QOhosBluetoothLocalDeviceProxy::ScanMode>> scanModeChangeConsumer)
{
    auto sharedScanModeChangeConsumer = QtOhos::moveToSharedPtr(std::move(scanModeChangeConsumer));

    auto connectionModule = jsState.eval<QNapi::Object>("@ohos.bluetooth.connection");

    auto scanModeChangeHandlerRef = QtOhos::moveToSharedPtr(
        QNapi::Reference<>::makePersistentFrom(
            QNapi::Function::New(
                jsState.env(),
                [weakScanModeChangeConsumer = QtOhos::makeWeakPtr(sharedScanModeChangeConsumer)](const QOhosCallbackInfo &cbInfo) {
                    auto scanModeChangeConsumer = weakScanModeChangeConsumer.lock();
                    if (!scanModeChangeConsumer) {
                        qOhosPrintfWarning(
                            "%s: got unexpected scan mode change callback call for detached handler",
                            Q_FUNC_INFO);
                        return cbInfo.Env().Undefined();
                    }

                    auto ohosScanMode = cbInfo.getFirstArg<QNapi::Number>(Q_FUNC_INFO);
                    (*scanModeChangeConsumer)(
                        cbInfo.jsState().tryMapOhosEnumFromJs<QOhosBluetoothLocalDeviceProxy::ScanMode>(ohosScanMode));

                    return cbInfo.Env().Undefined();
                })));

    try {
        connectionModule.eval("onScanModeChange(*)", {scanModeChangeHandlerRef->Value()});
    } catch (const Napi::Error &error) {
        if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
            qOhosPrintfWarning(
                "%s: cannot register the scan mode change consumer, error code: %u", Q_FUNC_INFO,
                qToUnderlying(*optErrorCode));
        }
        return {};
    }

    return QtOhos::makeDestroyNotifier(
        [sharedScanModeChangeConsumer, scanModeChangeHandlerRef,
            connectionModuleWeakRef = QtOhos::moveToSharedPtr(Napi::Weak(connectionModule))]() {
            auto connectionModuleValue = connectionModuleWeakRef->Value();
            if (!connectionModuleValue.IsObject()) {
                qOhosPrintfDebug(
                    "%s: not unregistering the scan mode change consumer, the module is not alive anymore",
                    Q_FUNC_INFO);
                return;
            }

            try {
                QNapi::checkedCast<QNapi::Object>(connectionModuleValue).eval(
                    "offScanModeChange(*)", {scanModeChangeHandlerRef->Value()});
            } catch (const Napi::Error &error) {
                if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
                    qOhosPrintfWarning(
                        "%s: cannot unregister the scan mode change consumer, error code: %u",
                        Q_FUNC_INFO, qToUnderlying(*optErrorCode));
                }
            }
        });
}

QOhosSupplier<std::optional<QOhosBluetoothLocalDeviceProxy::ScanMode>> makeScanModeSupplier(
    QOhosConsumer<std::optional<QOhosBluetoothLocalDeviceProxy::ScanMode>> optScanModeChangeConsumer)
{
    bool scanModeChangeEventRegistered = false;

    auto cachedScanModeSupplier =
        makeQOhosDataSource<std::optional<QOhosBluetoothLocalDeviceProxy::ScanMode>>(
            readScanMode,
            [&](QOhosJsState &jsState,
                QOhosConsumer<std::optional<QOhosBluetoothLocalDeviceProxy::ScanMode>> scanModeChangeConsumer) {
                auto scanModeChangeConsumerHandle =
                    registerScanModeChangeConsumer(jsState, std::move(scanModeChangeConsumer));
                scanModeChangeEventRegistered = scanModeChangeConsumerHandle != nullptr;

                return scanModeChangeConsumerHandle;
            },
            std::move(optScanModeChangeConsumer),
            QtOhos::invokeInQtThread,
            Q_FUNC_INFO);

    return [scanModeChangeEventRegistered, cachedScanModeSupplier = std::move(cachedScanModeSupplier)]()
        -> std::optional<QOhosBluetoothLocalDeviceProxy::ScanMode> {
        if (scanModeChangeEventRegistered) {
            if (const auto optCachedScanMode = cachedScanModeSupplier())
                return optCachedScanMode;
        }

        return QOhosJsThreadGateway::eval(readScanMode, Q_FUNC_INFO);
    };
}

}

QOhosBluetoothLocalDeviceProxy::QOhosBluetoothLocalDeviceProxy()
    : m_scanModeSupplier(
        makeQOhosLazySupplier<std::optional<ScanMode>>(
            std::nullopt,
            [this]() -> std::optional<QOhosSupplier<std::optional<ScanMode>>> {
                if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO))
                    return std::nullopt;

                return makeScanModeSupplier(
                    [this](auto optScanMode) {
                        if (optScanMode)
                            Q_EMIT scanModeChanged(*optScanMode);
                    });
            }))
{
}

std::shared_ptr<QOhosBluetoothLocalDeviceProxy> QOhosBluetoothLocalDeviceProxy::instance()
{
    static auto instance = std::shared_ptr<QOhosBluetoothLocalDeviceProxy>(new QOhosBluetoothLocalDeviceProxy());
    return instance;
}

std::optional<QString> QOhosBluetoothLocalDeviceProxy::tryGetLocalDeviceName() const
{
    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO))
        return std::nullopt;

    return QOhosJsThreadGateway::eval(
        [](QOhosJsState &jsState) -> std::optional<QString> {
            if (!isBluetoothEnabled(jsState))
                return std::nullopt;

            try {
                return QString::fromStdString(
                    jsState.eval<QNapi::String>("@ohos.bluetooth.connection.getLocalName()").Utf8Value());
            } catch (const Napi::Error &error) {
                if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
                    qOhosPrintfWarning(
                        "%s: cannot read the local device name, error code: %u", Q_FUNC_INFO,
                        qToUnderlying(*optErrorCode));
                }
                return std::nullopt;
            }
        });
}

std::optional<QOhosBluetoothLocalDeviceProxy::ScanMode> QOhosBluetoothLocalDeviceProxy::tryGetScanMode()
{
    return m_scanModeSupplier();
}

std::optional<QOhosBluetoothErrorCode> QOhosBluetoothLocalDeviceProxy::trySetBluetoothScanMode(
    QOhosBluetoothLocalDeviceProxy::ScanMode scanMode)
{
    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO)) {
        Q_EMIT missingPermission();
        return QOhosBluetoothErrorCode::PermissionDenied;
    }

    const auto optErrorCode = QOhosJsThreadGateway::eval(
        [&](QOhosJsState &jsState) -> std::optional<QOhosBluetoothErrorCode> {
            if (!isBluetoothEnabled(jsState))
                return QOhosBluetoothErrorCode::BluetoothDisabled;

            try {
                jsState.eval(
                    "@ohos.bluetooth.connection.setBluetoothScanMode(*)",
                    {
                        jsState.mapOhosEnumToJs(scanMode),
                        permanentlyDiscoverableDuration,
                    });
                return std::nullopt;
            } catch (const Napi::Error &error) {
                return tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO);
            }
        });

    if (optErrorCode == QOhosBluetoothErrorCode::PermissionDenied) {
        Q_EMIT missingPermission();
    }

    return optErrorCode;
}

}

QT_END_NAMESPACE

#include "moc_qohosbluetoothlocaldevice_p.cpp"

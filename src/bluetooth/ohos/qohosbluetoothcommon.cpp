// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qohosbluetoothcommon_p.h"

#include <QtBluetooth/private/qohosbluetoothenums_p.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qpermissions.h>
#include <QtCore/qstring.h>
#include <QtCore/private/qnapi_p.h>
#include <QtCore/private/qohoscommon_p.h>
#include <QtCore/private/qohoslogger_p.h>

#include <optional>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_OHOS)

namespace QtOhosBluetooth {

namespace {

std::optional<std::uint32_t> tryGetCodeFromJsBusinessError(const QNapi::Value &errorValue)
{
    if (!errorValue.IsObject())
        return std::nullopt;

    auto errorObject = QNapi::checkedCast<QNapi::Object>(errorValue);
    auto errorCodeValue = QNapi::getOptionalPropOrEmpty<QNapi::Value>(errorObject, "code");

    if (errorCodeValue.IsNumber())
        return QNapi::checkedCast<QNapi::Number>(errorCodeValue).Uint32Value();

    if (errorCodeValue.IsString()) {
        const auto codeText =
            QString::fromStdString(QNapi::checkedCast<QNapi::String>(errorCodeValue));
        bool parsed = false;
        const auto code = codeText.toUInt(&parsed);
        if (parsed)
            return code;

        qOhosPrintfWarning(
            "%s: non-numeric 'code' property in JS business error object: %s", Q_FUNC_INFO,
            qUtf8Printable(codeText));
    }

    return std::nullopt;
}

}

std::optional<QOhosBluetoothErrorCode> tryGetKnownErrorCode(
    const QNapi::Value &errorValue, const char *callerContextName)
{
    const auto optErrorCodeValue = tryGetCodeFromJsBusinessError(errorValue);
    if (!optErrorCodeValue) {
        qOhosPrintfError(
            "%s: no numeric 'code' property in JS business error object", callerContextName);
        return std::nullopt;
    }

    const auto errorCode = static_cast<QOhosBluetoothErrorCode>(*optErrorCodeValue);
    switch (errorCode) {
    case QOhosBluetoothErrorCode::PermissionDenied:
    case QOhosBluetoothErrorCode::InvalidParameter:
    case QOhosBluetoothErrorCode::CapabilityNotSupported:
    case QOhosBluetoothErrorCode::ServiceStopped:
    case QOhosBluetoothErrorCode::BluetoothDisabled:
    case QOhosBluetoothErrorCode::ProfileNotSupported:
    case QOhosBluetoothErrorCode::UserDidNotRespond:
    case QOhosBluetoothErrorCode::UserRefused:
    case QOhosBluetoothErrorCode::OperationFailed:
    case QOhosBluetoothErrorCode::InputOutput:
        return errorCode;
    }

    qOhosPrintfError(
        "%s: unexpected JS business error %u", callerContextName, *optErrorCodeValue);
    return std::nullopt;
}

bool checkAccessBluetoothPermissionGranted(const char *callerContextName)
{
    if (!qApp) {
        qOhosPrintfWarning("%s: no application instance to check the permission on", callerContextName);
        return false;
    }

    if (qApp->checkPermission(QBluetoothPermission{}) == Qt::PermissionStatus::Granted)
        return true;

    qCWarning(QT_BT_OHOS, "%s: access bluetooth permission not granted", callerContextName);
    return false;
}

std::optional<QtOhosBluetooth::enums::ohos::bluetooth::access::BluetoothState> readBluetoothState(
    QOhosJsState &jsState)
{
    using QtOhosBluetooth::enums::ohos::bluetooth::access::BluetoothState;

    try {
        return jsState.tryMapOhosEnumFromJs<BluetoothState>(
            jsState.eval<QNapi::Number>("@ohos.bluetooth.access.getState()"));
    } catch (const Napi::Error &error) {
        if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
            qOhosPrintfWarning(
                "%s: cannot read the bluetooth state, error code: %u", Q_FUNC_INFO,
                qToUnderlying(*optErrorCode));
        }
        return std::nullopt;
    }
}

bool isBluetoothEnabled(QOhosJsState &jsState)
{
    using QtOhosBluetooth::enums::ohos::bluetooth::access::BluetoothState;

    return readBluetoothState(jsState) == BluetoothState::STATE_ON;
}

}

QT_END_NAMESPACE

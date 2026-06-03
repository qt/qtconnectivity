// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QOHOSBLUETOOTHCOMMON_P_H
#define QOHOSBLUETOOTHCOMMON_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtBluetooth/private/qohosbluetoothenums_p.h>

#include <QtCore/qtconfigmacros.h>
#include <QtCore/private/qcore_ohos_p.h>

#include <cstdint>
#include <optional>

QT_BEGIN_NAMESPACE

namespace QtOhosBluetooth {

enum class QOhosBluetoothErrorCode : std::uint32_t {
    PermissionDenied = 201,
    InvalidParameter = 401,
    CapabilityNotSupported = 801,
    ServiceStopped = 2900001,
    BluetoothDisabled = 2900003,
    ProfileNotSupported = 2900004,
    UserDidNotRespond = 2900013,
    UserRefused = 2900014,
    OperationFailed = 2900099,
    InputOutput = 2901054,
};

std::optional<QOhosBluetoothErrorCode> tryGetKnownErrorCode(
    const QNapi::Value &errorValue, const char *callerContextName);

bool checkAccessBluetoothPermissionGranted(const char *callerContextName);
std::optional<QtOhosBluetooth::enums::ohos::bluetooth::access::BluetoothState> readBluetoothState(
    QOhosJsState &jsState);
bool isBluetoothEnabled(QOhosJsState &jsState);

}

QT_END_NAMESPACE

#endif // QOHOSBLUETOOTHCOMMON_P_H

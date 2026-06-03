// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QOHOSBLUETOOTHENUMS_P_H
#define QOHOSBLUETOOTHENUMS_P_H

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

#include <QtCore/qglobal.h>
#include <QtCore/qmetatype.h>
#include <array>
#include <info/application_target_sdk_version.h>

QT_BEGIN_NAMESPACE

namespace QtOhosBluetooth {

namespace enums {

namespace ohos {

namespace bluetooth {

namespace access {

enum class BluetoothState {
    STATE_BLE_ON,
    STATE_BLE_TURNING_OFF,
    STATE_BLE_TURNING_ON,
    STATE_OFF,
    STATE_ON,
    STATE_TURNING_OFF,
    STATE_TURNING_ON,
};

}

}

}

}

}

namespace QtOhos {

template<typename Enum>
struct OhosEnumMeta;

template<>
struct OhosEnumMeta<QtOhosBluetooth::enums::ohos::bluetooth::access::BluetoothState>
{
    using Enum = QtOhosBluetooth::enums::ohos::bluetooth::access::BluetoothState;
    static constexpr const char *fullTypeName = "@ohos.bluetooth.access.BluetoothState";
    static constexpr std::array<std::pair<Enum, const char *>, 7> enumeratorsNames = {{
        {Enum::STATE_BLE_ON, "STATE_BLE_ON"},
        {Enum::STATE_BLE_TURNING_OFF, "STATE_BLE_TURNING_OFF"},
        {Enum::STATE_BLE_TURNING_ON, "STATE_BLE_TURNING_ON"},
        {Enum::STATE_OFF, "STATE_OFF"},
        {Enum::STATE_ON, "STATE_ON"},
        {Enum::STATE_TURNING_OFF, "STATE_TURNING_OFF"},
        {Enum::STATE_TURNING_ON, "STATE_TURNING_ON"},
    }};
};

}

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE(QtOhosBluetooth::enums::ohos::bluetooth::access::BluetoothState));

#endif

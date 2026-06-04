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

namespace connection {

enum class BluetoothTransport {
    TRANSPORT_BR_EDR,
    TRANSPORT_DUAL,
    TRANSPORT_LE,
    TRANSPORT_UNKNOWN,
};

enum class BondState {
    BOND_STATE_BONDED,
    BOND_STATE_BONDING,
    BOND_STATE_INVALID,
};

enum class ScanMode {
    SCAN_MODE_CONNECTABLE,
    SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE,
    SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE,
    SCAN_MODE_GENERAL_DISCOVERABLE,
    SCAN_MODE_LIMITED_DISCOVERABLE,
    SCAN_MODE_NONE,
};

enum class UnbondCause {
    AUTH_FAILURE,
    AUTH_REJECTED,
    INTERNAL_ERROR,
    REMOTE_DEVICE_DOWN,
    USER_REMOVED,
};

}

namespace socket {

enum class SppType {
    SPP_L2CAP,
    SPP_L2CAP_BLE,
    SPP_RFCOMM,
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

template<>
struct OhosEnumMeta<QtOhosBluetooth::enums::ohos::bluetooth::connection::BluetoothTransport>
{
    using Enum = QtOhosBluetooth::enums::ohos::bluetooth::connection::BluetoothTransport;
    static constexpr const char *fullTypeName = "@ohos.bluetooth.connection.BluetoothTransport";
    static constexpr std::array<std::pair<Enum, const char *>, 4> enumeratorsNames = {{
        {Enum::TRANSPORT_BR_EDR, "TRANSPORT_BR_EDR"},
        {Enum::TRANSPORT_DUAL, "TRANSPORT_DUAL"},
        {Enum::TRANSPORT_LE, "TRANSPORT_LE"},
        {Enum::TRANSPORT_UNKNOWN, "TRANSPORT_UNKNOWN"},
    }};
};

template<>
struct OhosEnumMeta<QtOhosBluetooth::enums::ohos::bluetooth::connection::BondState>
{
    using Enum = QtOhosBluetooth::enums::ohos::bluetooth::connection::BondState;
    static constexpr const char *fullTypeName = "@ohos.bluetooth.connection.BondState";
    static constexpr std::array<std::pair<Enum, const char *>, 3> enumeratorsNames = {{
        {Enum::BOND_STATE_BONDED, "BOND_STATE_BONDED"},
        {Enum::BOND_STATE_BONDING, "BOND_STATE_BONDING"},
        {Enum::BOND_STATE_INVALID, "BOND_STATE_INVALID"},
    }};
};

template<>
struct OhosEnumMeta<QtOhosBluetooth::enums::ohos::bluetooth::connection::ScanMode>
{
    using Enum = QtOhosBluetooth::enums::ohos::bluetooth::connection::ScanMode;
    static constexpr const char *fullTypeName = "@ohos.bluetooth.connection.ScanMode";
    static constexpr std::array<std::pair<Enum, const char *>, 6> enumeratorsNames = {{
        {Enum::SCAN_MODE_CONNECTABLE, "SCAN_MODE_CONNECTABLE"},
        {Enum::SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE, "SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE"},
        {Enum::SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE, "SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE"},
        {Enum::SCAN_MODE_GENERAL_DISCOVERABLE, "SCAN_MODE_GENERAL_DISCOVERABLE"},
        {Enum::SCAN_MODE_LIMITED_DISCOVERABLE, "SCAN_MODE_LIMITED_DISCOVERABLE"},
        {Enum::SCAN_MODE_NONE, "SCAN_MODE_NONE"},
    }};
};

template<>
struct OhosEnumMeta<QtOhosBluetooth::enums::ohos::bluetooth::connection::UnbondCause>
{
    using Enum = QtOhosBluetooth::enums::ohos::bluetooth::connection::UnbondCause;
    static constexpr const char *fullTypeName = "@ohos.bluetooth.connection.UnbondCause";
    static constexpr std::array<std::pair<Enum, const char *>, 5> enumeratorsNames = {{
        {Enum::AUTH_FAILURE, "AUTH_FAILURE"},
        {Enum::AUTH_REJECTED, "AUTH_REJECTED"},
        {Enum::INTERNAL_ERROR, "INTERNAL_ERROR"},
        {Enum::REMOTE_DEVICE_DOWN, "REMOTE_DEVICE_DOWN"},
        {Enum::USER_REMOVED, "USER_REMOVED"},
    }};
};

template<>
struct OhosEnumMeta<QtOhosBluetooth::enums::ohos::bluetooth::socket::SppType>
{
    using Enum = QtOhosBluetooth::enums::ohos::bluetooth::socket::SppType;
    static constexpr const char *fullTypeName = "@ohos.bluetooth.socket.SppType";
    static constexpr std::array<std::pair<Enum, const char *>, 3> enumeratorsNames = {{
        {Enum::SPP_L2CAP, "SPP_L2CAP"},
        {Enum::SPP_L2CAP_BLE, "SPP_L2CAP_BLE"},
        {Enum::SPP_RFCOMM, "SPP_RFCOMM"},
    }};
};

}

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE(QtOhosBluetooth::enums::ohos::bluetooth::access::BluetoothState));
Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE(QtOhosBluetooth::enums::ohos::bluetooth::connection::BluetoothTransport));
Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE(QtOhosBluetooth::enums::ohos::bluetooth::connection::BondState));
Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE(QtOhosBluetooth::enums::ohos::bluetooth::connection::ScanMode));
Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE(QtOhosBluetooth::enums::ohos::bluetooth::connection::UnbondCause));
Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE(QtOhosBluetooth::enums::ohos::bluetooth::socket::SppType));

#endif

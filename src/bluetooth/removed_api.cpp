// Copyright (C) 2022 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#define QT_BLUETOOTH_BUILD_REMOVED_API

// Undefine Qt 128-bit int types
#define QT_NO_INT128

#include "qtbluetoothglobal.h"

QT_USE_NAMESPACE

// This part is not typical to the removed_api.cpp, because we need to have
// an extra condition. When adding new removed APIs for Qt 6.6, do not put them
// into this section. Instead, add them to the section below.
#if QT_BLUETOOTH_REMOVED_SINCE(6, 6) || !defined(QT_SUPPORTS_INT128)

#include "qbluetoothuuid.h"

static_assert(std::is_aggregate_v<quint128>);
static_assert(std::is_trivial_v<quint128>);

QBluetoothUuid::QBluetoothUuid(quint128 uuid)
    : QUuid(fromBytes(uuid.data))
{}

quint128 QBluetoothUuid::toUInt128() const
{
    quint128 uuid;
    const auto bytes = toBytes();
    memcpy(uuid.data, bytes.data, sizeof(uuid.data));
    return uuid;
}

#endif // QT_BLUETOOTH_REMOVED_SINCE(6, 6) || !defined(QT_SUPPORTS_INT128)

// This is a common section for Qt 6.6 removed APIs
#if QT_BLUETOOTH_REMOVED_SINCE(6, 6)

#include "qbluetoothaddress.h" // inlined API

#endif // QT_BLUETOOTH_REMOVED_SINCE(6, 6)

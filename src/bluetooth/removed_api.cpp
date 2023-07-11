// Copyright (C) 2022 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#define QT_BLUETOOTH_BUILD_REMOVED_API
// before we undef __SIZEOF_INT128__
#include <cstddef>
#include <cstdint>

#ifdef __SIZEOF_INT128__
// ensure QtCore/qtypes.h doesn't define quint128
#  undef __SIZEOF_INT128__
#endif

#include "qtbluetoothglobal.h"

QT_USE_NAMESPACE

#if QT_BLUETOOTH_REMOVED_SINCE(6, 6)
#include "qbluetoothaddress.h" // inlined API

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
#endif // QT_BLUETOOTH_REMOVED_SINCE(6, 6)

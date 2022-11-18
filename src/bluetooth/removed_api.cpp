// Copyright (C) 2022 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#define QT_BLUETOOTH_BUILD_REMOVED_API
#include <stddef.h>     // before we undef __SIZEOF_INT128__

#ifdef __SIZEOF_INT128__
// ensure QtCore/qtypes.h doesn't define quint128
#  undef __SIZEOF_INT128__
#endif

#include "qtbluetoothglobal.h"

QT_USE_NAMESPACE

#if QT_BLUETOOTH_REMOVED_SINCE(6, 6)
#include "qbluetoothuuid.h"

static_assert(std::is_aggregate_v<quint128>);
static_assert(std::is_trivial_v<quint128>);

QBluetoothUuid::QBluetoothUuid(quint128 uuid)
    : QUuid(uuid.d)
{}

quint128 QBluetoothUuid::toUInt128() const
{
    return { toBytes() };
}
#endif // QT_BLUETOOTH_REMOVED_SINCE(6, 6)

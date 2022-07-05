# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#####################################################################
## tst_qbluetoothlocaldevice Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothlocaldevice
    SOURCES
        tst_qbluetoothlocaldevice.cpp
    PUBLIC_LIBRARIES
        Qt::BluetoothPrivate
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qbluetoothlocaldevice CONDITION MACOS
    PUBLIC_LIBRARIES
        Qt::Widgets
)

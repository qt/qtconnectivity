# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qbluetoothlocaldevice Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothlocaldevice
    SOURCES
        tst_qbluetoothlocaldevice.cpp
    LIBRARIES
        Qt::BluetoothPrivate
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qbluetoothlocaldevice CONDITION MACOS
    LIBRARIES
        Qt::Widgets
)

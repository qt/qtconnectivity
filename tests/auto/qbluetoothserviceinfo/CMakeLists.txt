# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qbluetoothserviceinfo Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothserviceinfo
    SOURCES
        tst_qbluetoothserviceinfo.cpp
    LIBRARIES
        Qt::Bluetooth
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qbluetoothserviceinfo CONDITION MACOS
    LIBRARIES
        Qt::Widgets
)

qt_internal_extend_target(tst_qbluetoothserviceinfo CONDITION ANDROID AND NOT ANDROID_EMBEDDED
    DEFINES
        QT_ANDROID_BLUETOOTH
)

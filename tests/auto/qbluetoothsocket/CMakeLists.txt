# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#####################################################################
## tst_qbluetoothsocket Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothsocket
    SOURCES
        tst_qbluetoothsocket.cpp
    LIBRARIES
        Qt::BluetoothPrivate
        Qt::Network
)

#### Keys ignored in scope 1:.:.:qbluetoothsocket.pro:<TRUE>:
# OTHER_FILES = "README.txt"
# testcase.timeout = "250"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qbluetoothsocket CONDITION MACOS
    DEFINES
        QT_OSX_BLUETOOTH
    LIBRARIES
        Qt::Widgets
)

qt_internal_extend_target(tst_qbluetoothsocket CONDITION ANDROID AND NOT ANDROID_EMBEDDED
    DEFINES
        QT_ANDROID_BLUETOOTH
)

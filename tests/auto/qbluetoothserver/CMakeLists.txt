# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qbluetoothserver Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothserver
    SOURCES
        tst_qbluetoothserver.cpp
    LIBRARIES
        Qt::BluetoothPrivate
)

#### Keys ignored in scope 1:.:.:qbluetoothserver.pro:<TRUE>:
# OTHER_FILES = "README.txt"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qbluetoothserver CONDITION MACOS
    LIBRARIES
        Qt::Widgets
)

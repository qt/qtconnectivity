# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#####################################################################
## tst_qbluetoothaddress Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothaddress
    SOURCES
        tst_qbluetoothaddress.cpp
    LIBRARIES
        Qt::Bluetooth
)

# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#####################################################################
## tst_qbluetoothhostinfo Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothhostinfo
    SOURCES
        tst_qbluetoothhostinfo.cpp
    LIBRARIES
        Qt::Bluetooth
)

# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qbluetoothhostinfo Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothhostinfo
    SOURCES
        tst_qbluetoothhostinfo.cpp
    LIBRARIES
        Qt::Bluetooth
)

# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qbluetoothuuid Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothuuid
    SOURCES
        tst_qbluetoothuuid.cpp
    LIBRARIES
        Qt::Bluetooth
        Qt::CorePrivate
)

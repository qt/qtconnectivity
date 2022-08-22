# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qlowenergyservice Test:
#####################################################################

qt_internal_add_test(tst_qlowenergyservice
    SOURCES
        tst_qlowenergyservice.cpp
    LIBRARIES
        Qt::Bluetooth
)

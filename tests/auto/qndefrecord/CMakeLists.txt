# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qndefrecord Test:
#####################################################################

qt_internal_add_test(tst_qndefrecord
    SOURCES
        tst_qndefrecord.cpp
    LIBRARIES
        Qt::Nfc
)

# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qndefmessage Test:
#####################################################################

qt_internal_add_test(tst_qndefmessage
    SOURCES
        tst_qndefmessage.cpp
    LIBRARIES
        Qt::Nfc
)

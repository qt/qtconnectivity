# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#####################################################################
## tst_qndefmessage Test:
#####################################################################

qt_internal_add_test(tst_qndefmessage
    SOURCES
        tst_qndefmessage.cpp
    PUBLIC_LIBRARIES
        Qt::Nfc
)

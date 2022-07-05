# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

add_subdirectory(bluetooth)
add_subdirectory(nfc)
if(ANDROID)
    add_subdirectory(android)
endif()

if(QT_FEATURE_bluez)
    add_subdirectory(tools/sdpscanner)
endif()

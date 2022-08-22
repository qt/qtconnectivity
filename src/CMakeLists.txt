# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(bluetooth)
add_subdirectory(nfc)
if(ANDROID)
    add_subdirectory(android)
endif()

if(QT_FEATURE_bluez)
    add_subdirectory(tools/sdpscanner)
endif()

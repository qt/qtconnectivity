# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial

if(TARGET Qt::Bluetooth)
    add_subdirectory(bluetooth)
endif()
if(TARGET Qt::Nfc)
    add_subdirectory(nfc)
endif()

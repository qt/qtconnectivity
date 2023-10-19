# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial

set(BlueZ_FOUND 0)

find_package(PkgConfig QUIET)

pkg_check_modules(BLUEZ bluez IMPORTED_TARGET)

if(TARGET PkgConfig::BLUEZ)
    set(BlueZ_FOUND 1)
endif()

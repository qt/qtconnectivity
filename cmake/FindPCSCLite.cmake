# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

if(WIN32)
    add_library(PkgConfig::PCSCLITE INTERFACE IMPORTED)
    target_link_libraries(PkgConfig::PCSCLITE INTERFACE winscard)
    set(PCSCLITE_FOUND 1)
elseif(MACOS)
    qt_internal_find_apple_system_framework(FWPCSC PCSC)
    add_library(PkgConfig::PCSCLITE INTERFACE IMPORTED)
    target_link_libraries(PkgConfig::PCSCLITE INTERFACE ${FWPCSC})
    set(PCSCLITE_FOUND 1)
else()
    find_package(PkgConfig QUIET)

    pkg_check_modules(PCSCLITE libpcsclite IMPORTED_TARGET)
endif()

if(NOT TARGET PkgConfig::PCSCLITE)
    set(PCSCLITE_FOUND 0)
endif()

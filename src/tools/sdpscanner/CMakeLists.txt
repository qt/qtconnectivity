# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial

#####################################################################
## sdpscanner Tool:
#####################################################################

if(NOT TARGET PkgConfig::BLUEZ)
     qt_find_package(BlueZ PROVIDED_TARGETS PkgConfig::BLUEZ)
endif()

qt_internal_add_app(sdpscanner
    INSTALL_DIR "${INSTALL_LIBEXECDIR}"
    SOURCES
        main.cpp
    LIBRARIES
        PkgConfig::BLUEZ
)

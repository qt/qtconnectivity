# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(btchat LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/bluetooth/btchat")

find_package(Qt6 REQUIRED COMPONENTS Bluetooth Core Widgets)

qt_standard_project_setup()

qt_add_executable(btchat
    chat.cpp chat.h chat.ui
    chatclient.cpp chatclient.h
    chatserver.cpp chatserver.h
    main.cpp
    remoteselector.cpp remoteselector.h remoteselector.ui
)

set_target_properties(btchat PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(btchat PRIVATE
    Qt::Bluetooth
    Qt::Core
    Qt::Widgets
)

if (APPLE)
    # Using absolute path for shared plist files is a Ninja bug workaround
    get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../shared ABSOLUTE)
    if (IOS)
        set_target_properties(btchat PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.ios.plist"
        )
    else()
        set_target_properties(btchat PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.macos.plist"
        )
    endif()
endif()

install(TARGETS btchat
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

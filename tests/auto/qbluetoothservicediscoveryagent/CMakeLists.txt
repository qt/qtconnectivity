# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qbluetoothservicediscoveryagent Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothservicediscoveryagent
    SOURCES
        tst_qbluetoothservicediscoveryagent.cpp
    LIBRARIES
        Qt::Bluetooth
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qbluetoothservicediscoveryagent CONDITION MACOS
    LIBRARIES
        Qt::Widgets
)

set_target_properties(tst_qbluetoothservicediscoveryagent PROPERTIES
    MACOSX_BUNDLE TRUE
)

if (APPLE AND NOT IOS)
    # Ninja has trouble with relative paths, convert to absolute as a workaround
    get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../shared ABSOLUTE)
    set_target_properties(tst_qbluetoothservicediscoveryagent PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.macos.plist"
    )
endif()

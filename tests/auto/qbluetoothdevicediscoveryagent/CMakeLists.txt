#####################################################################
## tst_qbluetoothdevicediscoveryagent Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothdevicediscoveryagent
    SOURCES
        tst_qbluetoothdevicediscoveryagent.cpp
    PUBLIC_LIBRARIES
        Qt::BluetoothPrivate
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qbluetoothdevicediscoveryagent CONDITION MACOS
    PUBLIC_LIBRARIES
        Qt::Widgets
)

set_target_properties(tst_qbluetoothdevicediscoveryagent PROPERTIES
    MACOSX_BUNDLE TRUE
)

if (APPLE AND NOT IOS)
    # Ninja has trouble with relative paths, convert to absolute as a workaround
    get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../shared ABSOLUTE)
    set_target_properties(tst_qbluetoothdevicediscoveryagent PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.macos.plist"
    )
endif()

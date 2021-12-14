#####################################################################
## tst_qlowenergydescriptor Test:
#####################################################################

qt_internal_add_test(tst_qlowenergydescriptor
    SOURCES
        tst_qlowenergydescriptor.cpp
    PUBLIC_LIBRARIES
        Qt::Bluetooth
)

set_target_properties(tst_qlowenergydescriptor PROPERTIES
    MACOSX_BUNDLE TRUE
)

if (APPLE AND NOT IOS)
    # Ninja has trouble with relative paths, convert to absolute as a workaround
    get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../shared ABSOLUTE)
    set_target_properties(tst_qlowenergydescriptor PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.macos.plist"
    )
endif()

cmake_minimum_required(VERSION 3.16...3.21)

if(NOT TARGET Qt::Bluetooth)
    # for standalone build (and the only way for iOS)
    project(tst_qlowenergycontroller_device LANGUAGES CXX)

    set(CMAKE_AUTOMOC ON)

    find_package(Qt6 REQUIRED COMPONENTS Bluetooth Core Test Gui)

    qt_add_executable(
        tst_qlowenergycontroller_device
            tst_qlowenergycontroller_device.cpp
    )
    target_link_libraries(
        tst_qlowenergycontroller_device
    PUBLIC
        Qt::Core
        Qt::Bluetooth
        Qt::Test
        Qt::Gui
    )

else()

qt_internal_add_test(tst_qlowenergycontroller_device
    SOURCES
        tst_qlowenergycontroller_device.cpp
    PUBLIC_LIBRARIES
        Qt::Bluetooth
)

qt_internal_extend_target(tst_qlowenergycontroller_device
    CONDITION ANDROID AND NOT ANDROID_EMBEDDED
    DEFINES
        QT_ANDROID_BLUETOOTH
)

endif()

set_target_properties(tst_qlowenergycontroller_device PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

if(APPLE)
    # Ninja has trouble with relative paths, convert to absolute as a workaround
    get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../shared ABSOLUTE)
    if(IOS)
        set_target_properties(tst_qlowenergycontroller_device PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.ios.plist"
        )
    else()
        set_target_properties(tst_qlowenergycontroller_device PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.macos.plist"
        )
    endif()
endif()

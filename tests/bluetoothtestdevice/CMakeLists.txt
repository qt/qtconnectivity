#####################################################################
## bluetoothtestdevice Tool:
#####################################################################

cmake_minimum_required(VERSION 3.16...3.21)

if(NOT TARGET Qt::Bluetooth)
    # for standalone build
    project(bluetoothtestdevice LANGUAGES CXX)

    set(CMAKE_AUTOMOC ON)

    find_package(Qt6 COMPONENTS Bluetooth Core)
    if(ANDROID OR IOS)
        find_package(Qt6 COMPONENTS Gui)
    endif()

    qt_add_executable(
        bluetoothtestdevice
            bluetoothtestdevice.cpp
    )

else()

    qt_internal_add_executable(bluetoothtestdevice
        SOURCES
            bluetoothtestdevice.cpp
    )
endif()

set_target_properties(bluetoothtestdevice PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

if(APPLE)
    if(IOS)
        set_target_properties(bluetoothtestdevice PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.ios.plist"
        )
    else()
        set_target_properties(bluetoothtestdevice PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.macos.plist"
        )
    endif()
endif()

target_link_libraries(
        bluetoothtestdevice
    PUBLIC
        Qt::Core
        Qt::Bluetooth
)

if(ANDROID OR IOS)
    target_link_libraries(
            bluetoothtestdevice
            PUBLIC
            Qt::Gui
    )
endif()

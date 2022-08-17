cmake_minimum_required(VERSION 3.16)
project(heartrate-server LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/bluetooth/heartrate-server")

find_package(Qt6 REQUIRED COMPONENTS Bluetooth Core)

if(ANDROID OR IOS)
    find_package(Qt6 REQUIRED COMPONENTS Gui)
endif()

qt_add_executable(heartrate-server
    main.cpp
)

set_target_properties(heartrate-server PROPERTIES
    MACOSX_BUNDLE TRUE
)

target_link_libraries(heartrate-server PUBLIC
    Qt::Bluetooth
    Qt::Core
)

if(ANDROID OR IOS)
    target_link_libraries(heartrate-server PUBLIC
        Qt::Gui
    )
endif()

if (APPLE)
    if (IOS)
        set_target_properties(heartrate-server PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        )
    else()
        # Using absolute path for shared plist files is a Ninja bug workaround
        get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../shared ABSOLUTE)
        set_target_properties(heartrate-server PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.macos.plist"
        )
    endif()
endif()

install(TARGETS heartrate-server
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

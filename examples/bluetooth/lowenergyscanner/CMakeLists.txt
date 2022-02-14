cmake_minimum_required(VERSION 3.16)
project(lowenergyscanner LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/bluetooth/lowenergyscanner")

find_package(Qt6 REQUIRED COMPONENTS Bluetooth Core Gui Quick)

qt_add_executable(lowenergyscanner
    characteristicinfo.cpp characteristicinfo.h
    device.cpp device.h
    deviceinfo.cpp deviceinfo.h
    main.cpp
    serviceinfo.cpp serviceinfo.h
)

set_target_properties(lowenergyscanner PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_include_directories(lowenergyscanner PUBLIC
    .
)

target_link_libraries(lowenergyscanner PUBLIC
    Qt::Bluetooth
    Qt::Core
    Qt::Gui
    Qt::Quick
)

if (APPLE)
    if (IOS)
        set_target_properties(lowenergyscanner PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        )
    else()
        # Using absolute path for shared plist files is a Ninja bug workaround
        get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../shared ABSOLUTE)
        set_target_properties(lowenergyscanner PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.macos.plist"
        )
    endif()
endif()

# Resources:
set(resources_resource_files
    "assets/Characteristics.qml"
    "assets/Dialog.qml"
    "assets/Header.qml"
    "assets/Label.qml"
    "assets/Menu.qml"
    "assets/Services.qml"
    "assets/busy_dark.png"
    "assets/main.qml"
)

qt6_add_resources(lowenergyscanner "resources"
    PREFIX
        "/"
    FILES
        ${resources_resource_files}
)

install(TARGETS lowenergyscanner
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

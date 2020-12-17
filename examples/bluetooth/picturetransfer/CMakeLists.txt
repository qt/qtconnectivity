# Generated from picturetransfer.pro.

cmake_minimum_required(VERSION 3.14)
project(qml_picturetransfer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/bluetooth/picturetransfer")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS Bluetooth)

qt_add_executable(qml_picturetransfer
    filetransfer.cpp filetransfer.h
    main.cpp
)
set_target_properties(qml_picturetransfer PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(qml_picturetransfer PUBLIC
    Qt::Bluetooth
    Qt::Core
    Qt::Gui
    Qt::Quick
)


# Resources:
set(qmltransfer_resource_files
    "Button.qml"
    "DeviceDiscovery.qml"
    "FileSending.qml"
    "PictureSelector.qml"
    "background.png"
    "bttransfer.qml"
)

qt6_add_resources(qml_picturetransfer "qmltransfer"
    PREFIX
        "/"
    FILES
        ${qmltransfer_resource_files}
)

install(TARGETS qml_picturetransfer
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

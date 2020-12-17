# Generated from btfiletransfer.pro.

cmake_minimum_required(VERSION 3.14)
project(btfiletransfer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/bluetooth/btfiletransfer")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Bluetooth)
find_package(Qt6 COMPONENTS Widgets)

qt_add_executable(btfiletransfer
    main.cpp
    pindisplay.cpp pindisplay.h pindisplay.ui
    progress.cpp progress.h progress.ui
    remoteselector.cpp remoteselector.h remoteselector.ui
)
set_target_properties(btfiletransfer PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(btfiletransfer PUBLIC
    Qt::Bluetooth
    Qt::Core
    Qt::Widgets
)


# Resources:
set(btfiletransfer_resource_files
    "busy.gif"
    "pairing.gif"
)

qt6_add_resources(btfiletransfer "btfiletransfer"
    PREFIX
        "/icons"
    FILES
        ${btfiletransfer_resource_files}
)

install(TARGETS btfiletransfer
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

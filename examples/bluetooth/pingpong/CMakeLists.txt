cmake_minimum_required(VERSION 3.16)
project(btpingpong LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/bluetooth/pingpong")

find_package(Qt6 REQUIRED COMPONENTS Bluetooth Core Gui Quick)

qt_add_executable(btpingpong
    main.cpp
    pingpong.cpp pingpong.h
)

set_target_properties(btpingpong PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    OUTPUT_NAME "pingpong"
)

target_link_libraries(btpingpong PUBLIC
    Qt::Bluetooth
    Qt::Core
    Qt::Gui
    Qt::Quick
)

# Resources:
set(resource_resource_files
    "assets/Board.qml"
    "assets/Dialog.qml"
    "assets/Menu.qml"
    "assets/main.qml"
)

qt6_add_resources(btpingpong "resource"
    PREFIX
        "/"
    FILES
        ${resource_resource_files}
)

install(TARGETS btpingpong
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

cmake_minimum_required(VERSION 3.14)
project(pingpong LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/bluetooth/pingpong")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS Bluetooth)

qt_add_executable(pingpong
    main.cpp
    pingpong.cpp pingpong.h
)
set_target_properties(pingpong PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(pingpong PUBLIC
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

qt6_add_resources(pingpong "resource"
    PREFIX
        "/"
    FILES
        ${resource_resource_files}
)

if(ANDROID)
    find_package(Qt6 COMPONENTS AndroidExtras)
    target_link_libraries(pingpong PUBLIC
        Qt::AndroidExtras
    )
endif()

install(TARGETS pingpong
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

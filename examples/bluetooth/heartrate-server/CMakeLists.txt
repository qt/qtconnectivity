# Generated from heartrate-server.pro.

cmake_minimum_required(VERSION 3.14)
project(heartrate-server LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples/bluetooth/heartrate-server")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Bluetooth)

add_qt_gui_executable(heartrate-server
    main.cpp
)
target_link_libraries(heartrate-server PUBLIC
    Qt::Bluetooth
    Qt::Core
)

install(TARGETS heartrate-server
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

# Generated from btscanner.pro.

cmake_minimum_required(VERSION 3.14)
project(btscanner LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples/bluetooth/btscanner")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Bluetooth)
find_package(Qt6 COMPONENTS Widgets)

add_qt_gui_executable(btscanner
    device.cpp device.h device.ui
    main.cpp
    service.cpp service.h service.ui
)
target_link_libraries(btscanner PUBLIC
    Qt::Bluetooth
    Qt::Core
    Qt::Widgets
)

install(TARGETS btscanner
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

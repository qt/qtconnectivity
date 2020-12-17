# Generated from btscanner.pro.

cmake_minimum_required(VERSION 3.14)
project(btscanner LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/bluetooth/btscanner")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Bluetooth)
find_package(Qt6 COMPONENTS Widgets)

qt_add_executable(btscanner
    device.cpp device.h device.ui
    main.cpp
    service.cpp service.h service.ui
)
set_target_properties(btscanner PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
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

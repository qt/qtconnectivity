cmake_minimum_required(VERSION 3.16)
project(heartrate-server LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/bluetooth/heartrate-server")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Bluetooth)
if(ANDROID)
    find_package(Qt6 COMPONENTS Gui)
endif()

qt_add_executable(heartrate-server
    main.cpp
)
set_target_properties(heartrate-server PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(heartrate-server PUBLIC
    Qt::Bluetooth
    Qt::Core
)

if(ANDROID)
    target_link_libraries(heartrate-server PUBLIC
        Qt::Gui
    )
endif()

install(TARGETS heartrate-server
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

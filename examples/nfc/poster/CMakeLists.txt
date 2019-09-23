# Generated from poster.pro.

cmake_minimum_required(VERSION 3.14)
project(qml_poster LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS Network)
find_package(Qt6 COMPONENTS Nfc)

add_qt_gui_executable(qml_poster
    qmlposter.cpp
)
target_link_libraries(qml_poster PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Nfc
    Qt::Qml
    Qt::Quick
)

# Resources:
set(poster_resource_files
    "poster.qml"
)

qt6_add_resources(qml_poster "poster"
    PREFIX
        "/"
    FILES
        ${poster_resource_files}
)


install(TARGETS qml_poster
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

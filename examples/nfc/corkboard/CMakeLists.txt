# Generated from corkboard.pro.

cmake_minimum_required(VERSION 3.14)
project(qml_corkboard LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples/nfc/corkboard")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS Nfc)

add_qt_gui_executable(qml_corkboard
    main.cpp
)
target_link_libraries(qml_corkboard PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Nfc
    Qt::Quick
)


# Resources:
set(corkboard_resource_files
    "Mode.qml"
    "NfcFlag.png"
    "cork.jpg"
    "corkboards.qml"
    "note-yellow.png"
    "tack.png"
)

qt6_add_resources(qml_corkboard "corkboard"
    PREFIX
        "/"
    FILES
        ${corkboard_resource_files}
)

install(TARGETS qml_corkboard
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

#####################################################################
## bttestui Binary:
#####################################################################

cmake_minimum_required(VERSION 3.16...3.21)

# Resources:
set(bttest_resource_files
    "Button.qml"
    "main.qml"
)

if(NOT TARGET Qt::Bluetooth)
    # for standalone build
    project(bttestui LANGUAGES CXX)

    set(CMAKE_AUTOMOC ON)

    find_package(Qt6 REQUIRED COMPONENTS Bluetooth Core Gui Quick)

    qt_add_executable(bttestui
            btlocaldevice.cpp btlocaldevice.h
            main.cpp
    )

    target_link_libraries(bttestui PUBLIC
        Qt::Bluetooth
        Qt::Gui
        Qt::Quick
    )

    qt_add_resources(bttestui "bttest"
        PREFIX
            "/"
        FILES
            ${bttest_resource_files}
    )
else()
    # in-tree build
    qt_internal_add_executable(bttestui
        GUI
        SOURCES
            btlocaldevice.cpp btlocaldevice.h
            main.cpp
        PUBLIC_LIBRARIES
            Qt::Bluetooth
            Qt::Gui
            Qt::Quick
    )

    qt_internal_add_resource(bttestui "bttest"
        PREFIX
            "/"
        FILES
            ${bttest_resource_files}
    )
endif()


#### Keys ignored in scope 1:.:.:bttestui.pro:<TRUE>:
# OTHER_FILES = "main.qml" "Button.qml"
# TEMPLATE = "app"

## Scopes:
#####################################################################


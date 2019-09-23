# Generated from server.pro.

#####################################################################
## server Binary:
#####################################################################

add_qt_executable(server
    GUI
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} # special case
    SOURCES
        qlowenergycontroller-gattserver.cpp
    PUBLIC_LIBRARIES
        Qt::Bluetooth
)

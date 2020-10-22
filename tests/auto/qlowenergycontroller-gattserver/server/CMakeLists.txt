# Generated from server.pro.

#####################################################################
## server Binary:
#####################################################################

qt_internal_add_executable(server
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" # special case
    SOURCES
        qlowenergycontroller-gattserver.cpp
    PUBLIC_LIBRARIES
        Qt::Bluetooth
)

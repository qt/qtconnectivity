#####################################################################
## server Binary:
#####################################################################

qt_internal_add_executable(qlecontroller-server
    GUI
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    SOURCES
        qlowenergycontroller-gattserver.cpp
    PUBLIC_LIBRARIES
        Qt::Bluetooth
)

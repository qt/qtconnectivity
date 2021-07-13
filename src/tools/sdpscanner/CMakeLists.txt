#####################################################################
## sdpscanner Tool:
#####################################################################

if(NOT TARGET PkgConfig::BLUEZ)
     qt_find_package(BlueZ PROVIDED_TARGETS PkgConfig::BLUEZ)
endif()


qt_get_tool_target_name(target_name sdpscanner)
qt_internal_add_tool(${target_name}
    TOOLS_TARGET Bluetooth
    INSTALL_DIR "${INSTALL_LIBEXECDIR}"
    SOURCES
        main.cpp
    PUBLIC_LIBRARIES
        PkgConfig::BLUEZ
)

#### Keys ignored in scope 1:.:.:sdpscanner.pro:<TRUE>:
# QT_FOR_CONFIG = "bluetooth-private"
# TEMPLATE = "app"

## Scopes:
#####################################################################

# Generated from sdpscanner.pro.

#####################################################################
## sdpscanner Tool:
#####################################################################

# special case begin
if(NOT TARGET PkgConfig::BLUEZ)
     qt_find_package(BlueZ PROVIDED_TARGETS PkgConfig::BLUEZ)
endif()
# special case end


qt_get_tool_target_name(target_name sdpscanner)
qt_internal_add_tool(${target_name}
    TOOLS_TARGET Bluetooth # special case
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

# Generated from sdpscanner.pro.

#####################################################################
## sdpscanner Tool:
#####################################################################

qt_add_tool(sdpscanner
    TOOLS_TARGET Bluetooth # special case
    SOURCES
        main.cpp
    PUBLIC_LIBRARIES
        PkgConfig::BlueZ
)

#### Keys ignored in scope 1:.:.:sdpscanner.pro:<TRUE>:
# QT_FOR_CONFIG = "bluetooth-private"
# TEMPLATE = "app"

## Scopes:
#####################################################################

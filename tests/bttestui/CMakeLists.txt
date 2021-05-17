#####################################################################
## bttestui Binary:
#####################################################################

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

# Resources:
set(bttest_resource_files
    "Button.qml"
    "main.qml"
)

qt_internal_add_resource(bttestui "bttest"
    PREFIX
        "/"
    FILES
        ${bttest_resource_files}
)


#### Keys ignored in scope 1:.:.:bttestui.pro:<TRUE>:
# OTHER_FILES = "main.qml" "Button.qml"
# TEMPLATE = "app"

## Scopes:
#####################################################################


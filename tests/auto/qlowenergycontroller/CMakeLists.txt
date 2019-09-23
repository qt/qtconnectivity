# Generated from qlowenergycontroller.pro.

#####################################################################
## tst_qlowenergycontroller Test:
#####################################################################

add_qt_test(tst_qlowenergycontroller
    SOURCES
        tst_qlowenergycontroller.cpp
    LIBRARIES
        Qt::BluetoothPrivate
    PUBLIC_LIBRARIES
        Qt::Bluetooth
)

## Scopes:
#####################################################################

extend_target(tst_qlowenergycontroller CONDITION ANDROID
    PUBLIC_LIBRARIES
        Qt::Widgets
)

extend_target(tst_qlowenergycontroller CONDITION APPLE_IOS OR APPLE_OSX
    PUBLIC_LIBRARIES
        Qt::Widgets
)

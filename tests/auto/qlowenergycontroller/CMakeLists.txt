# Generated from qlowenergycontroller.pro.

#####################################################################
## tst_qlowenergycontroller Test:
#####################################################################

qt_add_test(tst_qlowenergycontroller
    SOURCES
        tst_qlowenergycontroller.cpp
    PUBLIC_LIBRARIES
        Qt::BluetoothPrivate
)

## Scopes:
#####################################################################

qt_extend_target(tst_qlowenergycontroller CONDITION ANDROID
    PUBLIC_LIBRARIES
        Qt::Widgets
)

qt_extend_target(tst_qlowenergycontroller CONDITION APPLE_IOS OR APPLE_OSX
    PUBLIC_LIBRARIES
        Qt::Widgets
)

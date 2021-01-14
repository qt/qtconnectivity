#####################################################################
## tst_qlowenergycontroller Test:
#####################################################################

qt_internal_add_test(tst_qlowenergycontroller
    SOURCES
        tst_qlowenergycontroller.cpp
    PUBLIC_LIBRARIES
        Qt::BluetoothPrivate
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qlowenergycontroller CONDITION ANDROID
    PUBLIC_LIBRARIES
        Qt::Widgets
)

qt_internal_extend_target(tst_qlowenergycontroller CONDITION IOS OR MACOS
    PUBLIC_LIBRARIES
        Qt::Widgets
)

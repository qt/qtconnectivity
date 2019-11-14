# Generated from test.pro.

#####################################################################
## tst_qlowenergycontroller-gattserver Test:
#####################################################################

qt_add_test(tst_qlowenergycontroller-gattserver
    SOURCES
        tst_qlowenergycontroller-gattserver.cpp
    LIBRARIES
        Qt::BluetoothPrivate
    PUBLIC_LIBRARIES
        Qt::Bluetooth
)

## Scopes:
#####################################################################

qt_extend_target(tst_qlowenergycontroller-gattserver CONDITION QT_FEATURE_linux_crypto_api
    DEFINES
        CONFIG_LINUX_CRYPTO_API
)

qt_extend_target(tst_qlowenergycontroller-gattserver CONDITION QT_FEATURE_bluez_le
    DEFINES
        CONFIG_BLUEZ_LE
)

qt_extend_target(tst_qlowenergycontroller-gattserver CONDITION boot2qt
    DEFINES
        CHECK_CMAC_SUPPORT
)

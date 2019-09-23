# Generated from test.pro.

#####################################################################
## tst_qlowenergycontroller-gattserver Test:
#####################################################################

add_qt_test(tst_qlowenergycontroller-gattserver
    SOURCES
        tst_qlowenergycontroller-gattserver.cpp
    LIBRARIES
        Qt::BluetoothPrivate
    PUBLIC_LIBRARIES
        Qt::Bluetooth
)

## Scopes:
#####################################################################

extend_target(tst_qlowenergycontroller-gattserver CONDITION QT_FEATURE_linux_crypto_api
    DEFINES
        CONFIG_LINUX_CRYPTO_API
)

extend_target(tst_qlowenergycontroller-gattserver CONDITION QT_FEATURE_bluez_le
    DEFINES
        CONFIG_BLUEZ_LE
)

extend_target(tst_qlowenergycontroller-gattserver CONDITION boot2qt
    DEFINES
        CHECK_CMAC_SUPPORT
)

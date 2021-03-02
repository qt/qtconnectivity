#####################################################################
## tst_qbluetoothdevicediscoveryagent Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothdevicediscoveryagent
    SOURCES
        tst_qbluetoothdevicediscoveryagent.cpp
    PUBLIC_LIBRARIES
        Qt::BluetoothPrivate
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qbluetoothdevicediscoveryagent CONDITION MACOS
    PUBLIC_LIBRARIES
        Qt::Widgets
)

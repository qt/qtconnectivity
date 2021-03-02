#####################################################################
## tst_qbluetoothservicediscoveryagent Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothservicediscoveryagent
    SOURCES
        tst_qbluetoothservicediscoveryagent.cpp
    PUBLIC_LIBRARIES
        Qt::Bluetooth
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qbluetoothservicediscoveryagent CONDITION MACOS
    PUBLIC_LIBRARIES
        Qt::Widgets
)

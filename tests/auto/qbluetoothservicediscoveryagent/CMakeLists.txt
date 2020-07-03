# Generated from qbluetoothservicediscoveryagent.pro.

#####################################################################
## tst_qbluetoothservicediscoveryagent Test:
#####################################################################

qt_add_test(tst_qbluetoothservicediscoveryagent
    SOURCES
        tst_qbluetoothservicediscoveryagent.cpp
    PUBLIC_LIBRARIES
        Qt::Bluetooth
        Qt::Concurrent
)

## Scopes:
#####################################################################

qt_extend_target(tst_qbluetoothservicediscoveryagent CONDITION APPLE_OSX
    PUBLIC_LIBRARIES
        Qt::Widgets
)

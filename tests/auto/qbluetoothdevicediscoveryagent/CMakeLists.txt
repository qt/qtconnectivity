# Generated from qbluetoothdevicediscoveryagent.pro.

#####################################################################
## tst_qbluetoothdevicediscoveryagent Test:
#####################################################################

add_qt_test(tst_qbluetoothdevicediscoveryagent
    SOURCES
        tst_qbluetoothdevicediscoveryagent.cpp
    LIBRARIES
        Qt::BluetoothPrivate
    PUBLIC_LIBRARIES
        Qt::Bluetooth
        Qt::Concurrent
)

## Scopes:
#####################################################################

extend_target(tst_qbluetoothdevicediscoveryagent CONDITION APPLE_OSX
    PUBLIC_LIBRARIES
        Qt::Widgets
)

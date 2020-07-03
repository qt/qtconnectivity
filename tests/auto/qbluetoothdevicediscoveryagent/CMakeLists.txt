# Generated from qbluetoothdevicediscoveryagent.pro.

#####################################################################
## tst_qbluetoothdevicediscoveryagent Test:
#####################################################################

qt_add_test(tst_qbluetoothdevicediscoveryagent
    SOURCES
        tst_qbluetoothdevicediscoveryagent.cpp
    PUBLIC_LIBRARIES
        Qt::BluetoothPrivate
        Qt::Concurrent
)

## Scopes:
#####################################################################

qt_extend_target(tst_qbluetoothdevicediscoveryagent CONDITION APPLE_OSX
    PUBLIC_LIBRARIES
        Qt::Widgets
)

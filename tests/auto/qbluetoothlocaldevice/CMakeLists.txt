# Generated from qbluetoothlocaldevice.pro.

#####################################################################
## tst_qbluetoothlocaldevice Test:
#####################################################################

qt_add_test(tst_qbluetoothlocaldevice
    SOURCES
        tst_qbluetoothlocaldevice.cpp
    PUBLIC_LIBRARIES
        Qt::BluetoothPrivate
        Qt::Concurrent
)

## Scopes:
#####################################################################

qt_extend_target(tst_qbluetoothlocaldevice CONDITION APPLE_OSX
    PUBLIC_LIBRARIES
        Qt::Widgets
)

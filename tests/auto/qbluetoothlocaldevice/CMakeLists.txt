# Generated from qbluetoothlocaldevice.pro.

#####################################################################
## tst_qbluetoothlocaldevice Test:
#####################################################################

qt_add_test(tst_qbluetoothlocaldevice
    SOURCES
        tst_qbluetoothlocaldevice.cpp
    LIBRARIES
        Qt::BluetoothPrivate
    PUBLIC_LIBRARIES
        Qt::Bluetooth
        Qt::Concurrent
)

## Scopes:
#####################################################################

qt_extend_target(tst_qbluetoothlocaldevice CONDITION APPLE_OSX
    PUBLIC_LIBRARIES
        Qt::Widgets
)

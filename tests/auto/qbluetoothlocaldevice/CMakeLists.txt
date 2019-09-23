# Generated from qbluetoothlocaldevice.pro.

#####################################################################
## tst_qbluetoothlocaldevice Test:
#####################################################################

add_qt_test(tst_qbluetoothlocaldevice
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

extend_target(tst_qbluetoothlocaldevice CONDITION APPLE_OSX
    PUBLIC_LIBRARIES
        Qt::Widgets
)

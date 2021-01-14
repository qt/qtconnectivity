#####################################################################
## tst_qbluetoothlocaldevice Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothlocaldevice
    SOURCES
        tst_qbluetoothlocaldevice.cpp
    PUBLIC_LIBRARIES
        Qt::BluetoothPrivate
        Qt::Concurrent
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qbluetoothlocaldevice CONDITION MACOS
    PUBLIC_LIBRARIES
        Qt::Widgets
)

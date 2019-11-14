# Generated from qbluetoothserver.pro.

#####################################################################
## tst_qbluetoothserver Test:
#####################################################################

qt_add_test(tst_qbluetoothserver
    SOURCES
        tst_qbluetoothserver.cpp
    LIBRARIES
        Qt::BluetoothPrivate
    PUBLIC_LIBRARIES
        Qt::Bluetooth
        Qt::Concurrent
)

#### Keys ignored in scope 1:.:.:qbluetoothserver.pro:<TRUE>:
# OTHER_FILES = "README.txt"

## Scopes:
#####################################################################

qt_extend_target(tst_qbluetoothserver CONDITION APPLE_OSX
    PUBLIC_LIBRARIES
        Qt::Widgets
)

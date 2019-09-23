# Generated from qbluetoothsocket.pro.

#####################################################################
## tst_qbluetoothsocket Test:
#####################################################################

add_qt_test(tst_qbluetoothsocket
    SOURCES
        tst_qbluetoothsocket.cpp
    LIBRARIES
        Qt::BluetoothPrivate
    PUBLIC_LIBRARIES
        Qt::Bluetooth
        Qt::Concurrent
        Qt::Network
)

#### Keys ignored in scope 1:.:.:qbluetoothsocket.pro:<TRUE>:
# OTHER_FILES = "README.txt"
# testcase.timeout = "250"

## Scopes:
#####################################################################

extend_target(tst_qbluetoothsocket CONDITION APPLE_OSX
    DEFINES
        QT_OSX_BLUETOOTH
    PUBLIC_LIBRARIES
        Qt::Widgets
)

extend_target(tst_qbluetoothsocket CONDITION ANDROID AND NOT ANDROID_EMBEDDED
    DEFINES
        QT_ANDROID_BLUETOOTH
)

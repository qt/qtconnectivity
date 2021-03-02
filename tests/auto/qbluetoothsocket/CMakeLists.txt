#####################################################################
## tst_qbluetoothsocket Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothsocket
    SOURCES
        tst_qbluetoothsocket.cpp
    PUBLIC_LIBRARIES
        Qt::BluetoothPrivate
        Qt::Network
)

#### Keys ignored in scope 1:.:.:qbluetoothsocket.pro:<TRUE>:
# OTHER_FILES = "README.txt"
# testcase.timeout = "250"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qbluetoothsocket CONDITION MACOS
    DEFINES
        QT_OSX_BLUETOOTH
    PUBLIC_LIBRARIES
        Qt::Widgets
)

qt_internal_extend_target(tst_qbluetoothsocket CONDITION ANDROID AND NOT ANDROID_EMBEDDED
    DEFINES
        QT_ANDROID_BLUETOOTH
)

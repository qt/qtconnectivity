#####################################################################
## tst_qbluetoothserver Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothserver
    SOURCES
        tst_qbluetoothserver.cpp
    PUBLIC_LIBRARIES
        Qt::BluetoothPrivate
)

#### Keys ignored in scope 1:.:.:qbluetoothserver.pro:<TRUE>:
# OTHER_FILES = "README.txt"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qbluetoothserver CONDITION MACOS
    PUBLIC_LIBRARIES
        Qt::Widgets
)

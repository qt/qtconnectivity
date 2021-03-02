#####################################################################
## tst_qbluetoothaddress Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothaddress
    SOURCES
        tst_qbluetoothaddress.cpp
    PUBLIC_LIBRARIES
        Qt::Bluetooth
)

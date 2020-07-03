# Generated from qbluetoothserviceinfo.pro.

#####################################################################
## tst_qbluetoothserviceinfo Test:
#####################################################################

qt_add_test(tst_qbluetoothserviceinfo
    SOURCES
        tst_qbluetoothserviceinfo.cpp
    PUBLIC_LIBRARIES
        Qt::Bluetooth
        Qt::Concurrent
)

## Scopes:
#####################################################################

qt_extend_target(tst_qbluetoothserviceinfo CONDITION APPLE_OSX
    PUBLIC_LIBRARIES
        Qt::Widgets
)

qt_extend_target(tst_qbluetoothserviceinfo CONDITION ANDROID AND NOT ANDROID_EMBEDDED
    DEFINES
        QT_ANDROID_BLUETOOTH
)

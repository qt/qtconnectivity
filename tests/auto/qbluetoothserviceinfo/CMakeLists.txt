#####################################################################
## tst_qbluetoothserviceinfo Test:
#####################################################################

qt_internal_add_test(tst_qbluetoothserviceinfo
    SOURCES
        tst_qbluetoothserviceinfo.cpp
    PUBLIC_LIBRARIES
        Qt::Bluetooth
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qbluetoothserviceinfo CONDITION MACOS
    PUBLIC_LIBRARIES
        Qt::Widgets
)

qt_internal_extend_target(tst_qbluetoothserviceinfo CONDITION ANDROID AND NOT ANDROID_EMBEDDED
    DEFINES
        QT_ANDROID_BLUETOOTH
)

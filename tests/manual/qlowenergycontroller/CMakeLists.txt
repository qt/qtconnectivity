qt_internal_add_test(tst_qlowenergycontroller_device
    SOURCES
        tst_qlowenergycontroller_device.cpp
    PUBLIC_LIBRARIES
        Qt::Bluetooth
)

qt_internal_extend_target(tst_qlowenergycontroller_device
    CONDITION ANDROID AND NOT ANDROID_EMBEDDED
    DEFINES
        QT_ANDROID_BLUETOOTH
)


TARGET = QtBluetooth
QT = core
QT_PRIVATE = concurrent


QMAKE_DOCS = $$PWD/doc/qtbluetooth.qdocconf
OTHER_FILES += doc/src/*.qdoc   # show .qdoc files in Qt Creator

load(qt_module)

PUBLIC_HEADERS += \
    qbluetoothglobal.h \
    qbluetoothaddress.h\
    qbluetoothhostinfo.h \
    qbluetoothuuid.h\
    qbluetoothdeviceinfo.h\
    qbluetoothserviceinfo.h\
    qbluetoothdevicediscoveryagent.h\
    qbluetoothservicediscoveryagent.h\
    qbluetoothsocket.h\
    qbluetoothserver.h \
    qbluetooth.h \
    qbluetoothlocaldevice.h \
    qbluetoothtransfermanager.h \
    qbluetoothtransferrequest.h \
    qlowenergyserviceinfo.h \
    qlowenergycharacteristicinfo.h \
    qlowenergydescriptorinfo.h \
    qbluetoothtransferreply.h \
    qlowenergycontroller.h

PRIVATE_HEADERS += \
    qbluetoothaddress_p.h\
    qbluetoothhostinfo_p.h \
    qbluetoothdeviceinfo_p.h\
    qbluetoothserviceinfo_p.h\
    qbluetoothdevicediscoveryagent_p.h\
    qbluetoothservicediscoveryagent_p.h\
    qbluetoothsocket_p.h\
    qbluetoothserver_p.h\
    qbluetoothtransferreply_p.h \
    qbluetoothtransferrequest_p.h \
    qprivatelinearbuffer_p.h \
    qbluetoothlocaldevice_p.h \
    qlowenergyserviceinfo_p.h \
    qlowenergycharacteristicinfo_p.h \
    qlowenergyprocess_p.h \
    qlowenergydescriptorinfo_p.h \
    qlowenergycontroller_p.h

SOURCES += \
    qbluetoothaddress.cpp\
    qbluetoothhostinfo.cpp \
    qbluetoothuuid.cpp\
    qbluetoothdeviceinfo.cpp\
    qbluetoothserviceinfo.cpp\
    qbluetoothdevicediscoveryagent.cpp\
    qbluetoothservicediscoveryagent.cpp\
    qbluetoothsocket.cpp\
    qbluetoothserver.cpp \
    qbluetoothlocaldevice.cpp \
    qbluetooth.cpp \
    qbluetoothtransfermanager.cpp \
    qbluetoothtransferrequest.cpp \
    qbluetoothtransferreply.cpp \
    qlowenergyserviceinfo.cpp \
    qlowenergycharacteristicinfo.cpp \
    qlowenergydescriptorinfo.cpp \
    qlowenergycontroller.cpp

config_bluez:qtHaveModule(dbus) {
    QT *= dbus
    DEFINES += QT_BLUEZ_BLUETOOTH

    include(bluez/bluez.pri)

    PRIVATE_HEADERS += \
        qbluetoothtransferreply_bluez_p.h

    SOURCES += \
        qbluetoothserviceinfo_bluez.cpp \
        qbluetoothdevicediscoveryagent_bluez.cpp\
        qbluetoothservicediscoveryagent_bluez.cpp \
        qbluetoothsocket_bluez.cpp \
        qbluetoothserver_bluez.cpp \
        qbluetoothlocaldevice_bluez.cpp \
        qbluetoothtransferreply_bluez.cpp \
        qlowenergyprocess_bluez.cpp \
        qlowenergyserviceinfo_bluez.cpp \
        qlowenergycharacteristicinfo_bluez.cpp

} else:CONFIG(blackberry) {
    DEFINES += QT_QNX_BLUETOOTH

    include(qnx/qnx.pri)

    LIBS += -lbtapi
    config_btapi10_2_1 {
         DEFINES += QT_QNX_BT_BLUETOOTH
    }

    PRIVATE_HEADERS += \
        qbluetoothtransferreply_qnx_p.h

    SOURCES += \
        qbluetoothdevicediscoveryagent_qnx.cpp \
        qbluetoothlocaldevice_qnx.cpp \
        qbluetoothserviceinfo_qnx.cpp \
        qbluetoothservicediscoveryagent_qnx.cpp \
        qbluetoothsocket_qnx.cpp \
        qbluetoothserver_qnx.cpp \
        qbluetoothtransferreply_qnx.cpp \
        qlowenergycharacteristicinfo_qnx.cpp \
        qlowenergyserviceinfo_qnx.cpp \
        qlowenergyprocess_qnx.cpp

} else:android:!android-no-sdk {
    include(android/android.pri)
    DEFINES += QT_ANDROID_BLUETOOTH
    QT += core-private androidextras

    ANDROID_PERMISSIONS = \
        android.permission.BLUETOOTH \
        android.permission.BLUETOOTH_ADMIN
    ANDROID_BUNDLED_JAR_DEPENDENCIES = \
        jar/QtAndroidBluetooth-bundled.jar:org.qtproject.qt5.android.bluetooth.QtBluetoothBroadcastReceiver
    ANDROID_JAR_DEPENDENCIES = \
        jar/QtAndroidBluetooth.jar:org.qtproject.qt5.android.bluetooth.QtBluetoothBroadcastReceiver

    SOURCES += \
        qbluetoothdevicediscoveryagent_android.cpp \
        qbluetoothlocaldevice_android.cpp \
        qbluetoothserviceinfo_android.cpp \
        qbluetoothservicediscoveryagent_android.cpp \
        qbluetoothsocket_android.cpp \
        qbluetoothserver_android.cpp

} else {
    message("Unsupported bluetooth platform, will not build a working QBluetooth library")
    message("Either no Qt dBus found or no Bluez headers")
    SOURCES += \
        qbluetoothdevicediscoveryagent_p.cpp \
        qbluetoothlocaldevice_p.cpp \
        qbluetoothserviceinfo_p.cpp \
        qbluetoothservicediscoveryagent_p.cpp \
        qbluetoothsocket_p.cpp \
        qbluetoothserver_p.cpp \
        qlowenergyserviceinfo_p.cpp \
        qlowenergycharacteristicinfo_p.cpp \
        qlowenergyprocess_p.cpp
}

OTHER_FILES +=

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

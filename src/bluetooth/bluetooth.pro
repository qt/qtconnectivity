load(qt_module)

TARGET = QtBluetooth
QPRO_PWD = $PWD

CONFIG += module
MODULE_PRI = ../../modules/qt_bluetooth.pri

DEFINES += QT_BUILD_BT_LIB QT_MAKEDLL

load(qt_module_config)

QT += concurrent
QT -= gui

PUBLIC_HEADERS += \
    qbluetoothaddress.h\
    qbluetoothuuid.h\
    qbluetoothdeviceinfo.h\
    qbluetoothserviceinfo.h\
    qbluetoothdevicediscoveryagent.h\
    qbluetoothservicediscoveryagent.h\
    qbluetoothsocket.h\
    qrfcommserver.h \
    ql2capserver.h \
    qbluetooth.h \
    qbluetoothlocaldevice.h \
    ql2capsocket.h \
    qrfcommsocket.h \
    qbluetoothtransfermanager.h \
    qbluetoothtransferrequest.h \
    qbluetoothtransferreply.h

PRIVATE_HEADERS += \
    qbluetoothaddress_p.h\
    qbluetoothdeviceinfo_p.h\
    qbluetoothserviceinfo_p.h\
    qbluetoothdevicediscoveryagent_p.h\
    qbluetoothservicediscoveryagent_p.h\
    qbluetoothsocket_p.h\
    qrfcommserver_p.h \
    ql2capserver_p.h \
    qbluetoothtransferreply_p.h \
    qbluetoothtransferrequest_p.h \
    qprivatelinearbuffer_p.h

SOURCES += \
    qbluetoothaddress.cpp\
    qbluetoothuuid.cpp\
    qbluetoothdeviceinfo.cpp\
    qbluetoothserviceinfo.cpp\
    qbluetoothdevicediscoveryagent.cpp\
    qbluetoothservicediscoveryagent.cpp\
    qbluetoothsocket.cpp\
    qrfcommserver.cpp \
    ql2capserver.cpp \
    qbluetoothlocaldevice.cpp \
    qbluetooth.cpp \
    ql2capsocket.cpp \
    qrfcommsocket.cpp \
    qbluetoothtransfermanager.cpp \
    qbluetoothtransferrequest.cpp \
    qbluetoothtransferreply.cpp

contains(config_test_bluez, yes):contains(QT_CONFIG, dbus) {
    QT *= dbus
    DEFINES += QT_BLUEZ_BLUETOOTH

    include(bluez/bluez.pri)

    PRIVATE_HEADERS += \
        qbluetoothtransferreply_bluez_p.h \
        qbluetoothlocaldevice_p.h

    SOURCES += \
        qbluetoothserviceinfo_bluez.cpp \
        qbluetoothdevicediscoveryagent_bluez.cpp\
        qbluetoothservicediscoveryagent_bluez.cpp \
        qbluetoothsocket_bluez.cpp \
        qrfcommserver_bluez.cpp \
        qbluetoothlocaldevice_bluez.cpp \
        qbluetoothtransferreply_bluez.cpp \
        qbluetoothtransfermanager_bluez.cpp \
        ql2capserver_bluez.cpp

    contains(DEFINES,NOKIA_BT_SERVICES) {
        message("Enabling Nokia BT services")
        QT += serviceframework
    }
    contains(DEFINES,NOKIA_BT_PATCHES) {
        message("Enabling Nokia BT patches")
        LIBS += -lbluetooth
    }

} else {
    message("Unsupported bluetooth platform, will not build a working QBluetooth library")
    message("Either no Qt dBus found or no Bluez headers")
    SOURCES += \
        qbluetoothdevicediscoveryagent_p.cpp \
        qbluetoothlocaldevice_p.cpp \
        qbluetoothserviceinfo_p.cpp \
        qbluetoothservicediscoveryagent_p.cpp \
        qbluetoothsocket_p.cpp \
        ql2capserver_p.cpp \
        qrfcommserver_p.cpp \
        qbluetoothtransfermanager_p.cpp

}

INCLUDEPATH += $$PWD
INCLUDEPATH += ..

OTHER_FILES +=

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

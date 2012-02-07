load(qt_module)

TARGET = QtBluetooth
QPRO_PWD = $PWD

CONFIG += module
MODULE_PRI = ../../modules/qt_bluetooth.pri

DEFINES += QT_BUILD_BT_LIB QT_MAKEDLL

load(qt_module_config)

QT += concurrent

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
    qbluetoothtransferrequest_p.h

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

symbian {
    contains(S60_VERSION, 3.1) | contains(S60_VERSION, 3.2) {
        DEFINES += DO_NOT_BUILD_BLUETOOTH_SYMBIAN_BACKEND
        message("S60 3.1 or 3.2 sdk not supported by bluetooth")
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
}

symbian {
    !contains(DEFINES, DO_NOT_BUILD_BLUETOOTH_SYMBIAN_BACKEND) {
        DEFINES += QT_SYMBIAN_BLUETOOTH
        INCLUDEPATH += $$MW_LAYER_SYSTEMINCLUDE
        include(symbian/symbian.pri)

        PRIVATE_HEADERS += \
            qbluetoothtransferreply_symbian_p.h \
            qbluetoothlocaldevice_p.h

        SOURCES += \
            qbluetoothserviceinfo_symbian.cpp\
            qbluetoothdevicediscoveryagent_symbian.cpp\
            qbluetoothservicediscoveryagent_symbian.cpp\
            qbluetoothsocket_symbian.cpp\
            qrfcommserver_symbian.cpp \
            qbluetoothlocaldevice_symbian.cpp \
            qbluetoothtransfermanager_symbian.cpp \
            qbluetoothtransferreply_symbian.cpp \
            ql2capserver_symbian.cpp

        contains(S60_VERSION, 5.0) {
            message("NOTICE - START")
            message("Bluetooth backend needs SDK plugin from Forum Nokia for 5.0 SDK")
            message("NOTICE - END")
            LIBS *= -lirobex
        } else {
            LIBS *= -lobex
        }
        LIBS *= -lesock \
                -lbluetooth \
                -lsdpagent \
                -lsdpdatabase \
                -lestlib \
                -lbtengsettings \
                -lbtmanclient \
                -lbtdevice
    }
} else:contains(config_test_bluez, yes):contains(QT_CONFIG, dbus) {
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

    exists(/usr/include/test_framework_4711.h) {
        message(Activating Nokia Bluetooth Services)
        DEFINES += NOKIA_BT_SERVICES
        QT += serviceframework
    }

} else {
    message("Unsupported bluetooth platform, will not build a working QBluetooth library")
    message("Either no Qt dBus found, no bluez headers, or not symbian")
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

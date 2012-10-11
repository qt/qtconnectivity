TARGET = QtBluetooth
QT = core
QT_PRIVATE = concurrent

QMAKE_DOCS = $$PWD/../../doc/qt5.qdocconf

load(qt_module)

PUBLIC_HEADERS += \
    qbluetoothaddress.h\
    qbluetoothhostinfo.h \
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
    qbluetoothhostinfo_p.h \
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
    qbluetoothhostinfo.cpp \
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

config_bluez:contains(QT_CONFIG, dbus) {
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

OTHER_FILES +=

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

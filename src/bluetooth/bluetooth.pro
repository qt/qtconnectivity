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
    qbluetoothtransferreply.h

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
    qbluetoothlocaldevice_p.h

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
    qbluetoothtransferreply.cpp

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
        qbluetoothtransferreply_bluez.cpp

} else:CONFIG(blackberry) {
    DEFINES += QT_QNX_BLUETOOTH

    include(qnx/qnx.pri)

    PRIVATE_HEADERS += \
        qbluetoothtransferreply_qnx_p.h

    SOURCES += \
        qbluetoothdevicediscoveryagent_qnx.cpp \
        qbluetoothlocaldevice_qnx.cpp \
        qbluetoothserviceinfo_qnx.cpp \
        qbluetoothservicediscoveryagent_qnx.cpp \
        qbluetoothsocket_qnx.cpp \
        qbluetoothserver_qnx.cpp \
        qbluetoothtransferreply_qnx.cpp

} else {
    message("Unsupported bluetooth platform, will not build a working QBluetooth library")
    message("Either no Qt dBus found or no Bluez headers")
    SOURCES += \
        qbluetoothdevicediscoveryagent_p.cpp \
        qbluetoothlocaldevice_p.cpp \
        qbluetoothserviceinfo_p.cpp \
        qbluetoothservicediscoveryagent_p.cpp \
        qbluetoothsocket_p.cpp \
        qbluetoothserver_p.cpp

}

OTHER_FILES +=

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS



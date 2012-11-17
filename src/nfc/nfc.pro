TARGET = QtNfc
QT = core

load(qt_module)

# All classes in this module are in the QtNfc namespace.  Define the namespace which moc generated
# code will be in.
DEFINES += QT_BEGIN_MOC_NAMESPACE=\""namespace QtNfc {"\"
DEFINES += QT_END_MOC_NAMESPACE=\""}"\"

PUBLIC_HEADERS += \
    qnfcglobal.h \
    qnearfieldmanager.h \
    qnearfieldtarget.h \
    qndefrecord.h \
    qndefnfctextrecord.h \
    qndefmessage.h \
    qndeffilter.h \
    qndefnfcurirecord.h \
    qnearfieldtagtype1.h \
    qnearfieldtagtype2.h \
    qllcpsocket.h \
    qnearfieldtagtype3.h \
    qnearfieldtagtype4.h \
    qllcpserver.h \
    qdeclarativendefrecord.h

PRIVATE_HEADERS += \
    qndefrecord_p.h \
    qnearfieldtarget_p.h \
    qnearfieldmanager_p.h \
    qtlv_p.h \
    checksum_p.h

SOURCES += \
    qnearfieldmanager.cpp \
    qnearfieldtarget.cpp \
    qndefrecord.cpp \
    qndefnfctextrecord.cpp \
    qndefmessage.cpp \
    qndeffilter.cpp \
    qndefnfcurirecord.cpp \
    qnearfieldtagtype1.cpp \
    qnearfieldtagtype2.cpp \
    qnearfieldtagtype3.cpp \
    qllcpsocket.cpp \
    qnearfieldtagtype4.cpp \
    qtlv.cpp \
    qllcpserver.cpp \
    qdeclarativendefrecord.cpp

maemo6|meego {
    NFC_BACKEND_AVAILABLE = yes

    QT *= dbus

    DBUS_INTERFACES += \
        maemo6/com.nokia.nfc.Manager.xml

    DBUS_ADAPTORS += \
        maemo6/com.nokia.nfc.AccessRequestor.xml \
        maemo6/com.nokia.nfc.NDEFHandler.xml

    # work around bug in Qt
    dbus_interface_source.depends = ${QMAKE_FILE_OUT_BASE}.h
    dbus_adaptor_source.depends = ${QMAKE_FILE_OUT_BASE}.h

    # Link against libdbus until Qt has support for passing file descriptors over DBus.
    CONFIG += link_pkgconfig
    DEFINES += DBUS_API_SUBJECT_TO_CHANGE
    PKGCONFIG += dbus-1

    PRIVATE_HEADERS += \
        qnearfieldmanager_maemo6_p.h \
        qnearfieldtarget_maemo6_p.h \
        qllcpsocket_maemo6_p.h \
        qllcpserver_maemo6_p.h \
        maemo6/adapter_interface_p.h \
        maemo6/target_interface_p.h \
        maemo6/tag_interface_p.h \
        maemo6/device_interface_p.h \
        maemo6/socketrequestor_p.h

    SOURCES += \
        qnearfieldmanager_maemo6.cpp \
        qnearfieldtarget_maemo6.cpp \
        qllcpsocket_maemo6_p.cpp \
        qllcpserver_maemo6_p.cpp \
        maemo6/adapter_interface.cpp \
        maemo6/target_interface.cpp \
        maemo6/tag_interface.cpp \
        maemo6/device_interface.cpp \
        maemo6/socketrequestor.cpp

    OTHER_FILES += \
        $$DBUS_INTERFACES \
        $$DBUS_ADAPTORS \
        maemo6/com.nokia.nfc.Adapter.xml \
        maemo6/com.nokia.nfc.Target.xml \
        maemo6/com.nokia.nfc.Tag.xml \
        maemo6/com.nokia.nfc.Device.xml \
        maemo6/com.nokia.nfc.LLCPRequestor.xml

    # Add OUT_PWD to INCLUDEPATH so that creator picks up headers for generated files
    # This is not needed for the build otherwise.
    INCLUDEPATH += $$OUT_PWD
}

qnx {
    NFC_BACKEND_AVAILABLE = yes
    DEFINES += QQNXNFC_DEBUG

    PRIVATE_HEADERS += \
        qllcpsocket_p.h \
        qllcpserver_p.h \
        qnearfieldmanagerimpl_p.h \
        qnx/qnxnfcmanager.h

    SOURCES += \
        qllcpsocket_p.cpp \
        qllcpserver_p.cpp \
        qnearfieldmanagerimpl_p.cpp\
        qnx/qnxnfcmanager.cpp
}

simulator {
    NFC_BACKEND_AVAILABLE = yes

    QT *= gui

    PRIVATE_HEADERS += \
        qnearfieldmanagervirtualbase_p.h \
        qnearfieldmanager_simulator_p.h \
        qllcpsocket_simulator_p.h \
        qllcpserver_simulator_p.h

    SOURCES += \
        qnearfieldmanagervirtualbase.cpp \
        qnearfieldmanager_simulator.cpp \
        qllcpsocket_simulator_p.cpp \
        qllcpserver_simulator_p.cpp
}

isEmpty(NFC_BACKEND_AVAILABLE) {
    message("Unsupported NFC platform, will not build a working QtNfc library.")

    PRIVATE_HEADERS += \
        qllcpsocket_p.h \
        qllcpserver_p.h \
        qnearfieldmanagerimpl_p.h

    SOURCES += \
        qllcpsocket_p.cpp \
        qllcpserver_p.cpp \
        qnearfieldmanagerimpl_p.cpp
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

load(qt_module)

TARGET = QtAddOnNfc
QPRO_PWD = $PWD

CONFIG += module
MODULE_PRI = ../../modules/qt_nfc.pri

QT = core

DEFINES += QT_BUILD_NFC_LIB QT_MAKEDLL

load(qt_module_config)

PUBLIC_HEADERS += \
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

contains(nfc_symbian_enabled, yes):symbian {
    NFC_BACKEND_AVAILABLE = yes

    QT += serviceframework

    PRIVATE_HEADERS += \
        qnearfieldmanager_symbian_p.h \
        qnearfieldtagtype1_symbian_p.h \
        qnearfieldtagtype2_symbian_p.h \
        qllcpsocket_symbian_p.h \
        qllcpserver_symbian_p.h \
        qllcpstate_symbian_p.h \
        qnearfieldtagtype3_symbian_p.h \
        qnearfieldtagtype4_symbian_p.h \
        qnearfieldtagmifare_symbian_p.h \
        qnearfieldllcpdevice_symbian_p.h \
        symbian/nearfieldmanager_symbian.h \
        symbian/nearfieldtag_symbian.h \
        symbian/nearfieldndeftarget_symbian.h \
        symbian/nearfieldtargetfactory_symbian.h \
        symbian/nearfieldutility_symbian.h \
        symbian/llcpserver_symbian.h \
        symbian/llcpsockettype1_symbian.h \
        symbian/llcpsockettype2_symbian.h \
        symbian/nearfieldtagimpl_symbian.h \
        symbian/nearfieldtagasyncrequest_symbian.h \
        symbian/nearfieldtagndefoperationcallback_symbian.h \
        symbian/nearfieldtagoperationcallback_symbian.h \
        symbian/nearfieldtagndefrequest_symbian.h \
        symbian/nearfieldtagcommandrequest_symbian.h \
        symbian/nearfieldtagcommandsrequest_symbian.h \
        symbian/debug.h \
        symbian/nearfieldtagimplcommon_symbian.h

    SOURCES += \
        qnearfieldmanager_symbian.cpp \
        qnearfieldtagtype1_symbian.cpp \
        qnearfieldtagtype2_symbian.cpp \
        qllcpsocket_symbian_p.cpp \
        qllcpserver_symbian_p.cpp \
        qllcpstate_symbian_p.cpp \
        qnearfieldtagtype3_symbian.cpp \
        qnearfieldtagtype4_symbian.cpp \
        qnearfieldtagmifare_symbian.cpp \
        qnearfieldllcpdevice_symbian.cpp \
        symbian/nearfieldmanager_symbian.cpp \
        symbian/nearfieldtag_symbian.cpp \
        symbian/nearfieldndeftarget_symbian.cpp \
        symbian/nearfieldtargetfactory_symbian.cpp \
        symbian/nearfieldutility_symbian.cpp \
        symbian/llcpserver_symbian.cpp \
        symbian/llcpsockettype1_symbian.cpp \
        symbian/llcpsockettype2_symbian.cpp \
        symbian/nearfieldtagasyncrequest_symbian.cpp \
        symbian/nearfieldtagndefrequest_symbian.cpp \
        symbian/nearfieldtagcommandrequest_symbian.cpp \
        symbian/nearfieldtagcommandsrequest_symbian.cpp \
        symbian/nearfieldtagimplcommon_symbian.cpp

    INCLUDEPATH += $${EPOCROOT}epoc32/include/mw

    LIBS += -lnfc -lndef -lndefaccess -lnfcdiscoveryservice -lllcp -lnfctagextension
}

isEmpty(NFC_BACKEND_AVAILABLE) {
    # unsupported platform stub

    PRIVATE_HEADERS += \
        qllcpsocket_p.h \
        qllcpserver_p.h \
        qnearfieldmanagerimpl_p.h

    SOURCES += \
        qllcpsocket_p.cpp \
        qllcpserver_p.cpp \
        qnearfieldmanagerimpl_p.cpp
}

INCLUDEPATH += $$PWD

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

TARGET = QtNfc
QT = core

load(qt_module)

QMAKE_DOCS = $$PWD/doc/qtnfc.qdocconf
OTHER_FILES += doc/src/*.qdoc   # show .qdoc files in Qt Creator

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

maemo {
    NFC_BACKEND_AVAILABLE = yes

    DEFINES += QTNFC_MAEMO

    QT *= dbus

    PRIVATE_HEADERS += \
        qnearfieldmanager_maemo6_p.h \
        qnearfieldtarget_maemo6_p.h \
        qllcpsocket_maemo6_p.h \
        qllcpserver_maemo6_p.h

    SOURCES += \
        qnearfieldmanager_maemo6.cpp \
        qnearfieldtarget_maemo6.cpp \
        qllcpsocket_maemo6_p.cpp \
        qllcpserver_maemo6_p.cpp

    INCLUDEPATH += $OUT_PWD
    DEPENDPATH += $OUT_PWD
    LIBS_PRIVATE += -Lmaemo6 -lnfc_maemo6
}

qnx {
    NFC_BACKEND_AVAILABLE = yes
    DEFINES += QNX_NFC #QQNXNFC_DEBUG

    LIBS += -lnfc

    PRIVATE_HEADERS += \
        qllcpserver_qnx_p.h \
        qllcpsocket_qnx_p.h \
        qnearfieldmanager_qnx_p.h \
        qnx/qnxnfcmanager_p.h \
        qnearfieldtarget_qnx_p.h \
        qnx/qnxnfceventfilter_p.h

    SOURCES += \
        qllcpserver_qnx_p.cpp \
        qllcpsocket_qnx_p.cpp \
        qnearfieldmanager_qnx.cpp \
        qnx/qnxnfcmanager.cpp \
        qnx/qnxnfceventfilter.cpp
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

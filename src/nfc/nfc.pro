TARGET = QtNfc
QT = core network
DEFINES += QT_NO_FOREACH

QMAKE_DOCS = $$PWD/doc/qtnfc.qdocconf
OTHER_FILES += doc/src/*.qdoc   # show .qdoc files in Qt Creator

HEADERS += \
    qtnfcglobal.h \
    qnearfieldmanager.h \
    qnearfieldtarget.h \
    qndefrecord.h \
    qndefnfctextrecord.h \
    qndefmessage.h \
    qndeffilter.h \
    qndefnfcurirecord.h \
    qndefnfcsmartposterrecord.h \
    qtnfcglobal_p.h \
    qndefrecord_p.h \
    qnearfieldtarget_p.h \
    qnearfieldmanager_p.h \
    qnearfieldtagtype1_p.h \
    qnearfieldtagtype2_p.h \
    qtlv_p.h \
    qndefnfcsmartposterrecord_p.h

SOURCES += \
    qnearfieldmanager.cpp \
    qnearfieldtarget.cpp \
    qnearfieldtarget_p.cpp \
    qndefrecord.cpp \
    qndefnfctextrecord.cpp \
    qndefmessage.cpp \
    qndeffilter.cpp \
    qndefnfcurirecord.cpp \
    qnearfieldtagtype1.cpp \
    qnearfieldtagtype2.cpp \
    qtlv.cpp \
    qndefnfcsmartposterrecord.cpp

android:!android-embedded {
    NFC_BACKEND_AVAILABLE = yes
    DEFINES += QT_ANDROID_NFC
    ANDROID_PERMISSIONS = \
        android.permission.NFC
    ANDROID_BUNDLED_JAR_DEPENDENCIES = \
        jar/Qt$${QT_MAJOR_VERSION}AndroidNfc.jar:org.qtproject.qt.android.nfc.QtNfc
    DEFINES += ANDROID_NFC
    QT_PRIVATE += core-private gui androidextras

    HEADERS += \
        android/androidjninfc_p.h \
        qnearfieldmanager_android_p.h \
        qnearfieldtarget_android_p.h \
        android/androidmainnewintentlistener_p.h


    SOURCES += \
        android/androidjninfc.cpp \
        qnearfieldmanager_android.cpp \
        qnearfieldtarget_android.cpp \
        android/androidmainnewintentlistener.cpp
}

isEmpty(NFC_BACKEND_AVAILABLE) {
    message("Unsupported NFC platform, will not build a working QtNfc library.")

    HEADERS += \
        qnearfieldmanager_generic_p.h

    SOURCES += \
        qnearfieldmanager_generic.cpp
}

load(qt_module)

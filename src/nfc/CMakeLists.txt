# Generated from nfc.pro.

#####################################################################
## Nfc Module:
#####################################################################

qt_add_module(Nfc
    SOURCES
        qllcpserver.cpp qllcpserver_p.h
        qllcpsocket.cpp qllcpsocket_p.h
        qndeffilter.cpp qndeffilter.h
        qndefmessage.cpp qndefmessage.h
        qndefnfcsmartposterrecord.cpp qndefnfcsmartposterrecord.h qndefnfcsmartposterrecord_p.h
        qndefnfctextrecord.cpp qndefnfctextrecord.h
        qndefnfcurirecord.cpp qndefnfcurirecord.h
        qndefrecord.cpp qndefrecord.h qndefrecord_p.h
        qnearfieldmanager.cpp qnearfieldmanager.h qnearfieldmanager_p.h
        qnearfieldtagtype1.cpp qnearfieldtagtype1_p.h
        qnearfieldtagtype2.cpp qnearfieldtagtype2_p.h
        qnearfieldtarget.cpp qnearfieldtarget.h qnearfieldtarget_p.cpp qnearfieldtarget_p.h
        qtlv.cpp qtlv_p.h
        qtnfcglobal.h qtnfcglobal_p.h
    DEFINES
        QT_NO_FOREACH
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Network
)

#### Keys ignored in scope 1:.:.:nfc.pro:<TRUE>:
# OTHER_FILES = "doc/src/*.qdoc"

## Scopes:
#####################################################################

if(ANDROID AND (ANDROID AND NOT ANDROID_EMBEDDED))
    set_property(TARGET Nfc APPEND PROPERTY QT_ANDROID_BUNDLED_JAR_DEPENDENCIES
        jar/QtNfc.jar:org.qtproject.qt5.android.nfc.QtNfc
    )
    set_property(TARGET Nfc APPEND PROPERTY QT_ANDROID_PERMISSIONS
        android.permission.NFC
    )
    set(NFC_BACKEND_AVAILABLE ON) # special case
endif()

qt_extend_target(Nfc CONDITION ANDROID AND NOT ANDROID_EMBEDDED
    SOURCES
        android/androidjninfc.cpp android/androidjninfc_p.h
        android/androidmainnewintentlistener.cpp android/androidmainnewintentlistener_p.h
        qllcpserver_android_p.cpp qllcpserver_android_p.h
        qllcpsocket_android_p.cpp qllcpsocket_android_p.h
        qnearfieldmanager_android.cpp qnearfieldmanager_android_p.h
        qnearfieldtarget_android.cpp qnearfieldtarget_android_p.h
    DEFINES
        ANDROID_NFC
        QT_ANDROID_NFC
    LIBRARIES
        Qt::AndroidExtras
        Qt::CorePrivate
        Qt::Gui
)

#### Keys ignored in scope 2:.:.:nfc.pro:ANDROID AND NOT ANDROID_EMBEDDED:
# NFC_BACKEND_AVAILABLE = "yes"

# special case begin
# qt_extend_target(Nfc CONDITION NFC_BACKEND_AVAILABLE_ISEMPTY
qt_extend_target(Nfc CONDITION NOT NFC_BACKEND_AVAILABLE
# special case end
    SOURCES
        qllcpserver_p.cpp
        qllcpserver_p_p.h
        qllcpsocket_p.cpp
        qllcpsocket_p_p.h
        qnearfieldmanager_generic.cpp qnearfieldmanager_generic_p.h
)
qt_add_docs(Nfc
    doc/qtnfc.qdocconf
)


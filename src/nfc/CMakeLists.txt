# Generated from nfc.pro.

#####################################################################
## Nfc Module:
#####################################################################

add_qt_module(Nfc
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
        qnearfieldsharemanager.cpp qnearfieldsharemanager.h qnearfieldsharemanager_p.h
        qnearfieldsharetarget.cpp qnearfieldsharetarget.h qnearfieldsharetarget_p.h
        qnearfieldtagtype1.cpp qnearfieldtagtype1_p.h
        qnearfieldtagtype2.cpp qnearfieldtagtype2_p.h
        qnearfieldtagtype3.cpp qnearfieldtagtype3_p.h
        qnearfieldtagtype4.cpp qnearfieldtagtype4_p.h
        qnearfieldtarget.cpp qnearfieldtarget.h qnearfieldtarget_p.h
        qnfc.cpp
        qqmlndefrecord.cpp qqmlndefrecord.h
        qtlv.cpp qtlv_p.h
        qtnfcglobal.h qtnfcglobal_p.h
    DEFINES
        QT_NO_FOREACH
    PUBLIC_LIBRARIES
        Qt::Core
)

#### Keys ignored in scope 1:.:.:nfc.pro:<TRUE>:
# OTHER_FILES = "doc/src/*.qdoc"
# _LOADED = "qt_module"

## Scopes:
#####################################################################

extend_target(Nfc CONDITION LINUX AND TARGET Qt::DBus
    SOURCES
        neard/adapter.cpp neard/adapter_p.h
        neard/agent.cpp neard/agent_p.h
        neard/dbusobjectmanager.cpp neard/dbusobjectmanager_p.h
        neard/dbusproperties.cpp neard/dbusproperties_p.h
        neard/manager.cpp neard/manager_p.h
        neard/neard_helper.cpp neard/neard_helper_p.h
        neard/tag.cpp neard/tag_p.h
        qllcpserver_p.cpp
        qllcpserver_p_p.h
        qllcpsocket_p.cpp
        qllcpsocket_p_p.h
        qnearfieldmanager_neard.cpp qnearfieldmanager_neard_p.h
        qnearfieldsharemanagerimpl_p.cpp qnearfieldsharemanagerimpl_p.h
        qnearfieldsharetargetimpl_p.cpp qnearfieldsharetargetimpl_p.h
        qnearfieldtarget_neard_p.cpp qnearfieldtarget_neard_p.h
    DEFINES
        NEARD_NFC
    LIBRARIES
        Qt::DBus
)

# special case begin
if(LINUX AND TARGET Qt::DBus)
    set(NFC_BACKEND_AVAILABLE ON)
endif()
# special case end

#### Keys ignored in scope 2:.:.:nfc.pro:LINUX AND NOT ANDROID AND TARGET Qt::DBus:
# NFC_BACKEND_AVAILABLE = "yes"

if(ANDROID AND (ANDROID AND NOT ANDROID_EMBEDDED))
    set_property(TARGET Nfc APPEND PROPERTY QT_ANDROID_BUNDLED_JAR_DEPENDENCIES
        jar/QtNfc.jar:org.qtproject.qt5.android.nfc.QtNfc
    )
    set_property(TARGET Nfc APPEND PROPERTY QT_ANDROID_PERMISSIONS
        android.permission.NFC
    )
    set(NFC_BACKEND_AVAILABLE ON) # special case
endif()

extend_target(Nfc CONDITION ANDROID AND NOT ANDROID_EMBEDDED
    SOURCES
        android/androidjninfc.cpp android/androidjninfc_p.h
        android/androidmainnewintentlistener.cpp android/androidmainnewintentlistener_p.h
        qllcpserver_android_p.cpp qllcpserver_android_p.h
        qllcpsocket_android_p.cpp qllcpsocket_android_p.h
        qnearfieldmanager_android.cpp qnearfieldmanager_android_p.h
        qnearfieldsharemanagerimpl_p.cpp qnearfieldsharemanagerimpl_p.h
        qnearfieldsharetargetimpl_p.cpp qnearfieldsharetargetimpl_p.h
        qnearfieldtarget_android.cpp qnearfieldtarget_android_p.cpp qnearfieldtarget_android_p.h
    DEFINES
        ANDROID_NFC
        QT_ANDROID_NFC
    LIBRARIES
        Qt::AndroidExtras
        Qt::CorePrivate
        Qt::Gui
)

#### Keys ignored in scope 4:.:.:nfc.pro:ANDROID AND NOT ANDROID_EMBEDDED:
# NFC_BACKEND_AVAILABLE = "yes"

# special case begin
# extend_target(Nfc CONDITION NFC_BACKEND_AVAILABLE_ISEMPTY
extend_target(Nfc CONDITION NOT NFC_BACKEND_AVAILABLE
# special case end
    SOURCES
        qllcpserver_p.cpp
        qllcpserver_p_p.h
        qllcpsocket_p.cpp
        qllcpsocket_p_p.h
        qnearfieldmanagerimpl_p.cpp qnearfieldmanagerimpl_p.h
        qnearfieldsharemanagerimpl_p.cpp qnearfieldsharemanagerimpl_p.h
        qnearfieldsharetargetimpl_p.cpp qnearfieldsharetargetimpl_p.h
        qnearfieldtarget_p.cpp
)
add_qt_docs(Nfc
    doc/qtnfc.qdocconf
)

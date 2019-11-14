# Generated from nfc.pro.

#####################################################################
## nfc Plugin:
#####################################################################

qt_add_qml_module(declarative_nfc # special case
    URI "QtNfc"
    CLASSNAME QNfcQmlPlugin
    SKIP_TYPE_REGISTRATION
    SOURCES
        plugin.cpp
        qdeclarativendeffilter.cpp qdeclarativendeffilter_p.h
        qdeclarativendefmimerecord.cpp qdeclarativendefmimerecord_p.h
        qdeclarativendeftextrecord.cpp qdeclarativendeftextrecord_p.h
        qdeclarativendefurirecord.cpp qdeclarativendefurirecord_p.h
        qdeclarativenearfield.cpp qdeclarativenearfield_p.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Nfc
        Qt::Qml
)

#### Keys ignored in scope 1:.:.:nfc.pro:<TRUE>:
# TARGETPATH = "QtNfc"

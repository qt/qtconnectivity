TARGET = declarative_nfc
TARGETPATH = QtAddOn/nfc

include(qnfcimport.pri)
target.path = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH
DESTDIR = $$QT.nfc.imports/$$TARGETPATH
INSTALLS += target

qmldir.files += $$PWD/qmldir
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$TARGETPATH
INSTALLS += qmldir

QT += declarative nfc

# Input
HEADERS += \
    qdeclarativenearfieldsocket_p.h \
    qdeclarativenearfield_p.h \
    qdeclarativendeffilter_p.h \
    qdeclarativendeftextrecord_p.h \
    qdeclarativendefurirecord_p.h \
    qdeclarativendefmimerecord_p.h

SOURCES += plugin.cpp \
    qdeclarativenearfieldsocket.cpp \
    qdeclarativenearfield.cpp \
    qdeclarativendeffilter.cpp \
    qdeclarativendeftextrecord.cpp \
    qdeclarativendefurirecord.cpp \
    qdeclarativendefmimerecord.cpp

INSTALLS += qmldir

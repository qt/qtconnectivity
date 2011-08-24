QT       += testlib

TARGET = tst_qnearfieldtagtype2
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += testcase

TEMPLATE = app

INCLUDEPATH += ../../../src/nfc
DEPENDPATH += ../../../src/nfc
INCLUDEPATH += ../common
DEPENDPATH += ../common

INCLUDEPATH += ../../../src/global
DEPENDPATH += ../../../src/global

QMAKE_LIBDIR += $$QT_MOBILITY_BUILD_TREE/lib

HEADERS += ../common/qnfctagtestcommon.h \
           ../common/qdummyslot.h \
           ../common/qautomsgbox.h \
           ../common/qnfctagutil.h

SOURCES += tst_qnearfieldtagtype2.cpp \
           ../common/qdummyslot.cpp \
           ../common/qautomsgbox.cpp \
           ../common/qnfctagutil.cpp

symbian:TARGET.CAPABILITY = LocalServices


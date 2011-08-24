QT       += testlib

TARGET = tst_qnearfieldmanager
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += testcase

TEMPLATE = app

INCLUDEPATH += ../../../src/nfc
DEPENDPATH += ../../../src/nfc

INCLUDEPATH += ../common
DEPENDPATH += ../common

HEADERS += ../common/qautomsgbox.h
SOURCES += tst_qnearfieldmanager.cpp \
        ../common/qautomsgbox.cpp

INCLUDEPATH += ../../../src/global
DEPENDPATH += ../../../src/global

QMAKE_LIBDIR += $$QT_MOBILITY_BUILD_TREE/lib

symbian:TARGET.CAPABILITY = ALL -TCB

include(../mobility_examples.pri)

CONFIG += mobility strict_flags
MOBILITY += connectivity

INCLUDEPATH += $$PWD/../../src/connectivity/nfc
DEPENDPATH += $$PWD/../../src/connectivity/nfc

TARGET = annotatedurl

SOURCES += main.cpp \
    mainwindow.cpp \
    annotatedurl.cpp

HEADERS  += mainwindow.h \
    annotatedurl.h

FORMS    += mainwindow.ui

symbian:TARGET.CAPABILITY += LocalServices


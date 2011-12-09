QT += nfc widgets

CONFIG += strict_flags

INCLUDEPATH += $$PWD/../../src/connectivity/nfc
DEPENDPATH += $$PWD/../../src/connectivity/nfc

TARGET = annotatedurl

SOURCES += main.cpp \
    mainwindow.cpp \
    annotatedurl.cpp

HEADERS  += mainwindow.h \
    annotatedurl.h

FORMS    += mainwindow.ui


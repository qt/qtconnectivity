include(../mobility_examples.pri)

CONFIG += mobility strict_flags
MOBILITY += connectivity

INCLUDEPATH += $$PWD/../../src/connectivity/nfc
DEPENDPATH += $$PWD/../../src/connectivity/nfc

QT += core gui

TARGET = ndefeditor
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    textrecordeditor.cpp \
    urirecordeditor.cpp \
    mimeimagerecordeditor.cpp

HEADERS += \
    mainwindow.h \
    textrecordeditor.h \
    urirecordeditor.h \
    mimeimagerecordeditor.h

FORMS += \
    mainwindow.ui \
    textrecordeditor.ui \
    urirecordeditor.ui \
    mimeimagerecordeditor.ui

symbian:TARGET.CAPABILITY += LocalServices

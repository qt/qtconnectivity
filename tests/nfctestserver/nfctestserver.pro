
INCLUDEPATH += $$PWD/../../src/nfc

TARGET = nfctestserver

CONFIG += console strict_flags
CONFIG -= app_bundle

QT = core nfc

TEMPLATE = app

SOURCES += main.cpp \
    socketcontroller.cpp \
    servercontroller.cpp

HEADERS += \
    socketcontroller.h \
    servercontroller.h \
    servicenames.h
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

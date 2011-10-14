
INCLUDEPATH += $$PWD/../../src/nfc
DEPENDPATH += $$PWD/../../src/nfc


TARGET = nfctestserver

CONFIG += qt debug warn_on console depend_includepath
CONFIG += console strict_flags
CONFIG -= app_bundle

QMAKE_LIBS += -Wl,-rpath,$${QT.nfc.libs}

QT = core nfc

TEMPLATE = app

SOURCES += main.cpp \
    socketcontroller.cpp \
    servercontroller.cpp

HEADERS += \
    socketcontroller.h \
    servercontroller.h \
    servicenames.h

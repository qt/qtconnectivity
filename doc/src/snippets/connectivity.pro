TARGET = connectivity
TEMPLATE=app

QT += bluetooth nfc

INCLUDEPATH += ../../src/connectivity/nfc
DEPENDPATH += ../../src/connectivity/nfc

INCLUDEPATH += ../../src/connectivity/bluetooth
DEPENDPATH += ../../src/connectivity/bluetooth

SOURCES = main.cpp \
    nfc.cpp \
    devicediscovery.cpp \
    servicediscovery.cpp \
    foorecord.cpp

HEADERS = \
    foorecord.h

CONFIG += strict_flags

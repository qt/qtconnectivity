TARGET = connectivity
TEMPLATE=app
include(../../../../features/basic_examples_setup.pri)

INCLUDEPATH += ../../src/connectivity/nfc
DEPENDPATH += ../../src/connectivity/nfc

INCLUDEPATH += ../../src/connectivity/bluetooth
DEPENDPATH += ../../src/connectivity/bluetooth

CONFIG += mobility
MOBILITY = connectivity

SOURCES = main.cpp \
    nfc.cpp \
    devicediscovery.cpp \
    servicediscovery.cpp \
    foorecord.cpp

HEADERS = \
    foorecord.h

CONFIG += strict_flags

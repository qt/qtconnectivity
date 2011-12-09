TARGET = btscanner

INCLUDEPATH += ../../src/connectivity/bluetooth
DEPENDPATH += ../../src/connectivity/bluetooth

include(../mobility_examples.pri)

CONFIG += mobility
MOBILITY = connectivity
TEMPLATE = app

SOURCES = \
    main.cpp \
    device.cpp \
    service.cpp

HEADERS = \
    device.h \
    service.h

FORMS = \
    device.ui \
    service.ui

symbian {
    TARGET.CAPABILITY = ReadDeviceData LocalServices WriteDeviceData
}


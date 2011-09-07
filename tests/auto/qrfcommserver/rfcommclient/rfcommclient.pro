TARGET = rfcommclient
SOURCES += main.cpp rfcommclient.cpp
HEADERS += rfcommclient.h

QT = core

INCLUDEPATH += ../../../../src/connectivity/bluetooth
DEPENDPATH += ../../../../src/connectivity/bluetooth

include(../../../../common.pri)

CONFIG += mobility
MOBILITY = connectivity
symbian: TARGET.CAPABILITY =  LocalServices NetworkControl WriteDeviceData

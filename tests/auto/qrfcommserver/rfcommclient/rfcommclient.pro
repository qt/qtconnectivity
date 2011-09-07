TARGET = rfcommclient
SOURCES += main.cpp rfcommclient.cpp
HEADERS += rfcommclient.h

QT = core

symbian: TARGET.CAPABILITY =  LocalServices NetworkControl WriteDeviceData

TARGET = rfcommclient
SOURCES += main.cpp rfcommclient.cpp
HEADERS += rfcommclient.h

QT = core

symbian: TARGET.CAPABILITY =  LocalServices NetworkControl WriteDeviceData

target.path = $$[QT_INSTALL_LIBS]/tests/$$TARGET/
INSTALLS += target

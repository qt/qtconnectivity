TARGET = rfcommclient
SOURCES += main.cpp rfcommclient.cpp
HEADERS += rfcommclient.h

QT = core

symbian: TARGET.CAPABILITY =  LocalServices NetworkControl WriteDeviceData

!include(../../auto.pri):error(missing auto.pri)
target.path = $$TEST_INSTALL_BASE/tst_qrfcommserver

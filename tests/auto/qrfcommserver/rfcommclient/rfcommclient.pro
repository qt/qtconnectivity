TARGET = rfcommclient
SOURCES += main.cpp rfcommclient.cpp
HEADERS += rfcommclient.h

QT = core

symbian: TARGET.CAPABILITY =  LocalServices NetworkControl WriteDeviceData

# This is a helper app for tst_qrfcommserver
target.path = $$[QT_INSTALL_TESTS]/tst_qrfcommserver
INSTALLS += target

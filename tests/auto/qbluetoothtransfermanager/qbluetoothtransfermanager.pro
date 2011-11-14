SOURCES += tst_qbluetoothtransfermanager.cpp
TARGET=tst_qbluetoothtransfermanager
CONFIG += testcase

QT = core bluetooth testlib

INCLUDEPATH += ../../../tests/btclient
DEPENDPATH += ../../../tests/btclient

symbian: TARGET.CAPABILITY = All-TCB #ReadDeviceData LocalServices WriteDeviceData

CONFIG += insignificant_test    # QTBUG-22017

target.path = $$[QT_INSTALL_LIBS]/tests/$$TARGET/
INSTALLS += target

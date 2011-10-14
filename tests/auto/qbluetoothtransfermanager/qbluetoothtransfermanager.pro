SOURCES += tst_qbluetoothtransfermanager.cpp
TARGET=tst_qbluetoothtransfermanager
CONFIG += testcase

QT = core bluetooth testlib

INCLUDEPATH += ../../../tests/btclient
DEPENDPATH += ../../../tests/btclient

symbian: TARGET.CAPABILITY = All-TCB #ReadDeviceData LocalServices WriteDeviceData

CONFIG += insignificant_test    # QTBUG-22017

SOURCES += tst_qbluetoothserviceinfo.cpp
TARGET = tst_qbluetoothserviceinfo
CONFIG += testcase

QT = core bluetooth testlib

INCLUDEPATH += ../../../src/bluetooth
DEPENDPATH += ../../../src/bluetooth

symbian: TARGET.CAPABILITY = ReadDeviceData LocalServices WriteDeviceData

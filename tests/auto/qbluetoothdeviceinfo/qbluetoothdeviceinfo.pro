SOURCES += tst_qbluetoothdeviceinfo.cpp
TARGET=tst_qbluetoothdeviceinfo
CONFIG += testcase

QT = core bluetooth testlib

INCLUDEPATH += ../../../src/bluetooth
DEPENDPATH += ../../../src/bluetooth

symbian: TARGET.CAPABILITY = ReadDeviceData LocalServices WriteDeviceData

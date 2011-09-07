SOURCES += tst_qbluetoothserviceinfo.cpp
TARGET = tst_qbluetoothserviceinfo
CONFIG += testcase

QT = core bluetooth testlib

symbian: TARGET.CAPABILITY = ReadDeviceData LocalServices WriteDeviceData

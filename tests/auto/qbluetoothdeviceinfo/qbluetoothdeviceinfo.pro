SOURCES += tst_qbluetoothdeviceinfo.cpp
TARGET=tst_qbluetoothdeviceinfo
CONFIG += testcase

QT = core bluetooth testlib

symbian: TARGET.CAPABILITY = ReadDeviceData LocalServices WriteDeviceData

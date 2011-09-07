SOURCES += tst_qbluetoothservicediscoveryagent.cpp
TARGET = tst_qbluetoothservicediscoveryagent
CONFIG += testcase

QT = core bluetooth testlib

symbian: TARGET.CAPABILITY = ReadDeviceData LocalServices WriteDeviceData

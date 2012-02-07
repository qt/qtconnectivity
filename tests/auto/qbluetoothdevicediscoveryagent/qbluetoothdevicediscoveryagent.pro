SOURCES += tst_qbluetoothdevicediscoveryagent.cpp
TARGET=tst_qbluetoothdevicediscoveryagent
CONFIG += testcase

QT = core concurrent bluetooth testlib

symbian: TARGET.CAPABILITY = ReadDeviceData LocalServices WriteDeviceData

CONFIG += insignificant_test    # QTBUG-22017

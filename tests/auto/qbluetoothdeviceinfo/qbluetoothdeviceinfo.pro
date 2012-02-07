SOURCES += tst_qbluetoothdeviceinfo.cpp
TARGET=tst_qbluetoothdeviceinfo
CONFIG += testcase

QT = core concurrent bluetooth testlib

symbian: TARGET.CAPABILITY = ReadDeviceData LocalServices WriteDeviceData

CONFIG += insignificant_test    # QTBUG-22017

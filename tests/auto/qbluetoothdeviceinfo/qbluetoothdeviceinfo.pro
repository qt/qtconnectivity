SOURCES += tst_qbluetoothdeviceinfo.cpp
TARGET=tst_qbluetoothdeviceinfo
CONFIG += testcase

QT = core concurrent bluetooth testlib

CONFIG += insignificant_test    # QTBUG-22017

SOURCES += tst_qbluetoothserviceinfo.cpp
TARGET = tst_qbluetoothserviceinfo
CONFIG += testcase

QT = core concurrent bluetooth testlib

CONFIG += insignificant_test    # QTBUG-22017

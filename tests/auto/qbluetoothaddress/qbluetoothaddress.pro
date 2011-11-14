SOURCES += tst_qbluetoothaddress.cpp
TARGET = tst_qbluetoothaddress
CONFIG += testcase

QT = core bluetooth testlib

!include(../auto.pri):error(missing auto.pri)

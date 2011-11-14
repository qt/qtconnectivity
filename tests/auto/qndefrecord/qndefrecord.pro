SOURCES += tst_qndefrecord.cpp
TARGET = tst_qndefrecord
CONFIG += testcase

QT = core nfc testlib

!include(../auto.pri):error(missing auto.pri)

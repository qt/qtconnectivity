SOURCES += tst_qndefmessage.cpp
TARGET = tst_qndefmessage
CONFIG += testcase

QT = core nfc testlib

CONFIG += insignificant_test    # QTBUG-22016

!include(../auto.pri):error(missing auto.pri)

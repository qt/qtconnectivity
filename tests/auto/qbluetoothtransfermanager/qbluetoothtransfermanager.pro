SOURCES += tst_qbluetoothtransfermanager.cpp
TARGET=tst_qbluetoothtransfermanager
CONFIG += testcase
testcase.timeout = 250 # this test is slow

QT = core concurrent bluetooth testlib

INCLUDEPATH += ../../../tests/btclient

TESTDATA += *.txt
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

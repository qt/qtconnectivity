SOURCES += tst_qbluetoothtransfermanager.cpp
TARGET=tst_qbluetoothtransfermanager
CONFIG += testcase
testcase.timeout = 250 # this test is slow

QT = core concurrent bluetooth testlib

INCLUDEPATH += ../../../tests/btclient
DEPENDPATH += ../../../tests/btclient

CONFIG += insignificant_test    # QTBUG-22017

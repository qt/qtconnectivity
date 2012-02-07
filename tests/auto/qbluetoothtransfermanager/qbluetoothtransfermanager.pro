SOURCES += tst_qbluetoothtransfermanager.cpp
TARGET=tst_qbluetoothtransfermanager
CONFIG += testcase

QT = core concurrent bluetooth testlib

INCLUDEPATH += ../../../tests/btclient
DEPENDPATH += ../../../tests/btclient

CONFIG += insignificant_test    # QTBUG-22017

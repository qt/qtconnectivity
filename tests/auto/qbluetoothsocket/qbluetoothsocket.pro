SOURCES += tst_qbluetoothsocket.cpp
TARGET = tst_qbluetoothsocket
CONFIG += testcase
testcase.timeout = 250 # this test is slow

QT = core concurrent network bluetooth testlib

INCLUDEPATH += ../../../tests/btclient
DEPENDPATH += ../../../tests/btclient

OTHER_FILES += \
    README.txt

CONFIG += insignificant_test    # QTBUG-22017

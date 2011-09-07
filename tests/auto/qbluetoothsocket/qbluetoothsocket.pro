SOURCES += tst_qbluetoothsocket.cpp
TARGET = tst_qbluetoothsocket
CONFIG += testcase

QT = core network bluetooth testlib

INCLUDEPATH += ../../../tests/btclient
DEPENDPATH += ../../../tests/btclient

symbian: TARGET.CAPABILITY = NetworkControl ReadDeviceData LocalServices WriteDeviceData

OTHER_FILES += \
    README.txt

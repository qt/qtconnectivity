SOURCES += tst_qbluetoothtransferrequest.cpp
TARGET=tst_qbluetoothtransferrequest
CONFIG += testcase

QT = core concurrent bluetooth testlib

symbian: TARGET.CAPABILITY = ReadDeviceData LocalServices WriteDeviceData

CONFIG += insignificant_test    # QTBUG-22017

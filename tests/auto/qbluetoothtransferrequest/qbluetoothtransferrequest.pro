SOURCES += tst_qbluetoothtransferrequest.cpp
TARGET=tst_qbluetoothtransferrequest
CONFIG += testcase

QT = core concurrent bluetooth testlib

CONFIG += insignificant_test    # QTBUG-22017

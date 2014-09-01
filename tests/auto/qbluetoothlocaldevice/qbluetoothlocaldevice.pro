SOURCES += tst_qbluetoothlocaldevice.cpp
TARGET=tst_qbluetoothlocaldevice
CONFIG += testcase

QT = core concurrent bluetooth testlib

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

osx:CONFIG += insignificant_test


SOURCES += tst_qbluetoothlocaldevice.cpp
TARGET=tst_qbluetoothlocaldevice
CONFIG += testcase

QT = core concurrent bluetooth testlib
osx:QT += widgets
osx:CONFIG += insignificant_test
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

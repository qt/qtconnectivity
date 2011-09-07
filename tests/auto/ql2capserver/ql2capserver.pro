SOURCES += tst_ql2capserver.cpp
TARGET = tst_ql2capserver
CONFIG += testcase

QT = core bluetooth testlib

INCLUDEPATH += ../../../src/bluetooth
DEPENDPATH += ../../../src/bluetooth

symbian: TARGET.CAPABILITY =  LocalServices NetworkControl

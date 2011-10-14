SOURCES += tst_qnearfieldtagtype2.cpp
TARGET = tst_qnearfieldtagtype2
CONFIG += testcase

QT = core nfc-private testlib

DEFINES += SRCDIR=\\\"$$PWD\\\"

maemo*:CONFIG += insignificant_test

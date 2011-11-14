SOURCES += tst_qnearfieldmanager.cpp
TARGET = tst_qnearfieldmanager
CONFIG += testcase

QT = core nfc-private testlib

DEFINES += SRCDIR=\\\"$$PWD/../nfcdata\\\"

maemo*:CONFIG += insignificant_test

nfcdata.files=../nfcdata
TESTDATA+=nfcdata

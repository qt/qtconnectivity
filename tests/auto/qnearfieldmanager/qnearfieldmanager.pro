SOURCES += tst_qnearfieldmanager.cpp
TARGET = tst_qnearfieldmanager
CONFIG += testcase

QT = core nfc-private testlib

DEFINES += SRCDIR=\\\"$$PWD/../nfcdata\\\"

maemo*:CONFIG += insignificant_test

!include(../auto.pri):error(missing auto.pri)

testdata.files=../nfcdata
testdata.path=$$target.path
INSTALLS+=testdata

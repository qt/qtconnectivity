SOURCES += tst_qnearfieldtagtype1.cpp
TARGET = tst_qnearfieldtagtype1
CONFIG += testcase

QT = core nfc-private testlib


DEFINES += SRCDIR=\\\"$$PWD/../nfcdata\\\"
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

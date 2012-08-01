SOURCES += tst_qnearfieldmanager.cpp
TARGET = tst_qnearfieldmanager
CONFIG += testcase

QT = core nfc-private testlib

DEFINES += SRCDIR=\\\"$$PWD/../nfcdata\\\"
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

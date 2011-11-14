SOURCES += tst_qnearfieldtagtype1.cpp
TARGET = tst_qnearfieldtagtype1
CONFIG += testcase

QT = core nfc-private testlib


DEFINES += SRCDIR=\\\"$$PWD/../nfcdata\\\"

maemo*:CONFIG += insignificant_test

target.path = $$[QT_INSTALL_LIBS]/tests/$$TARGET/
INSTALLS += target

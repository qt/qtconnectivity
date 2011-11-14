SOURCES += tst_qnearfieldtagtype2.cpp
TARGET = tst_qnearfieldtagtype2
CONFIG += testcase

QT = core nfc-private testlib

DEFINES += SRCDIR=\\\"$$PWD\\\"

maemo*:CONFIG += insignificant_test

target.path = $$[QT_INSTALL_LIBS]/tests/$$TARGET/
INSTALLS += target

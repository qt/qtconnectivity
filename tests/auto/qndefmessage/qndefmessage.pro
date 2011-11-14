SOURCES += tst_qndefmessage.cpp
TARGET = tst_qndefmessage
CONFIG += testcase

QT = core nfc testlib

CONFIG += insignificant_test    # QTBUG-22016

target.path = $$[QT_INSTALL_LIBS]/tests/$$TARGET/
INSTALLS += target

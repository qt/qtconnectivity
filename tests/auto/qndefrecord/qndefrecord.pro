SOURCES += tst_qndefrecord.cpp
TARGET = tst_qndefrecord
CONFIG += testcase

QT = core nfc testlib

target.path = $$[QT_INSTALL_LIBS]/tests/$$TARGET/
INSTALLS += target

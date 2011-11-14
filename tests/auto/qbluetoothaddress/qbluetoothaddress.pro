SOURCES += tst_qbluetoothaddress.cpp
TARGET = tst_qbluetoothaddress
CONFIG += testcase

QT = core bluetooth testlib

target.path = $$[QT_INSTALL_LIBS]/tests/$$TARGET/
INSTALLS += target

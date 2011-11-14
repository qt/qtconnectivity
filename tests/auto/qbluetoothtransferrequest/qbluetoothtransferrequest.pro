SOURCES += tst_qbluetoothtransferrequest.cpp
TARGET=tst_qbluetoothtransferrequest
CONFIG += testcase

QT = core bluetooth testlib

symbian: TARGET.CAPABILITY = ReadDeviceData LocalServices WriteDeviceData

CONFIG += insignificant_test    # QTBUG-22017

target.path = $$[QT_INSTALL_LIBS]/tests/$$TARGET/
INSTALLS += target

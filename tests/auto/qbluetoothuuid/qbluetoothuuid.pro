SOURCES += tst_qbluetoothuuid.cpp
TARGET = tst_qbluetoothuuid
CONFIG += testcase

QT = core bluetooth testlib

symbian: {
    LIBS *= -lbluetooth
}

target.path = $$[QT_INSTALL_LIBS]/tests/$$TARGET/
INSTALLS += target

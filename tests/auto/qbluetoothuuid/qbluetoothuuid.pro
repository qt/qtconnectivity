SOURCES += tst_qbluetoothuuid.cpp
TARGET = tst_qbluetoothuuid
CONFIG += testcase

QT = core bluetooth testlib

INCLUDEPATH += ../../../src/bluetooth
DEPENDPATH += ../../../src/bluetooth

symbian: {
    LIBS *= -lbluetooth
}

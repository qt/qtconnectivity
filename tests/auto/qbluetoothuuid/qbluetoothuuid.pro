SOURCES += tst_qbluetoothuuid.cpp
TARGET = tst_qbluetoothuuid
CONFIG += testcase

QT = core bluetooth testlib

symbian: {
    LIBS *= -lbluetooth
}

!include(../auto.pri):error(missing auto.pri)

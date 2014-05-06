SOURCES += tst_qlowenergycharacteristicinfo.cpp
TARGET = tst_qlowenergycharacteristicinfo
CONFIG += testcase

QT = core bluetooth testlib
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
blackberry {
    LIBS += -lbtapi
}

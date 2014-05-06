SOURCES += tst_qlowenergyserviceinfo.cpp
TARGET = tst_qlowenergyserviceinfo
CONFIG += testcase

QT = core bluetooth testlib
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
blackberry {
    LIBS += -lbtapi
}

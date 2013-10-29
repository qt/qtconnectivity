SOURCES += tst_qlowenergydescriptorinfo.cpp
TARGET = tst_qlowenergydescriptorinfo
CONFIG += testcase

QT = core bluetooth testlib
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
qnx {
    LIBS += -lbtapi
}

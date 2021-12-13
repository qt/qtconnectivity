QT = core bluetooth-private testlib

# Android requires GUI application when running test
android: QT += widgets

TARGET = tst_qlowenergycontroller
CONFIG += testcase

SOURCES += tst_qlowenergycontroller.cpp

macos: QMAKE_INFO_PLIST = ../shared/Info.macos.plist

osx|ios {
    QT += widgets
}

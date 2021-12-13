SOURCES += tst_qlowenergycharacteristic.cpp
TARGET = tst_qlowenergycharacteristic
CONFIG += testcase

macos: QMAKE_INFO_PLIST = ../shared/Info.macos.plist

QT = core bluetooth testlib



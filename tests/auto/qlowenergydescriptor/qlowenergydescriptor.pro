SOURCES += tst_qlowenergydescriptor.cpp
TARGET = tst_qlowenergydescriptor
CONFIG += testcase

macos: QMAKE_INFO_PLIST = ../shared/Info.macos.plist

QT = core bluetooth testlib


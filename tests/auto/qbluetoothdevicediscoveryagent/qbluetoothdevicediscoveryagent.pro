SOURCES += tst_qbluetoothdevicediscoveryagent.cpp
TARGET=tst_qbluetoothdevicediscoveryagent
CONFIG += testcase

macos: QMAKE_INFO_PLIST = ../shared/Info.macos.plist

QT = core concurrent bluetooth-private testlib
osx:QT += widgets

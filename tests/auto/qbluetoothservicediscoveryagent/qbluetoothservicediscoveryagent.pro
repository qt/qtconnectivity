SOURCES += tst_qbluetoothservicediscoveryagent.cpp
TARGET = tst_qbluetoothservicediscoveryagent
CONFIG += testcase

macos: QMAKE_INFO_PLIST = ../shared/Info.macos.plist

QT = core concurrent bluetooth testlib
osx:QT += widgets


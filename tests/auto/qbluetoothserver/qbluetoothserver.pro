SOURCES += tst_qbluetoothserver.cpp
TARGET = tst_qbluetoothserver
CONFIG += testcase

QT = core concurrent bluetooth testlib
osx:QT += widgets

OTHER_FILES += \
    README.txt

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

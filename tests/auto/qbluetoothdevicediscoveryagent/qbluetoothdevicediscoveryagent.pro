SOURCES += tst_qbluetoothdevicediscoveryagent.cpp
TARGET=tst_qbluetoothdevicediscoveryagent
CONFIG += testcase

QT = core concurrent bluetooth testlib
osx:QT += widgets

config_bluez:qtHaveModule(dbus) {
    DEFINES += QT_BLUEZ_BLUETOOTH
}

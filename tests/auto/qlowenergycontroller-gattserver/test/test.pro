QT = core bluetooth bluetooth-private testlib

TARGET = tst_qlowenergycontroller-gattserver
CONFIG += testcase c++11

config_linux_crypto_api:DEFINES += CONFIG_LINUX_CRYPTO_API
config_bluez_le:DEFINES += CONFIG_BLUEZ_LE

SOURCES += tst_qlowenergycontroller-gattserver.cpp

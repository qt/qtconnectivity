QT = core bluetooth bluetooth-private testlib

TARGET = tst_qlowenergycontroller-gattserver
CONFIG += testcase c++11

config_linux_crypto_api:DEFINES += CONFIG_LINUX_CRYPTO_API

SOURCES += tst_qlowenergycontroller-gattserver.cpp

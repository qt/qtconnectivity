QT       += testlib

TARGET = tst_qcontenthandler
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += testcase

TEMPLATE = app

INCLUDEPATH += ../common
DEPENDPATH += ../common

HEADERS += ../common/qautomsgbox.h
SOURCES += tst_qcontenthandler.cpp \
        ../common/qautomsgbox.cpp
symbian:TARGET.CAPABILITY = LocalServices ReadUserData WriteUserData NetworkServices UserEnvironment Location SwEvent ReadDeviceData WriteDeviceData

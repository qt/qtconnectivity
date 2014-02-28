SOURCES += tst_qrfcommserver.cpp
TARGET = tst_qrfcommserver
CONFIG += testcase

QT = core concurrent bluetooth testlib

OTHER_FILES += \
    README.txt

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

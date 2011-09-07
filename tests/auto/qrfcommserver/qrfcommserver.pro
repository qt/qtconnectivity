SOURCES += tst_qrfcommserver.cpp
TARGET = tst_qrfcommserver
CONFIG += testcase

QT = core bluetooth testlib

INCLUDEPATH += ../../../src/bluetooth
DEPENDPATH += ../../../src/bluetooth

symbian: TARGET.CAPABILITY =  LocalServices NetworkControl WriteDeviceData

OTHER_FILES += \
    README.txt



SOURCES += tst_qrfcommserver.cpp
TARGET = tst_qrfcommserver
CONFIG += testcase

QT = core bluetooth testlib

symbian: TARGET.CAPABILITY =  LocalServices NetworkControl WriteDeviceData

OTHER_FILES += \
    README.txt

CONFIG += insignificant_test    # QTBUG-22017

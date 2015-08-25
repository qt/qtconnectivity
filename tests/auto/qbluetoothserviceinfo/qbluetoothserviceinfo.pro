SOURCES += tst_qbluetoothserviceinfo.cpp
TARGET = tst_qbluetoothserviceinfo
CONFIG += testcase

QT = core concurrent bluetooth testlib
osx:QT += widgets


android:!android-no-sdk {
    DEFINES += QT_ANDROID_BLUETOOTH
}

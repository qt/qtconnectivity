QT += nfc widgets

CONFIG += strict_flags

TARGET = annotatedurl

SOURCES += main.cpp \
    mainwindow.cpp \
    annotatedurl.cpp

HEADERS  += mainwindow.h \
    annotatedurl.h

android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

    DISTFILES += \
        android/AndroidManifest.xml
}

target.path = $$[QT_INSTALL_EXAMPLES]/nfc/annotatedurl
INSTALLS += target

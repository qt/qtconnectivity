QT += nfc widgets
requires(qtConfig(filedialog))

TARGET = ndefeditor
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    textrecordeditor.cpp \
    urirecordeditor.cpp \
    mimeimagerecordeditor.cpp

HEADERS += \
    mainwindow.h \
    textrecordeditor.h \
    urirecordeditor.h \
    mimeimagerecordeditor.h

ios: QMAKE_INFO_PLIST = Info.plist

FORMS += \
    mainwindow.ui \
    textrecordeditor.ui \
    urirecordeditor.ui \
    mimeimagerecordeditor.ui

android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

    DISTFILES += \
        android/AndroidManifest.xml
}

target.path = $$[QT_INSTALL_EXAMPLES]/nfc/ndefeditor
INSTALLS += target

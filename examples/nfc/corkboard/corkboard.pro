QT += quick nfc

SOURCES += \
    main.cpp

TARGET = qml_corkboard
TEMPLATE = app

RESOURCES += \
    corkboard.qrc

OTHER_FILES += \
    corkboards.qml \
    Mode.qml

android {
OTHER_FILES += \
    android/AndroidManifest.xml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
}

TARGET = Qt$${QT_MAJOR_VERSION}AndroidNfc

CONFIG += java
DESTDIR = $$[QT_INSTALL_PREFIX/get]/jar
API_VERSION = android-18

PATHPREFIX = $$PWD/src/org/qtproject/qt/android/nfc

JAVACLASSPATH += $$PWD/src/
JAVASOURCES += \
    $$PWD/src/org/qtproject/qt/android/nfc/QtNfc.java \
    $$PWD/src/org/qtproject/qt/android/nfc/QtNfcBroadcastReceiver.java \

# install
target.path = $$[QT_INSTALL_PREFIX]/jar
INSTALLS += target

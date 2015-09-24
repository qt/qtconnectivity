CONFIG += java
DESTDIR = $$[QT_INSTALL_PREFIX/get]/jar
API_VERSION = android-10

PATHPREFIX = $$PWD/src/org/qtproject/qt5/android/nfc

JAVACLASSPATH += $$PWD/src/
JAVASOURCES += \
    $$PWD/src/org/qtproject/qt5/android/nfc/QtNfc.java

# install
target.path = $$[QT_INSTALL_PREFIX]/jar
INSTALLS += target

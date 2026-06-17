TEMPLATE = app
TARGET = heartrate-server

QT = core bluetooth
android|darwin: QT += gui

CONFIG += console

SOURCES += main.cpp

ios: QMAKE_INFO_PLIST = Info.qmake.ios.plist
macos: QMAKE_INFO_PLIST = Info.qmake.macos.plist

target.path = $$[QT_INSTALL_EXAMPLES]/bluetooth/heartrate-server
INSTALLS += target

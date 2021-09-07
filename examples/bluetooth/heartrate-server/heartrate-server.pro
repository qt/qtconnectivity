TEMPLATE = app
TARGET = heartrate-server

QT = core bluetooth
android: QT += gui
CONFIG += c++11

SOURCES += main.cpp

ios: QMAKE_INFO_PLIST = Info.plist

target.path = $$[QT_INSTALL_EXAMPLES]/bluetooth/heartrate-server
INSTALLS += target

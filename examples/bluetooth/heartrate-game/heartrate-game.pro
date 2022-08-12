TEMPLATE = app
TARGET = heartrate-game

QT += qml quick bluetooth
CONFIG += c++11

CONFIG += qmltypes
QML_IMPORT_NAME = Shared
QML_IMPORT_MAJOR_VERSION = 1

HEADERS += \
    connectionhandler.h \
    deviceinfo.h \
    devicefinder.h \
    devicehandler.h \
    bluetoothbaseclass.h \
    heartrate-global.h

SOURCES += main.cpp \
    connectionhandler.cpp \
    deviceinfo.cpp \
    devicefinder.cpp \
    devicehandler.cpp \
    bluetoothbaseclass.cpp

ios: QMAKE_INFO_PLIST = Info.plist
macos: QMAKE_INFO_PLIST = ../shared/Info.qmake.macos.plist

RESOURCES += qml.qrc \
    images.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

target.path = $$[QT_INSTALL_EXAMPLES]/bluetooth/heartrate-game
INSTALLS += target

include(../../mobility_examples.pri)

QT += declarative network

SOURCES += \
    qmlposter.cpp

TARGET = qml_poster
TEMPLATE = app

symbian {
    TARGET.CAPABILITY = LocalServices
    TARGET.EPOCHEAPSIZE = 0x20000 0x2000000
}

RESOURCES += \
    poster.qrc

OTHER_FILES += \
    poster.qml

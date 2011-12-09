include(../../mobility_examples.pri)

QT += declarative network
SOURCES += qmlscanner.cpp

TARGET = qml_scanner
TEMPLATE = app

win32 {
    #required by Qt SDK to resolve Mobility libraries
    CONFIG+=mobility
    MOBILITY+=connectivity
}

symbian {
    TARGET.CAPABILITY = LocalServices NetworkServices Location ReadUserData WriteUserData ReadDeviceData WriteDeviceData
    TARGET.EPOCHEAPSIZE = 0x20000 0x2000000
}

RESOURCES += \
    scanner.qrc

OTHER_FILES += \
    scanner.qml

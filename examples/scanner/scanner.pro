QT += concurrent widgets declarative network bluetooth quick
SOURCES += qmlscanner.cpp

TARGET = qml_scanner
TEMPLATE = app

RESOURCES += \
    scanner.qrc

OTHER_FILES += \
    scanner.qml

QT += quick bluetooth

SOURCES += \
    main.cpp

TARGET = qml_transfer
TEMPLATE = app

RESOURCES += \
    qmltransfer.qrc

OTHER_FILES += \
    bttransfer.qml \
    Button.qml

OTHER_FILES += \
    DeviceDiscovery.qml

OTHER_FILES += \
    PictureSelector.qml

HEADERS += \
    filetransfer.h

SOURCES += \
    filetransfer.cpp

OTHER_FILES += \
    FileSending.qml


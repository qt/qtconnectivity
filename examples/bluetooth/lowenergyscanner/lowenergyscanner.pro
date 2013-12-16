TARGET = lowenergyscanner
INCLUDEPATH += .

QT += quick bluetooth

# Input
SOURCES += main.cpp \
    device.cpp \
    deviceinfo.cpp \
    serviceinfo.cpp \
    characteristicinfo.cpp

OTHER_FILES += assets/Menu.qml \
    assets/Services.qml \
    bar-descriptor.xml \
    assets/Characteristics.qml \
    assets/Details.qml \
    bar-descriptor.xml \
    assets/Dialog.qml \
    assets/Header.qml \
    assets/Label.qml

HEADERS += \
    device.h \
    deviceinfo.h \
    serviceinfo.h \
    characteristicinfo.h

RESOURCES += \
    resources.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/bluetooth/lowenergyscanner
INSTALLS += target

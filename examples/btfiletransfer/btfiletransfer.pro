TARGET = btfiletransfer

INCLUDEPATH += ../../src/connectivity/bluetooth
DEPENDPATH += ../../src/connectivity/bluetooth

include(../mobility_examples.pri)

CONFIG += mobility
MOBILITY = connectivity

SOURCES = \
    main.cpp \
    remoteselector.cpp \
    progress.cpp \
    pindisplay.cpp

HEADERS = \
    remoteselector.h \
    progress.h \
    pindisplay.h

FORMS = \
    remoteselector.ui \
    progress.ui \
    pindisplay.ui


symbian {
    TARGET.CAPABILITY = LocalServices UserEnvironment ReadUserData WriteUserData NetworkServices ReadDeviceData WriteDeviceData
}

OTHER_FILES += \
    busy.gif \
    pairing.gif

RESOURCES += \
    btfiletransfer.qrc

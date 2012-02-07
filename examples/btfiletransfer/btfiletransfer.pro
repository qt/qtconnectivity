TARGET = btfiletransfer

QT += bluetooth widgets

INCLUDEPATH += ../../src/connectivity/bluetooth
DEPENDPATH += ../../src/connectivity/bluetooth

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

OTHER_FILES += \
    busy.gif \
    pairing.gif

RESOURCES += \
    btfiletransfer.qrc

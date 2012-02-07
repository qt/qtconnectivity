TARGET = bttennis

INCLUDEPATH += \
    ../../src/connectivity/bluetooth \
    ../../src/connectivity/nfc

DEPENDPATH += \
    ../../src/connectivity/bluetooth \
    ../../src/connectivity/nfc

QT += concurrent bluetooth nfc widgets

SOURCES = \
    main.cpp \
    board.cpp \
    tennis.cpp \
    controller.cpp \
    tennisserver.cpp \
    tennisclient.cpp \
    tennisview.cpp \
    handover.cpp

HEADERS = \
    board.h \
    tennis.h \
    controller.h \
    tennisserver.h \
    tennisclient.h \
    tennisview.h \
    handover.h

FORMS = \
    tennis.ui

RESOURCES += \
    tennis.qrc

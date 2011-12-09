TARGET = bttennis

INCLUDEPATH += \
    ../../src/connectivity/bluetooth \
    ../../src/connectivity/nfc

DEPENDPATH += \
    ../../src/connectivity/bluetooth \
    ../../src/connectivity/nfc

include(../demos.pri)

CONFIG += mobility
MOBILITY = connectivity

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

symbian {
    TARGET.CAPABILITY = LocalServices UserEnvironment ReadUserData WriteUserData NetworkServices
}

RESOURCES += \
    tennis.qrc

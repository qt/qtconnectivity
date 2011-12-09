TARGET = btchat

INCLUDEPATH += ../../src/connectivity/bluetooth
DEPENDPATH += ../../src/connectivity/bluetooth

include(../mobility_examples.pri)

CONFIG += mobility
MOBILITY = connectivity

SOURCES = \
    main.cpp \
    chat.cpp \
    remoteselector.cpp \
    chatserver.cpp \
    chatclient.cpp

HEADERS = \
    chat.h \
    remoteselector.h \
    chatserver.h \
    chatclient.h

FORMS = \
    chat.ui \
    remoteselector.ui

symbian {
    TARGET.CAPABILITY = LocalServices UserEnvironment ReadUserData WriteUserData NetworkServices NetworkControl ReadDeviceData WriteDeviceData SwEvent
}

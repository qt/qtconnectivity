TARGET = btchat

QT += bluetooth widgets

INCLUDEPATH += ../../src/connectivity/bluetooth
DEPENDPATH += ../../src/connectivity/bluetooth

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

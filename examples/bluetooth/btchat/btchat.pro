TEMPLATE = app
TARGET = btchat

QT = core bluetooth widgets
requires(qtConfig(listwidget))

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

ios: QMAKE_INFO_PLIST = ../shared/Info.qmake.ios.plist
macos: QMAKE_INFO_PLIST = ../shared/Info.qmake.macos.plist

target.path = $$[QT_INSTALL_EXAMPLES]/bluetooth/btchat
INSTALLS += target

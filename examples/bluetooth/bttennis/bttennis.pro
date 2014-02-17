TEMPLATE = app
TARGET = bttennis

QT = core bluetooth widgets
#QT += nfc

SOURCES = \
    main.cpp \
    board.cpp \
    tennis.cpp \
    controller.cpp \
    tennisserver.cpp \
    tennisclient.cpp \
    tennisview.cpp
#    handover.cpp

HEADERS = \
    board.h \
    tennis.h \
    controller.h \
    tennisserver.h \
    tennisclient.h \
    tennisview.h
#    handover.h

FORMS = \
    tennis.ui

RESOURCES += \
    tennis.qrc

OTHER_FILES = \
    connect.png

target.path = $$[QT_INSTALL_EXAMPLES]/bluetooth/bttennis
INSTALLS += target

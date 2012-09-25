QT += nfc widgets

TARGET = ndefeditor
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    textrecordeditor.cpp \
    urirecordeditor.cpp \
    mimeimagerecordeditor.cpp

HEADERS += \
    mainwindow.h \
    textrecordeditor.h \
    urirecordeditor.h \
    mimeimagerecordeditor.h

FORMS += \
    mainwindow.ui \
    textrecordeditor.ui \
    urirecordeditor.ui \
    mimeimagerecordeditor.ui

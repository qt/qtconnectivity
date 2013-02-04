QT += nfc widgets

TARGET = ndefeditor
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    textrecordeditor.cpp \
    urirecordeditor.cpp \
    mimeimagerecordeditor.cpp \
    messageeditor.cpp \
    recordeditor.cpp \
    smartposterrecordeditor.cpp

HEADERS += \
    mainwindow.h \
    textrecordeditor.h \
    urirecordeditor.h \
    mimeimagerecordeditor.h \
    messageeditor.h \
    recordeditor.h \
    smartposterrecordeditor.h

FORMS += \
    mainwindow.ui \
    textrecordeditor.ui \
    urirecordeditor.ui \
    mimeimagerecordeditor.ui \
    messageeditor.ui \
    smartposterrecordeditor.ui

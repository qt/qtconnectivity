TEMPLATE = lib
CONFIG += plugin
PLUGIN_TYPE=serviceframework

QT += concurrent bluetooth serviceframework

HEADERS += bluetoothtransferplugin.h \
           bluetoothtransfer.h
SOURCES += bluetoothtransferplugin.cpp \
           bluetoothtransfer.cpp
TARGET = serviceframework_bluetoothtransferplugin
DESTDIR = .

xml.path = $$QT_MOBILITY_EXAMPLES/xmldata
xml.files = bluetoothtransferservice.xml
INSTALLS += xml

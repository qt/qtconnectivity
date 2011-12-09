TEMPLATE = lib
CONFIG += plugin
PLUGIN_TYPE=serviceframework
INCLUDEPATH += ../../src/serviceframework

HEADERS += bluetoothtransferplugin.h \
           bluetoothtransfer.h
SOURCES += bluetoothtransferplugin.cpp \
           bluetoothtransfer.cpp
TARGET = serviceframework_bluetoothtransferplugin
DESTDIR = .

include(../mobility_examples.pri)
CONFIG += mobility
MOBILITY = serviceframework

symbian {
    load(data_caging_paths)
    pluginDep.sources = serviceframework_bluetoothtransferplugin.dll
    pluginDep.path = $$QT_PLUGINS_BASE_DIR    
    DEPLOYMENT += pluginDep

    xmlautoimport.path = /private/2002AC7F/import/
    xmlautoimport.sources = bluetoothtransferservice.xml
    DEPLOYMENT += xmlautoimport

    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = LocalServices Location NetworkServices ReadUserData WriteUserData UserEnvironment
}

xml.path = $$QT_MOBILITY_EXAMPLES/xmldata
xml.files = bluetoothtransferservice.xml
INSTALLS += xml

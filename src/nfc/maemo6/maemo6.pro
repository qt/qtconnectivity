TEMPLATE = lib
TARGET = nfc_maemo6

CONFIG += static

QT = core dbus

DBUS_INTERFACES += \
    com.nokia.nfc.Manager.xml

DBUS_ADAPTORS += \
    com.nokia.nfc.AccessRequestor.xml \
    com.nokia.nfc.LLCPRequestor.xml \
    com.nokia.nfc.NDEFHandler.xml

# work around bug in Qt
dbus_interface_source.depends = ${QMAKE_FILE_OUT_BASE}.h
dbus_adaptor_source.depends = ${QMAKE_FILE_OUT_BASE}.h

# Link against libdbus until Qt has support for passing file descriptors over DBus.
CONFIG += link_pkgconfig
DEFINES += DBUS_API_SUBJECT_TO_CHANGE
PKGCONFIG += dbus-1

HEADERS += \
    adapter_interface_p.h \
    target_interface_p.h \
    tag_interface_p.h \
    device_interface_p.h

SOURCES += \
    adapter_interface.cpp \
    target_interface.cpp \
    tag_interface.cpp \
    device_interface.cpp

OTHER_FILES += \
    $$DBUS_INTERFACES \
    $$DBUS_ADAPTORS \
    com.nokia.nfc.Adapter.xml \
    com.nokia.nfc.Target.xml \
    com.nokia.nfc.Tag.xml \
    com.nokia.nfc.Device.xml

# Add OUT_PWD to INCLUDEPATH so that creator picks up headers for generated files
# This is not needed for the build otherwise.
INCLUDEPATH += $$OUT_PWD

# We don't need to install this lib, it's only used for building qtconnectivity
# However we do have to make sure that 'make install' builds it.
dummytarget.CONFIG = dummy_install
INSTALLS += dummytarget

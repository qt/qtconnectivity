TARGET = declarative_bluetooth
TARGETPATH = QtBluetooth

include(qbluetoothimport.pri)
target.path = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH
DESTDIR = $$QT.bluetooth.imports/$$TARGETPATH

QT += quick bluetooth network

# Input
HEADERS += \
    qdeclarativebluetoothservice_p.h \
    qdeclarativebluetoothsocket_p.h \
    qdeclarativebluetoothimageprovider_p.h \
    qdeclarativebluetoothdiscoverymodel_p.h

SOURCES += plugin.cpp \
    qdeclarativebluetoothservice.cpp \
    qdeclarativebluetoothsocket.cpp \
    qdeclarativebluetoothdiscoverymodel.cpp \
    qdeclarativebluetoothimageprovider.cpp

RESOURCES += bluetooth.qrc

# plugin.qmltypes is used by Qt Creator for syntax highlighting and the QML code model.  It needs
# to be regenerated whenever the QML elements exported by the plugin change.  This cannot be done
# automatically at compile time because qmlplugindump does not support some QML features and it may
# not be possible when cross-compiling.
#
# To regenerate run 'make qmltypes' which will update the plugins.qmltypes file in the source
# directory.  Then review and commit the changes made to plugins.qmltypes.
#
# This will run the following command:
#     qmlplugindump <import name> <import version> <path to import plugin> > plugins.qmltypes
# e.g.:
#     qmlplugindump QtBluetooth 5.0 imports/QtLocation/libdeclarative_bluetooth.so > plugins.qmltypes

load(resolve_target)
qmltypes.target = qmltypes
qmltypes.commands = $$[QT_INSTALL_BINS]/qmlplugindump QtBluetooth 5.0 $$QMAKE_RESOLVED_TARGET > $$PWD/plugins.qmltypes
message(Should execute $$qmltypes.commands)
qmltypes.depends = $$QMAKE_RESOLVED_TARGET
QMAKE_EXTRA_TARGETS += qmltypes

# Tell qmake to create such makefile that qmldir, plugins.qmltypes and target
# (i.e. declarative_bluetooth) are all copied to $$[QT_INSTALL_IMPORTS]/QtBluetooth directory,

qmldir.files += $$PWD/qmldir $$PWD/plugins.qmltypes
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$TARGETPATH

INSTALLS += target qmldir
message(installs are $$INSTALLS)

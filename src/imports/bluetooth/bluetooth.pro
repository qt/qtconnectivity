TARGET = declarative_bluetooth
TARGETPATH = QtBluetooth

include(qbluetoothimport.pri)
target.path = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH
DESTDIR = $$QT.bluetooth.imports/$$TARGETPATH
INSTALLS += target

qmldir.files += $$PWD/qmldir
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$TARGETPATH
INSTALLS += qmldir

QT += declarative bluetooth network

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


QT += quick bluetooth network

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

load(qml_plugin)

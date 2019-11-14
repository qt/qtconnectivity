# Generated from bluetooth.pro.

#####################################################################
## bluetooth Plugin:
#####################################################################

qt_add_qml_module(declarative_bluetooth # special case
    URI "QtBluetooth"
    CLASSNAME QBluetoothQmlPlugin
    SKIP_TYPE_REGISTRATION
    SOURCES
        plugin.cpp
        qdeclarativebluetoothdiscoverymodel.cpp qdeclarativebluetoothdiscoverymodel_p.h
        qdeclarativebluetoothservice.cpp qdeclarativebluetoothservice_p.h
        qdeclarativebluetoothsocket.cpp qdeclarativebluetoothsocket_p.h
    DEFINES
        QT_DISABLE_DEPRECATED_BEFORE=0
    PUBLIC_LIBRARIES
        Qt::Bluetooth
        Qt::Core
        Qt::Quick
)

#### Keys ignored in scope 1:.:.:bluetooth.pro:<TRUE>:
# TARGETPATH = "QtBluetooth"

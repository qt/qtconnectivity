/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include <QtCore/QLoggingCategory>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlExtensionPlugin>

#include "qdeclarativebluetoothdiscoverymodel_p.h"
#include "qdeclarativebluetoothservice_p.h"
#include "qdeclarativebluetoothsocket_p.h"

QT_USE_NAMESPACE

class QBluetoothQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    QBluetoothQmlPlugin(QObject *parent = nullptr) : QQmlExtensionPlugin(parent) { }
    void registerTypes(const char *uri)
    {
        // @uri QtBluetooth

        Q_ASSERT(uri == QStringLiteral("QtBluetooth"));

        int major = 5;
        int minor = 0;

        // Register the 5.0 types
        //5.0 is silent and not advertised
        qmlRegisterType<QDeclarativeBluetoothDiscoveryModel >(uri, major, minor, "BluetoothDiscoveryModel");
        qmlRegisterType<QDeclarativeBluetoothService        >(uri, major, minor, "BluetoothService");
        qmlRegisterType<QDeclarativeBluetoothSocket         >(uri, major, minor, "BluetoothSocket");

        // Register the 5.2 types
        minor = 2;
        qmlRegisterType<QDeclarativeBluetoothDiscoveryModel >(uri, major, minor, "BluetoothDiscoveryModel");
        qmlRegisterType<QDeclarativeBluetoothService        >(uri, major, minor, "BluetoothService");
        qmlRegisterType<QDeclarativeBluetoothSocket         >(uri, major, minor, "BluetoothSocket");

        // Register the latest Qt version as QML type version
        qmlRegisterModule(uri, QT_VERSION_MAJOR, QT_VERSION_MINOR);
    }
};

Q_LOGGING_CATEGORY(QT_BT_QML, "qt.bluetooth.qml")

#include "plugin.moc"

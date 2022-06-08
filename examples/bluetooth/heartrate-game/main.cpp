// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QLoggingCategory>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "connectionhandler.h"
#include "devicefinder.h"
#include "devicehandler.h"

int main(int argc, char *argv[])
{
    // QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    QGuiApplication app(argc, argv);

    ConnectionHandler connectionHandler;
    DeviceHandler deviceHandler;
    DeviceFinder deviceFinder(&deviceHandler);

    qmlRegisterUncreatableType<DeviceHandler>("Shared", 1, 0, "AddressType", "Enum is not a type");

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("connectionHandler", &connectionHandler);
    engine.rootContext()->setContextProperty("deviceFinder", &deviceFinder);
    engine.rootContext()->setContextProperty("deviceHandler", &deviceHandler);

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    return app.exec();
}

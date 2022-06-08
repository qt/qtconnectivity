// Copyright (C) 2014 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QQmlContext>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QLoggingCategory>
#include "pingpong.h"


int main(int argc, char *argv[])
{
    // QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    QGuiApplication app(argc, argv);
    PingPong pingPong;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("pingPong", &pingPong);
    engine.load(QUrl("qrc:/assets/main.qml"));
    return app.exec();
}

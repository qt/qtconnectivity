// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "connectionhandler.h"
#include "devicefinder.h"
#include "devicehandler.h"
#include "heartrate-global.h"

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QGuiApplication>

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QLoggingCategory>

#ifndef Q_OS_WIN
bool simulator = false;
#else
bool simulator = true;
#endif

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Bluetooth Low Energy Heart Rate Game");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption simulatorOption("simulator", "Simulator");
    parser.addOption(simulatorOption);

    QCommandLineOption verboseOption("verbose", "Verbose mode");
    parser.addOption(verboseOption);
    parser.process(app);

    if (parser.isSet(verboseOption))
        QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    simulator = parser.isSet(simulatorOption);

    ConnectionHandler connectionHandler;
    DeviceHandler deviceHandler;
    DeviceFinder deviceFinder(&deviceHandler);

    QQmlApplicationEngine engine;
    engine.setInitialProperties({
        {"connectionHandler", QVariant::fromValue(&connectionHandler)},
        {"deviceFinder", QVariant::fromValue(&deviceFinder)},
        {"deviceHandler", QVariant::fromValue(&deviceHandler)}
    });

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

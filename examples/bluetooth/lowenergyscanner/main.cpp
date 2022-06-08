// Copyright (C) 2013 BlackBerry Limited. All rights reserved.
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/QLoggingCategory>
#include <QQmlContext>
#include <QGuiApplication>
#include <QQuickView>
#include "device.h"


int main(int argc, char *argv[])
{
    // QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    QGuiApplication app(argc, argv);

    Device d;
    QQuickView view;
    view.rootContext()->setContextProperty("device", &d);

    view.setSource(QUrl("qrc:/assets/main.qml"));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();
    return QGuiApplication::exec();
}

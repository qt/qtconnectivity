// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtGui/QGuiApplication>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickView>
#include <QtQml/qqml.h>

#include <QtCore/QLoggingCategory>

#include "btlocaldevice.h"

int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    QGuiApplication app(argc, argv);
    qmlRegisterType<BtLocalDevice>("Local", 5, 2, "BluetoothDevice");

    QQuickView view;
    view.setSource(QStringLiteral("qrc:///main.qml"));
    view.setWidth(550);
    view.setHeight(550);
    view.setResizeMode(QQuickView::SizeRootObjectToView);

    QObject::connect(view.engine(), SIGNAL(quit()), qApp, SLOT(quit()));
    view.show();

    return QGuiApplication::exec();
}

// Copyright (C) 2013 BlackBerry Limited. All rights reserved.
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "device.h"

#include <QtCore/qloggingcategory.h>
#include <QtGui/qguiapplication.h>
#include <QtQml/qqmlcontext.h>
#include <QtQuick/qquickview.h>

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

// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "device.h"

#include <QtCore/qloggingcategory.h>
#include <QtWidgets/qapplication.h>

int main(int argc, char *argv[])
{
    // QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    QApplication app(argc, argv);
    DeviceDiscoveryDialog d;

    d.show();

    return app.exec();
}

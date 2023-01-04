// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "device.h"

#include <QApplication>
#include <QtCore/QLoggingCategory>

#if QT_CONFIG(permissions)
#include <QPermissions>
#endif

int main(int argc, char *argv[])
{
    // QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    QApplication app(argc, argv);
    DeviceDiscoveryDialog d;

    d.show();

    // Check, and if needed, request a permission to use Bluetooth.
#if QT_CONFIG(permissions)
    const auto permissionStatus = app.checkPermission(QBluetoothPermission{});
    if (permissionStatus == Qt::PermissionStatus::Undetermined) {
        app.requestPermission(QBluetoothPermission{}, [](const QPermission &){
        });
    }
    // Else means either 'Granted' or 'Denied' and both normally must be
    // changed using the system's settings application.
#endif // QT_CONFIG(permissions)

    return app.exec();
}

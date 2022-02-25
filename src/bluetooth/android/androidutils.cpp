/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

#include "androidutils_p.h"

#include <QtCore/QLoggingCategory>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtCore/private/qjnihelpers_p.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

static QString androidPermissionString(BluetoothPermission permission)
{
    switch (permission) {
    case BluetoothPermission::Scan:
        return {QStringLiteral("android.permission.BLUETOOTH_SCAN")};
    case BluetoothPermission::Advertise:
        return {QStringLiteral("android.permission.BLUETOOTH_ADVERTISE")};
    case BluetoothPermission::Connect:
        return {QStringLiteral("android.permission.BLUETOOTH_CONNECT")};
    }
    return {};
}

bool ensureAndroidPermission(BluetoothPermission permission)
{
    // The current set of permissions are only applicable with 31+
    if (QtAndroidPrivate::androidSdkVersion() < 31)
        return true;

    const auto permString = androidPermissionString(permission);

    // First check if we have the permission already
    if (QtAndroidPrivate::checkPermission(permString)
            == QtAndroidPrivate::PermissionsResult::Granted) {
        return true;
    }

    // If we didn't have the permission, request it
    QAndroidJniEnvironment env;
    auto result = QtAndroidPrivate::requestPermissionsSync(env, QStringList() << permString);
    if (result.contains(permString)
            && result[permString] == QtAndroidPrivate::PermissionsResult::Granted) {
        return true;
    }

     qCWarning(QT_BT_ANDROID) << "Permission not authorized:" << permString;
     return false;
}

QT_END_NAMESPACE

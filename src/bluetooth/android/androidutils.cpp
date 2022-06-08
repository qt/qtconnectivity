// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "androidutils_p.h"

#include <QtCore/QLoggingCategory>
#include <QtCore/private/qandroidextras_p.h>

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
    if (QNativeInterface::QAndroidApplication::sdkVersion() < 31)
        return true;

    const auto permString = androidPermissionString(permission);

    // First check if we have the permission already
    if (QtAndroidPrivate::checkPermission(permString).result() == QtAndroidPrivate::Authorized)
        return true;

    // If we didn't have the permission, request it
    if (QtAndroidPrivate::requestPermission(permString).result() == QtAndroidPrivate::Authorized)
        return true;

     qCWarning(QT_BT_ANDROID) << "Permission not authorized:" << permString;
     return false;
}

QT_END_NAMESPACE

/****************************************************************************
**
** Copyright (C) 2023 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
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

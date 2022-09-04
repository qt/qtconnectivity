/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
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

QJniObject getDefaultBluetoothAdapter()
{
    QJniObject service = QJniObject::getStaticObjectField("android/content/Context",
                                                          "BLUETOOTH_SERVICE",
                                                          "Ljava/lang/String;");
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    QJniObject manager = context.callObjectMethod("getSystemService",
                                                  "(Ljava/lang/String;)Ljava/lang/Object;",
                                                  service.object());
    QJniObject adapter;
    if (manager.isValid())
        adapter = manager.callObjectMethod("getAdapter", "()Landroid/bluetooth/BluetoothAdapter;");

    // ### Qt 7 check if the below double-get of the adapter can be removed.
    // It is a workaround for QTBUG-57489, fixed in 2016. According to the bug it occurred on
    // a certain device running Android 6.0.1 (Qt 6 supports Android 6.0 as the minimum).
    // For completeness: the original workaround was for the deprecated getDefaultAdapter()
    // method, and it is thus unclear if this is needed even in Qt 6 anymore. In addition the
    // impacted device is updateable to Android 8 which may also have fixed the issue.
    if (!adapter.isValid())
        adapter = manager.callObjectMethod("getAdapter", "()Landroid/bluetooth/BluetoothAdapter;");

    return adapter;
}

QT_END_NAMESPACE

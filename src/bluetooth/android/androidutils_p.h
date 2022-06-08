// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QANDROIDBLUETOOTHUTILS_H
#define QANDROIDBLUETOOTHUTILS_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qglobal.h>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

enum class BluetoothPermission {
    Scan,
    Advertise,
    Connect
};

// Checks if a permssion is already authorized and requests if not.
// Returns true if permission is successfully authorized
bool ensureAndroidPermission(BluetoothPermission permission);

QT_END_NAMESPACE

#endif // QANDROIDBLUETOOTHUTILS_H

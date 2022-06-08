// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef JNI_ANDROID_P_H
#define JNI_ANDROID_P_H

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

#include <QtCore/QJniObject>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

enum JavaNames {
    BluetoothAdapter = 0,
    BluetoothDevice,
    ActionAclConnected,
    ActionAclDisconnected,
    ActionBondStateChanged,
    ActionDiscoveryStarted,
    ActionDiscoveryFinished,
    ActionFound,
    ActionScanModeChanged,
    ActionUuid,
    ExtraBondState,
    ExtraDevice,
    ExtraPairingKey,
    ExtraPairingVariant,
    ExtraRssi,
    ExtraScanMode,
    ExtraUuid
};

QJniObject valueForStaticField(JavaNames javaName, JavaNames javaFieldName);

QT_END_NAMESPACE

#endif // JNI_ANDROID_P_H

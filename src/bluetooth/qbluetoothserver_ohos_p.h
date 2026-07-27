// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHSERVER_OHOS_P_H
#define QBLUETOOTHSERVER_OHOS_P_H

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
#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/qbluetoothsocket.h>

#include <QtCore/qstring.h>
#include <QtCore/qtconfigmacros.h>
#include <QtCore/qtypes.h>

#include <deque>
#include <memory>

QT_BEGIN_NAMESPACE

namespace QtOhosBluetooth {

class QOhosBluetoothServerProxy;

struct QOhosBluetoothServerProxyContext
{
    quint16 port = 0;
    QBluetooth::SecurityFlags securityFlags = QBluetooth::Security::NoSecurity;
    std::shared_ptr<QOhosBluetoothServerProxy> bluetoothServerProxy;
    std::shared_ptr<QBluetoothSocket> bluetoothSocket;
    std::deque<std::unique_ptr<QBluetoothSocket>> pendingConnectionSockets;
};

std::shared_ptr<QOhosBluetoothServerProxyContext> makeServerProxyContext(quint16 port);
std::shared_ptr<QOhosBluetoothServerProxyContext> tryGetServerProxyContext(quint16 port);

bool isServiceNameAlreadyRegistered(const QString &serviceName);

}

QT_END_NAMESPACE

#endif // QBLUETOOTHSERVER_OHOS_P_H

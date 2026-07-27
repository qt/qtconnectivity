// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QOHOSBLUETOOTHSOCKET_P_H
#define QOHOSBLUETOOTHSOCKET_P_H

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

#include <QtBluetooth/private/qohosbluetoothcommon_p.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qtconfigmacros.h>
#include <QtCore/qtypes.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

QT_BEGIN_NAMESPACE

namespace QtOhosBluetooth {

constexpr int invalidSocketDescriptor = -1;

std::shared_ptr<int> makeSppClientSocketHandle(int socketDescriptor);
std::shared_ptr<int> makeSppServerSocketHandle(int socketDescriptor);

class QOhosBluetoothSocketProxy : public QObject
{
    Q_OBJECT
public:
    QOhosBluetoothSocketProxy();

    bool connect(const QString &deviceId, const QString &uuid, bool secure);
    bool takeOverSocket(std::shared_ptr<int> socketHandle);
    bool write(const char *data, qint64 maxSize);
    void close();

    std::optional<QString> tryGetPeerDeviceAddress() const;
    std::optional<QString> tryGetPeerDeviceName() const;

    static bool registerPendingSocketHandle(std::shared_ptr<int> socketHandle);
    static void dropPendingSocketHandle(int socketDescriptor);
    static std::shared_ptr<int> takePendingSocketHandle(int socketDescriptor);

Q_SIGNALS:
    void connected(int socketDescriptor);
    void disconnected();
    void dataReceived(const QByteArray &data);
    void errorOccurred(QOhosBluetoothErrorCode errorCode);

private:
    void onSocketConnected(std::uint64_t connectRequestId, std::shared_ptr<int> socketHandle);
    void onConnectFailed(std::uint64_t connectRequestId, QOhosBluetoothErrorCode errorCode);
    void reportError(QOhosBluetoothErrorCode errorCode);

    struct JsScopeData
    {
        std::shared_ptr<int> socketHandle;
        std::shared_ptr<void> sppReadHandlerHandle;
    };

    std::shared_ptr<JsScopeData> m_jsScopeData;
    std::optional<std::string> m_deviceId;
    std::uint64_t m_connectRequestsCounter = 0;
    std::optional<std::uint64_t> m_pendingConnectRequestId;

    static std::unordered_map<int, std::shared_ptr<int>> &pendingSocketHandles();
};

}

QT_END_NAMESPACE

#endif // QOHOSBLUETOOTHSOCKET_P_H

// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtBluetooth/private/qbluetoothserver_ohos_p.h>
#include <QtBluetooth/private/qohosbluetoothserver_p.h>

#include <iterator>
#include <limits>
#include <optional>
#include <unordered_map>

QT_BEGIN_NAMESPACE

namespace QtOhosBluetooth {

namespace {

std::unordered_map<quint16, std::weak_ptr<QOhosBluetoothServerProxyContext>> &serverProxyContexts()
{
    static std::unordered_map<quint16, std::weak_ptr<QOhosBluetoothServerProxyContext>> contexts;
    return contexts;
}

void dropExpiredServerProxyContexts()
{
    auto &contexts = serverProxyContexts();
    for (auto context = contexts.begin(); context != contexts.end();)
        context = context->second.expired() ? contexts.erase(context) : std::next(context);
}

std::optional<quint16> tryFindFreeServerPort()
{
    constexpr int maxServerPort = std::numeric_limits<quint8>::max();
    for (int candidatePort = 1; candidatePort <= maxServerPort; ++candidatePort) {
        if (!tryGetServerProxyContext(quint16(candidatePort)))
            return quint16(candidatePort);
    }

    return std::nullopt;
}

}

std::shared_ptr<QOhosBluetoothServerProxyContext> makeServerProxyContext(quint16 port)
{
    dropExpiredServerProxyContexts();

    const auto optServerPort = port != 0
        ? std::make_optional(port)
        : tryFindFreeServerPort();
    if (!optServerPort || tryGetServerProxyContext(*optServerPort))
        return nullptr;

    auto serverProxyContext = std::make_shared<QOhosBluetoothServerProxyContext>();
    serverProxyContext->port = *optServerPort;
    serverProxyContext->bluetoothServerProxy = std::make_shared<QOhosBluetoothServerProxy>();
    serverProxyContext->bluetoothSocket = std::make_shared<QBluetoothSocket>();
    serverProxyContexts()[serverProxyContext->port] = serverProxyContext;

    return serverProxyContext;
}

std::shared_ptr<QOhosBluetoothServerProxyContext> tryGetServerProxyContext(quint16 port)
{
    auto &contexts = serverProxyContexts();
    const auto found = contexts.find(port);
    return found != contexts.end()
        ? found->second.lock()
        : nullptr;
}

}

QT_END_NAMESPACE

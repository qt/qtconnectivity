// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothserviceinfo_p.h"

#include <QtBluetooth/qbluetoothserviceinfo.h>
#include <QtBluetooth/private/qbluetoothserver_ohos_p.h>
#include <QtBluetooth/private/qohosbluetoothserver_p.h>

#include <QtCore/qloggingcategory.h>

#include <optional>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_OHOS)

namespace {

std::optional<quint16> tryGetRfcommServerPort(int serverChannel)
{
    if (serverChannel < 0) {
        qCWarning(QT_BT_OHOS, "%s: Only RFCOMM services can be registered on HarmonyOS", Q_FUNC_INFO);
        return std::nullopt;
    }

    if (serverChannel == 0) {
        qCCritical(
            QT_BT_OHOS, "%s: No RFCOMM port in the service. Cannot register service.", Q_FUNC_INFO);
        return std::nullopt;
    }

    return static_cast<quint16>(serverChannel);
}

}

QBluetoothServiceInfoPrivate::QBluetoothServiceInfoPrivate() = default;

QBluetoothServiceInfoPrivate::~QBluetoothServiceInfoPrivate()
{
    unregisterService();
}

bool QBluetoothServiceInfoPrivate::isRegisteredWithServerProxy() const
{
    const auto serverProxy = registeredServerProxy.lock();
    return serverProxy && optRegisteredServiceId
        && serverProxy->tryGetRegisteredServiceId() == optRegisteredServiceId;
}

bool QBluetoothServiceInfoPrivate::isRegistered() const
{
    return registered && isRegisteredWithServerProxy();
}

bool QBluetoothServiceInfoPrivate::unregisterService()
{
    if (!registered)
        return false;

    registered = false;

    if (isRegisteredWithServerProxy()) {
        registeredServerProxy.lock()->close();
    } else {
        qCDebug(
            QT_BT_OHOS, "%s: the server is already closed. The service is unregistered anyway.",
            Q_FUNC_INFO);
    }

    registeredServerProxy.reset();
    optRegisteredServiceId.reset();

    return true;
}

bool QBluetoothServiceInfoPrivate::registerService(const QBluetoothAddress &localAdapter)
{
    if (!localAdapter.isNull()) {
        qCWarning(
            QT_BT_OHOS,
            "%s: HarmonyOS does not support multi device adapters. Ignoring local adapter address ...",
            Q_FUNC_INFO);
    }

    if (isRegistered())
        return false;

    registered = false;
    registeredServerProxy.reset();
    optRegisteredServiceId.reset();

    const auto optPort = tryGetRfcommServerPort(serverChannel());
    if (!optPort)
        return false;

    const auto serviceName = attributes.value(QBluetoothServiceInfo::ServiceName).toString();
    const auto uuid = attributes.value(QBluetoothServiceInfo::ServiceId).value<QBluetoothUuid>();

    if (serviceName.isEmpty()) {
        qCCritical(
            QT_BT_OHOS, "%s: No service name in the service. Cannot register service.",
            Q_FUNC_INFO);
        return false;
    }

    if (uuid.isNull()) {
        qCCritical(
            QT_BT_OHOS, "%s: No service UUID in the service. Cannot register service.",
            Q_FUNC_INFO);
        return false;
    }

    if (QtOhosBluetooth::isServiceNameAlreadyRegistered(serviceName)) {
        qCCritical(QT_BT_OHOS, "%s: Server with such service name is registered", Q_FUNC_INFO);
        return false;
    }

    auto serverProxyContext = QtOhosBluetooth::tryGetServerProxyContext(*optPort);
    if (!serverProxyContext) {
        qCCritical(QT_BT_OHOS, "%s: Server is not active. Cannot register services.", Q_FUNC_INFO);
        return false;
    }

    auto proxy = serverProxyContext->bluetoothServerProxy;

    if (proxy->isServiceRegistered()) {
        qCCritical(
            QT_BT_OHOS, "%s: Server has already registered service. Cannot register services.",
            Q_FUNC_INFO);
        return false;
    }

    const auto secure = (serverProxyContext->securityFlags != QBluetooth::Security::NoSecurity);

    if (!proxy->listen(serviceName, uuid.toString(QUuid::WithoutBraces), secure))
        return false;

    if (!proxy->accept()) {
        proxy->close();
        return false;
    }

    registered = true;
    registeredServerProxy = proxy;
    optRegisteredServiceId = proxy->tryGetRegisteredServiceId();

    return true;
}

QT_END_NAMESPACE

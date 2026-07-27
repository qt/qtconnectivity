// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothserver_ohos_p.h"

#include <QtBluetooth/qbluetoothserver.h>
#include <QtBluetooth/qbluetoothsocket.h>
#include <QtBluetooth/private/qbluetoothserver_p.h>
#include <QtBluetooth/private/qohosbluetoothaccess_p.h>
#include <QtBluetooth/private/qohosbluetoothcommon_p.h>
#include <QtBluetooth/private/qohosbluetoothserver_p.h>
#include <QtBluetooth/private/qohosbluetoothsocket_p.h>

#include <QtCore/q20utility.h>
#include <QtCore/qloggingcategory.h>

#include <memory>

QT_BEGIN_NAMESPACE

using QtOhosBluetooth::QOhosBluetoothErrorCode;

Q_DECLARE_LOGGING_CATEGORY(QT_BT_OHOS)

namespace {

QBluetoothServer::Error mapToServerError(QOhosBluetoothErrorCode errorCode)
{
    switch (errorCode) {
    case QOhosBluetoothErrorCode::PermissionDenied:
        return QBluetoothServer::MissingPermissionsError;
    case QOhosBluetoothErrorCode::CapabilityNotSupported:
    case QOhosBluetoothErrorCode::ProfileNotSupported:
        return QBluetoothServer::UnsupportedProtocolError;
    case QOhosBluetoothErrorCode::InputOutput:
        return QBluetoothServer::InputOutputError;
    case QOhosBluetoothErrorCode::BluetoothDisabled:
    case QOhosBluetoothErrorCode::ServiceStopped:
        return QBluetoothServer::PoweredOffError;
    case QOhosBluetoothErrorCode::InvalidParameter:
    case QOhosBluetoothErrorCode::UserDidNotRespond:
    case QOhosBluetoothErrorCode::UserRefused:
    case QOhosBluetoothErrorCode::OperationFailed:
        break;
    }

    return QBluetoothServer::UnknownError;
}

}

QBluetoothServerPrivate::QBluetoothServerPrivate(
        QBluetoothServiceInfo::Protocol sType, QBluetoothServer *parent)
    : serverType(sType)
    , q_ptr(parent)
{
}

QBluetoothServerPrivate::~QBluetoothServerPrivate() = default;

bool QBluetoothServerPrivate::isListening() const
{
    return socket != nullptr && socket->state() == QBluetoothSocket::SocketState::ListeningState;
}

void QBluetoothServerPrivate::resetServerContext()
{
    socket = nullptr;
    m_bluetoothServerProxyContext.reset();
}

void QBluetoothServer::close()
{
    d_ptr->resetServerContext();
}

bool QBluetoothServer::listen(const QBluetoothAddress &address, quint16 port)
{
    if (!address.isNull()) {
        qCWarning(
            QT_BT_OHOS,
            "%s: HarmonyOS does not support multi device adapters. Ignoring local adapter address ...",
            Q_FUNC_INFO);
    }

    const auto reportError = [this](Error error) {
        d_ptr->m_lastError = error;
        QMetaObject::invokeMethod(
            this,
            [this, error]() {
                Q_EMIT errorOccurred(error);
            },
            Qt::QueuedConnection);
    };

    if (serverType() != QBluetoothServiceInfo::RfcommProtocol) {
        reportError(UnsupportedProtocolError);
        return false;
    }

    if (!QtOhosBluetooth::checkAccessBluetoothPermissionGranted(Q_FUNC_INFO)) {
        reportError(MissingPermissionsError);
        return false;
    }

    auto accessProxy = QtOhosBluetooth::QOhosBluetoothAccessProxy::instance();
    if (accessProxy->tryGetBluetoothState()
        != QtOhosBluetooth::QOhosBluetoothAccessProxy::BluetoothState::STATE_ON) {
        reportError(PoweredOffError);
        return false;
    }

    if (isListening()) {
        reportError(ServiceAlreadyRegisteredError);
        return false;
    }

    if (d_ptr->m_bluetoothServerProxyContext
        && !d_ptr->m_bluetoothServerProxyContext->bluetoothServerProxy->isServiceRegistered()) {
        d_ptr->resetServerContext();
    }

    if (!d_ptr->m_bluetoothServerProxyContext) {
        d_ptr->m_bluetoothServerProxyContext = QtOhosBluetooth::makeServerProxyContext(port);
        if (!d_ptr->m_bluetoothServerProxyContext) {
            reportError(ServiceAlreadyRegisteredError);
            return false;
        }
        d_ptr->m_bluetoothServerProxyContext->securityFlags = d_ptr->securityFlags;
        d_ptr->socket = d_ptr->m_bluetoothServerProxyContext->bluetoothSocket.get();

        auto serverProxy = d_ptr->m_bluetoothServerProxyContext->bluetoothServerProxy.get();

        QObject::connect(
            serverProxy, &QtOhosBluetooth::QOhosBluetoothServerProxy::clientAccepted,
            this,
            [this](int clientSocketDescriptor) {
                auto context = d_ptr->m_bluetoothServerProxyContext;
                if (!context) {
                    QtOhosBluetooth::QOhosBluetoothSocketProxy::dropPendingSocketHandle(
                        clientSocketDescriptor);
                    return;
                }

                if (q20::cmp_greater_equal(
                        context->pendingConnectionSockets.size(),
                        d_ptr->maxPendingConnections)) {
                    qCDebug(
                        QT_BT_OHOS,
                        "%s: maximum number of pending connections reached. Rejecting the client ...",
                        Q_FUNC_INFO);
                    QtOhosBluetooth::QOhosBluetoothSocketProxy::dropPendingSocketHandle(
                        clientSocketDescriptor);
                    return;
                }

                auto pendingConnectionSocket = std::make_unique<QBluetoothSocket>();
                const auto socketDescriptorSet = pendingConnectionSocket->setSocketDescriptor(
                    clientSocketDescriptor, QBluetoothServiceInfo::RfcommProtocol,
                    QBluetoothSocket::SocketState::ConnectedState, QBluetoothSocket::ReadWrite);

                if (!socketDescriptorSet) {
                    QtOhosBluetooth::QOhosBluetoothSocketProxy::dropPendingSocketHandle(
                        clientSocketDescriptor);
                    return;
                }

                context->pendingConnectionSockets.push_back(std::move(pendingConnectionSocket));
                Q_EMIT newConnection();
            });

        QObject::connect(
            serverProxy, &QtOhosBluetooth::QOhosBluetoothServerProxy::errorOccurred,
            this,
            [this](QOhosBluetoothErrorCode errorCode) {
                d_ptr->m_lastError = mapToServerError(errorCode);
                Q_EMIT errorOccurred(d_ptr->m_lastError);
            });

        QObject::connect(
            serverProxy, &QtOhosBluetooth::QOhosBluetoothServerProxy::acceptingStopped,
            this,
            [this]() {
                auto context = d_ptr->m_bluetoothServerProxyContext;
                if (context && context->bluetoothServerProxy->isServiceRegistered())
                    return;

                if (d_ptr->socket)
                    d_ptr->socket->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
            });
    }

    d_ptr->socket->setSocketState(QBluetoothSocket::SocketState::ListeningState);
    return true;
}

void QBluetoothServer::setMaxPendingConnections(int numConnections)
{
    d_ptr->maxPendingConnections = numConnections;
}

bool QBluetoothServer::hasPendingConnections() const
{
    auto context = d_ptr->m_bluetoothServerProxyContext;
    return context && !context->pendingConnectionSockets.empty();
}

QBluetoothSocket *QBluetoothServer::nextPendingConnection()
{
    auto context = d_ptr->m_bluetoothServerProxyContext;
    if (!context || context->pendingConnectionSockets.empty())
        return nullptr;

    auto pendingConnectionSocket = std::move(context->pendingConnectionSockets.front());
    context->pendingConnectionSockets.pop_front();

    return pendingConnectionSocket.release();
}

QBluetoothAddress QBluetoothServer::serverAddress() const
{
    return {};
}

quint16 QBluetoothServer::serverPort() const
{
    auto context = d_ptr->m_bluetoothServerProxyContext;
    return context ? context->port : 0;
}

void QBluetoothServer::setSecurityFlags(QBluetooth::SecurityFlags security)
{
    if (isListening()) {
        qCWarning(
            QT_BT_OHOS,
            "%s: the security flags must be set before listen(). Ignoring the new flags ...",
            Q_FUNC_INFO);
        return;
    }

    d_ptr->securityFlags = security;
}

QBluetooth::SecurityFlags QBluetoothServer::securityFlags() const
{
    return d_ptr->securityFlags;
}

QT_END_NAMESPACE

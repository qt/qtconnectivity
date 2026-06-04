// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qohosbluetoothsocket_p.h"

#include <QtBluetooth/private/qohosbluetoothaccess_p.h>
#include <QtBluetooth/private/qohosbluetoothcommon_p.h>
#include <QtBluetooth/private/qohosbluetoothenums_p.h>
#include <QtBluetooth/private/qohosbluetoothremotedevice_p.h>

#include <QtCore/q26numeric.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/private/qcore_ohos_p.h>
#include <QtCore/private/qnapi_p.h>
#include <QtCore/private/qohosjstools_p.h>
#include <QtCore/private/qohoslogger_p.h>

#include <cstddef>
#include <cstring>
#include <memory>
#include <optional>
#include <string>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_OHOS)

namespace QtOhosBluetooth {

namespace {

std::shared_ptr<int> makeSppSocketHandle(int socketDescriptor, const char *closeCallExpression)
{
    return QtOhos::makeSharedPtrWithAttachedExtraData(
        std::make_shared<int>(socketDescriptor),
        QtOhos::makeDestroyNotifier(
            [socketDescriptor, closeCallExpression]() {
                QOhosJsThreadGateway::invoke(
                    [socketDescriptor, closeCallExpression](QOhosJsState &jsState) {
                        try {
                            jsState.eval(closeCallExpression, {socketDescriptor});
                        } catch (const Napi::Error &error) {
                            const auto errorCode =
                                tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)
                                    .value_or(QOhosBluetoothErrorCode::OperationFailed);
                            qOhosPrintfWarning(
                                "%s: cannot close the socket %d with %s, error code: %u",
                                Q_FUNC_INFO, socketDescriptor, closeCallExpression,
                                qToUnderlying(errorCode));
                        }
                    });
            }));
}

}

std::shared_ptr<int> makeSppClientSocketHandle(int socketDescriptor)
{
    return makeSppSocketHandle(socketDescriptor, "@ohos.bluetooth.socket.sppCloseClientSocket(*)");
}

namespace {

using SppType = QtOhosBluetooth::enums::ohos::bluetooth::socket::SppType;

std::shared_ptr<void> registerSocketSppReadListener(
    QOhosJsState &jsState, int socketDescriptor, QOhosConsumer<QByteArray> sppReadListener,
    QOhosConsumer<QOhosBluetoothErrorCode> errorCodeConsumer)
{
    return registerQOhosOnOffMethodsBasedEventHandler(
        jsState.eval<QNapi::Object>("@ohos.bluetooth.socket"), "sppRead",
        [sppReadListener = std::move(sppReadListener)](const QOhosCallbackInfo &cbInfo) {
            auto arrayBuffer = cbInfo.getFirstArg<QNapi::ArrayBuffer>(Q_FUNC_INFO);
            sppReadListener(
                QByteArray(
                    static_cast<const char *>(arrayBuffer.Data()),
                    static_cast<qsizetype>(arrayBuffer.ByteLength())));
        },
        {
            .extraOnArg = std::make_optional(QNapi::ValueWrapper(socketDescriptor)),
            .extraOffArg = std::make_optional(QNapi::ValueWrapper(socketDescriptor)),
            .optOnCallExceptionHandler =
                [errorCodeConsumer = std::move(errorCodeConsumer)](const Napi::Error &error) {
                    errorCodeConsumer(
                        tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)
                            .value_or(QOhosBluetoothErrorCode::OperationFailed));
                },
        });
}

std::shared_ptr<void> registerSocketSppReadHandler(
    int socketDescriptor, QOhosConsumer<QByteArray> receivedDataConsumer,
    QOhosConsumer<QOhosBluetoothErrorCode> errorCodeConsumer)
{
    auto sharedReceivedDataConsumer = QtOhos::moveToSharedPtr(std::move(receivedDataConsumer));
    auto weakReceivedDataConsumer = QtOhos::makeWeakPtr(sharedReceivedDataConsumer);

    auto socketSppReadHandler = QOhosJsThreadGateway::eval(
        [&](QOhosJsState &jsState) {
            return registerSocketSppReadListener(
                jsState, socketDescriptor,
                [weakReceivedDataConsumer](QByteArray receivedData) {
                    QtOhos::invokeInQtThread(
                        [weakReceivedDataConsumer, receivedData = std::move(receivedData)]() {
                            auto receivedDataConsumer = weakReceivedDataConsumer.lock();
                            if (receivedDataConsumer)
                                (*receivedDataConsumer)(std::move(receivedData));
                        });
                },
                std::move(errorCodeConsumer));
        },
        Q_FUNC_INFO);

    if (!socketSppReadHandler)
        return nullptr;

    return QtOhos::makeSharedPtrWithAttachedExtraData(
        socketSppReadHandler, sharedReceivedDataConsumer);
}

std::optional<bool> tryReadSocketConnectedState(QOhosJsState &jsState, int socketDescriptor)
{
    try {
        return jsState
            .eval<QNapi::Boolean>(
                "@ohos.bluetooth.socket.isConnected(*)", {socketDescriptor})
            .Value();
    } catch (const Napi::Error &error) {
        const auto errorCode =
            tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)
                .value_or(QOhosBluetoothErrorCode::OperationFailed);
        qOhosPrintfWarning(
            "%s: cannot read the socket connected state, error code: %u", Q_FUNC_INFO,
            qToUnderlying(errorCode));
        return std::nullopt;
    }
}

std::optional<std::string> tryGetDeviceIdFromSocket(int socket)
{
    if (socket == invalidSocketDescriptor)
        return std::nullopt;

    return QOhosJsThreadGateway::eval(
        [&](QOhosJsState &jsState) -> std::optional<std::string> {
            try {
                return jsState.eval<QNapi::String>(
                    "@ohos.bluetooth.socket.getDeviceId(*)", {socket});
            } catch (const Napi::Error &error) {
                if (const auto optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)) {
                    qOhosPrintfWarning(
                        "%s: cannot read the device id of the socket %d, error code: %u",
                        Q_FUNC_INFO, socket, qToUnderlying(*optErrorCode));
                }
                return std::nullopt;
            }
        });
}

}

QOhosBluetoothSocketProxy::QOhosBluetoothSocketProxy()
{
    QObject::connect(
        QOhosBluetoothAccessProxy::instance().get(),
        &QOhosBluetoothAccessProxy::stateChanged,
        this,
        [this](QOhosBluetoothAccessProxy::BluetoothState bluetoothState) {
            if (bluetoothState == QOhosBluetoothAccessProxy::BluetoothState::STATE_ON)
                return;

            if (m_pendingConnectRequestId) {
                qCWarning(
                    QT_BT_OHOS, "%s: bluetooth is turned off. Aborting the connect request ...",
                    Q_FUNC_INFO);
                onConnectFailed(
                    *m_pendingConnectRequestId, QOhosBluetoothErrorCode::BluetoothDisabled);
                return;
            }

            if (!m_jsScopeData)
                return;

            qCWarning(
                QT_BT_OHOS, "%s: bluetooth is turned off. Closing the socket ...", Q_FUNC_INFO);
            reportError(QOhosBluetoothErrorCode::BluetoothDisabled);
            close();
        });
}

std::unordered_map<int, std::shared_ptr<int>> &QOhosBluetoothSocketProxy::pendingSocketHandles()
{
    static std::unordered_map<int, std::shared_ptr<int>> handles;
    return handles;
}

std::shared_ptr<int> QOhosBluetoothSocketProxy::takePendingSocketHandle(int socketDescriptor)
{
    auto &handles = pendingSocketHandles();
    auto found = handles.find(socketDescriptor);
    if (found == handles.end())
        return nullptr;

    auto socketHandle = std::move(found->second);
    handles.erase(found);
    return socketHandle;
}

bool QOhosBluetoothSocketProxy::connect(const QString &deviceId, const QString &uuid, bool secure)
{
    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO)) {
        reportError(QOhosBluetoothErrorCode::PermissionDenied);
        return false;
    }

    const auto connectRequestId = ++m_connectRequestsCounter;
    m_pendingConnectRequestId = connectRequestId;

    auto selfRef = QtOhos::makeQThreadSafeRef(this);
    auto optConnectErrorCode = QOhosJsThreadGateway::eval(
        [&](QOhosJsState &jsState) -> std::optional<QOhosBluetoothErrorCode> {
            auto sppOptions = QNapi::makeObject(
                jsState.env(),
                {
                    {"uuid", uuid.toStdString()},
                    {"secure", secure},
                    {"type", jsState.mapOhosEnumToJs(SppType::SPP_RFCOMM)},
                });

            try {
                jsState.eval(
                    "@ohos.bluetooth.socket.sppConnect(*)",
                    {
                        deviceId.toStdString(),
                        sppOptions,
                        [selfRef, connectRequestId](const QNapi::CallbackInfo &callbackInfo) {
                            QNapi::Value error;
                            QNapi::Value data;
                            callbackInfo.getLeadingArgs(Q_FUNC_INFO, error, data);
                            auto socketId = data.IsNumber()
                                ? QNapi::checkedCast<QNapi::Number>(data).Int32Value()
                                : invalidSocketDescriptor;

                            if (socketId == invalidSocketDescriptor) {
                                const auto errorCode =
                                    tryGetKnownErrorCode(error, Q_FUNC_INFO)
                                        .value_or(QOhosBluetoothErrorCode::OperationFailed);
                                qOhosPrintfError(
                                    "%s: sppConnect() failed with error code: %u", Q_FUNC_INFO,
                                    qToUnderlying(errorCode));
                                selfRef.visitInQtThreadIfAlive(
                                    [connectRequestId, errorCode](auto &self) {
                                        self.onConnectFailed(connectRequestId, errorCode);
                                    });
                                return;
                            }

                            auto socketHandle = makeSppClientSocketHandle(socketId);

                            selfRef.visitInQtThreadIfAlive(
                                [connectRequestId, socketHandle](auto &self) {
                                    self.onSocketConnected(connectRequestId, socketHandle);
                                });
                        }
                    });
                return std::nullopt;
            } catch (const Napi::Error &error) {
                return tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO);
            }
        });

    if (optConnectErrorCode) {
        qCCritical(
            QT_BT_OHOS, "%s: sppConnect() failed with error code: %u", Q_FUNC_INFO,
            qToUnderlying(*optConnectErrorCode));
        onConnectFailed(connectRequestId, *optConnectErrorCode);
        return false;
    }

    m_deviceId = deviceId.toStdString();

    return true;
}

void QOhosBluetoothSocketProxy::onSocketConnected(
    std::uint64_t connectRequestId, std::shared_ptr<int> socketHandle)
{
    if (m_pendingConnectRequestId != connectRequestId) {
        qCDebug(
            QT_BT_OHOS,
            "%s: the connection has been closed before it was established. Letting the socket "
            "handle close the socket ...",
            Q_FUNC_INFO);
        return;
    }

    m_pendingConnectRequestId.reset();

    if (!takeOverSocket(std::move(socketHandle))) {
        m_deviceId.reset();
        reportError(QOhosBluetoothErrorCode::OperationFailed);
    }
}

void QOhosBluetoothSocketProxy::onConnectFailed(
    std::uint64_t connectRequestId, QOhosBluetoothErrorCode errorCode)
{
    if (m_pendingConnectRequestId != connectRequestId)
        return;

    m_pendingConnectRequestId.reset();
    m_deviceId.reset();

    reportError(errorCode);
}

bool QOhosBluetoothSocketProxy::takeOverSocket(std::shared_ptr<int> socketHandle)
{
    if (!socketHandle) {
        qCCritical(QT_BT_OHOS, "%s: no socket handle to take over", Q_FUNC_INFO);
        return false;
    }

    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO))
        return false;

    auto selfRef = QtOhos::makeQThreadSafeRef(this);
    auto optSubscriptionErrorCode = std::make_shared<std::optional<QOhosBluetoothErrorCode>>();

    auto sppReadHandlerHandle = registerSocketSppReadHandler(
        *socketHandle,
        [selfRef](QByteArray receivedData) {
            if (receivedData.isEmpty())
                return;

            selfRef.visitInQtThreadIfAlive(
                [receivedData = std::move(receivedData)](auto &self) {
                    Q_EMIT self.dataReceived(receivedData);
                });
        },
        [optSubscriptionErrorCode](QOhosBluetoothErrorCode errorCode) {
            *optSubscriptionErrorCode = errorCode;
        });

    if (!sppReadHandlerHandle) {
        qCCritical(
            QT_BT_OHOS,
            "%s: cannot register the SPP read handler, error code: %u. Socket is not connected.",
            Q_FUNC_INFO,
            qToUnderlying(
                optSubscriptionErrorCode->value_or(QOhosBluetoothErrorCode::OperationFailed)));
        return false;
    }

    m_jsScopeData = QtOhos::makeProxyWithJsThreadDeleter(
        QtOhos::moveToSharedPtr(
            JsScopeData {
                .socketHandle = socketHandle,
                .sppReadHandlerHandle = std::move(sppReadHandlerHandle),
            }));

    const auto socketDescriptor = *socketHandle;
    QMetaObject::invokeMethod(
        this,
        [this, socketDescriptor]() {
            Q_EMIT connected(socketDescriptor);
        },
        Qt::QueuedConnection);

    return true;
}

bool QOhosBluetoothSocketProxy::write(const char *data, qint64 maxSize)
{
    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO)) {
        reportError(QOhosBluetoothErrorCode::PermissionDenied);
        return false;
    }

    if (!m_jsScopeData) {
        qCWarning(QT_BT_OHOS, "%s: the socket is already closed", Q_FUNC_INFO);
        reportError(QOhosBluetoothErrorCode::OperationFailed);
        return false;
    }

    const auto socketDescriptor = *m_jsScopeData->socketHandle;

    struct WriteOutcome
    {
        std::optional<QOhosBluetoothErrorCode> optErrorCode = std::nullopt;
        bool socketStillConnected = true;
    };

    const auto writeSize = q26::saturate_cast<std::size_t>(maxSize);
    const auto writeOutcome = QOhosJsThreadGateway::eval(
        [&](QOhosJsState &jsState) -> WriteOutcome {
            try {
                auto arrayBuffer = QNapi::ArrayBuffer::New(jsState.env(), writeSize);
                std::memcpy(arrayBuffer.Data(), data, writeSize);
                jsState.eval("@ohos.bluetooth.socket.sppWrite(*)", {socketDescriptor, arrayBuffer});
                return {};
            } catch (const Napi::Error &error) {
                return {
                    .optErrorCode = tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO),
                    .socketStillConnected =
                        tryReadSocketConnectedState(jsState, socketDescriptor).value_or(true),
                };
            }
        });

    if (!writeOutcome.optErrorCode)
        return true;

    reportError(*writeOutcome.optErrorCode);

    if (!writeOutcome.socketStillConnected) {
        qCWarning(
            QT_BT_OHOS, "%s: the peer is no longer connected. Closing the socket ...", Q_FUNC_INFO);
        close();
    }

    return false;
}

void QOhosBluetoothSocketProxy::close()
{
    if (!m_jsScopeData && !m_pendingConnectRequestId)
        return;

    m_jsScopeData.reset();
    m_pendingConnectRequestId.reset();
    m_deviceId.reset();

    QMetaObject::invokeMethod(this, &QOhosBluetoothSocketProxy::disconnected, Qt::QueuedConnection);
}

std::optional<QString> QOhosBluetoothSocketProxy::tryGetPeerDeviceAddress() const
{
    const auto peerDeviceIdFromSocket = m_jsScopeData
        ? tryGetDeviceIdFromSocket(*m_jsScopeData->socketHandle)
        : std::optional<std::string>();
    const auto &optPeerDeviceId = peerDeviceIdFromSocket
        ? peerDeviceIdFromSocket
        : m_deviceId;

    return optPeerDeviceId
        ? std::optional(QString::fromStdString(*optPeerDeviceId))
        : std::nullopt;
}

std::optional<QString> QOhosBluetoothSocketProxy::tryGetPeerDeviceName() const
{
    const auto deviceId = tryGetPeerDeviceAddress();
    if (!deviceId)
        return std::nullopt;

    auto remoteDeviceProxy = QOhosBluetoothRemoteDeviceProxy::instance();
    return remoteDeviceProxy->tryGetRemoteDeviceName(*deviceId);
}

void QOhosBluetoothSocketProxy::reportError(QOhosBluetoothErrorCode errorCode)
{
    QMetaObject::invokeMethod(
        this,
        [this, errorCode]() {
            Q_EMIT errorOccurred(errorCode);
        },
        Qt::QueuedConnection);
}

}

QT_END_NAMESPACE

#include "moc_qohosbluetoothsocket_p.cpp"

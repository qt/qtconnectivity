// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qohosbluetoothserver_p.h"

#include <QtBluetooth/private/qohosbluetoothaccess_p.h>
#include <QtBluetooth/private/qohosbluetoothcommon_p.h>
#include <QtBluetooth/private/qohosbluetoothenums_p.h>
#include <QtBluetooth/private/qohosbluetoothsocket_p.h>

#include <QtCore/qloggingcategory.h>
#include <QtCore/private/qcore_ohos_p.h>
#include <QtCore/private/qnapi_p.h>
#include <QtCore/private/qohoslogger_p.h>

#include <memory>
#include <optional>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_OHOS)

namespace QtOhosBluetooth {

namespace {

using SppType = QtOhosBluetooth::enums::ohos::bluetooth::socket::SppType;

}

QOhosBluetoothServerProxy::QOhosBluetoothServerProxy()
{
    QObject::connect(
        QOhosBluetoothAccessProxy::instance().get(),
        &QOhosBluetoothAccessProxy::stateChanged,
        this,
        [this](QOhosBluetoothAccessProxy::BluetoothState bluetoothState) {
            if (bluetoothState == QOhosBluetoothAccessProxy::BluetoothState::STATE_ON)
                return;

            if (!m_serverSocketHandle)
                return;

            const auto self = weak_from_this().lock();
            if (!self)
                return;

            qCWarning(
                QT_BT_OHOS, "%s: bluetooth is turned off. Closing the server ...", Q_FUNC_INFO);
            reportError(QOhosBluetoothErrorCode::BluetoothDisabled);
            close();
        });
}

bool QOhosBluetoothServerProxy::listen(const QString &serviceName, const QString &uuid, bool secure)
{
    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO)) {
        reportError(QOhosBluetoothErrorCode::PermissionDenied);
        return false;
    }

    struct SppListenResult
    {
        int serverSocketId = invalidSocketDescriptor;
        std::optional<QOhosBluetoothErrorCode> optErrorCode = std::nullopt;
    };

    const auto listenResult = QOhosJsThreadGateway::evalWithPromise<SppListenResult>(
        [&](QOhosJsState &jsState, QOhosTaskPromise<SppListenResult> evalPromise) {
            auto sppOptions = QNapi::makeObject(
                jsState.env(),
                {
                    {"uuid", uuid.toStdString()},
                    {"secure", secure},
                    {"type", jsState.mapOhosEnumToJs(SppType::SPP_RFCOMM)},
                });

            auto thenCatchPromises = std::move(evalPromise).makeThenCatchBranches(Q_FUNC_INFO);

            try {
                jsState.eval(
                    "@ohos.bluetooth.socket.sppListen(*)",
                    {
                        serviceName.toStdString(),
                        sppOptions,
                        [thenPromise = std::move(thenCatchPromises.first)](
                            const QNapi::CallbackInfo &callbackInfo) {
                            QNapi::Value error;
                            QNapi::Value data;
                            callbackInfo.getLeadingArgs(Q_FUNC_INFO, error, data);
                            int socketId = data.IsNumber()
                                ? QNapi::checkedCast<QNapi::Number>(data).Int32Value()
                                : invalidSocketDescriptor;

                            if (socketId == invalidSocketDescriptor) {
                                thenPromise(
                                    SppListenResult{
                                        socketId, tryGetKnownErrorCode(error, Q_FUNC_INFO)});
                                return;
                            }

                            thenPromise(SppListenResult{socketId, {}});
                        }
                    });
            } catch (const Napi::Error &error) {
                thenCatchPromises.second(
                    SppListenResult{
                        invalidSocketDescriptor,
                        tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO)});
            }
        });

    if (listenResult.optErrorCode) {
        qCCritical(
            QT_BT_OHOS, "%s: sppListen() failed with error code: %u", Q_FUNC_INFO,
            qToUnderlying(*listenResult.optErrorCode));
        reportError(*listenResult.optErrorCode);
        return false;
    }

    if (listenResult.serverSocketId == invalidSocketDescriptor) {
        qCCritical(QT_BT_OHOS, "%s: sppListen() reported no server socket", Q_FUNC_INFO);
        reportError(QOhosBluetoothErrorCode::OperationFailed);
        return false;
    }

    m_serverSocketHandle = makeSppServerSocketHandle(listenResult.serverSocketId);
    m_registeredServiceName = serviceName;
    m_optRegisteredServiceId = ++m_registeredServicesCounter;

    return true;
}

bool QOhosBluetoothServerProxy::accept()
{
    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO)) {
        reportError(QOhosBluetoothErrorCode::PermissionDenied);
        return false;
    }

    if (!m_serverSocketHandle) {
        qCCritical(
            QT_BT_OHOS, "%s: Cannot accept clients connections. There is no server socket created",
            Q_FUNC_INFO);
        return false;
    }

    auto optAcceptErrorCode = QOhosJsThreadGateway::eval(
        [&](QOhosJsState &jsState) -> std::optional<QOhosBluetoothErrorCode> {
            try {
                jsState.eval(
                    "@ohos.bluetooth.socket.sppAccept(*)",
                    {
                        *m_serverSocketHandle,
                        [selfWeakRef = weak_from_this(), armedServerSocketWatch = QtOhos::makeWeakPtr(m_serverSocketHandle)](
                            const QNapi::CallbackInfo &callbackInfo) {
                            QNapi::Value error;
                            QNapi::Value data;
                            callbackInfo.getLeadingArgs(Q_FUNC_INFO, error, data);
                            int clientSocketId = data.IsNumber()
                                ? QNapi::checkedCast<QNapi::Number>(data).Int32Value()
                                : invalidSocketDescriptor;

                            if (clientSocketId == invalidSocketDescriptor) {
                                const bool abortedByOwnServerSocketClose =
                                    armedServerSocketWatch.expired();
                                if (abortedByOwnServerSocketClose) {
                                    qOhosPrintfInfo(
                                        "%s: sppAccept() was aborted by closing the server socket",
                                        Q_FUNC_INFO);
                                    return;
                                }

                                const auto errorCode =
                                    tryGetKnownErrorCode(error, Q_FUNC_INFO)
                                        .value_or(QOhosBluetoothErrorCode::OperationFailed);
                                qOhosPrintfError(
                                    "%s: sppAccept() failed with error code: %u", Q_FUNC_INFO,
                                    qToUnderlying(errorCode));
                                QtOhos::invokeInQtThread(
                                    [selfWeakRef, errorCode]() {
                                        if (const auto self = selfWeakRef.lock())
                                            self->onAcceptFailed(errorCode);
                                    });
                                return;
                            }

                            auto clientSocketHandle = makeSppClientSocketHandle(clientSocketId);

                            QtOhos::invokeInQtThread(
                                [selfWeakRef, clientSocketHandle]() {
                                    if (const auto self = selfWeakRef.lock())
                                        self->onClientAccepted(clientSocketHandle);
                                });
                        }
                    });
                return std::nullopt;
            } catch (const Napi::Error &error) {
                return tryGetKnownErrorCode(error.Value(), Q_FUNC_INFO);
            }
        });

    if (optAcceptErrorCode) {
        qCCritical(
            QT_BT_OHOS, "%s: sppAccept() failed with error code: %u", Q_FUNC_INFO,
            qToUnderlying(*optAcceptErrorCode));
        reportError(*optAcceptErrorCode);
        return false;
    }

    return true;
}

void QOhosBluetoothServerProxy::onClientAccepted(std::shared_ptr<int> clientSocketHandle)
{
    if (!isServiceRegistered()) {
        qCDebug(
            QT_BT_OHOS, "%s: service is not registered anymore. Dropping the accepted client ...",
            Q_FUNC_INFO);
        return;
    }

    if (!QOhosBluetoothSocketProxy::registerPendingSocketHandle(clientSocketHandle)) {
        reportError(QOhosBluetoothErrorCode::OperationFailed);
        scheduleAcceptNextConnection();
        return;
    }

    Q_EMIT clientAccepted(*clientSocketHandle);

    scheduleAcceptNextConnection();
}

void QOhosBluetoothServerProxy::onAcceptFailed(QOhosBluetoothErrorCode errorCode)
{
    if (!isServiceRegistered())
        return;

    reportError(errorCode);
    close();
}

void QOhosBluetoothServerProxy::scheduleAcceptNextConnection()
{
    QMetaObject::invokeMethod(
        this,
        [selfWeakRef = weak_from_this()]() {
            if (const auto self = selfWeakRef.lock())
                self->acceptNextConnection();
        },
        Qt::QueuedConnection);
}

void QOhosBluetoothServerProxy::acceptNextConnection()
{
    if (!isServiceRegistered())
        return;

    if (!checkAccessBluetoothPermissionGranted(Q_FUNC_INFO)) {
        reportError(QOhosBluetoothErrorCode::PermissionDenied);
        close();
        return;
    }

    const auto optBluetoothState = QOhosBluetoothAccessProxy::instance()->tryGetBluetoothState();
    if (!optBluetoothState) {
        qCWarning(
            QT_BT_OHOS,
            "%s: the bluetooth state is unknown. Cannot accept next clients connections",
            Q_FUNC_INFO);
        reportError(QOhosBluetoothErrorCode::OperationFailed);
        close();
        return;
    }

    if (*optBluetoothState != QOhosBluetoothAccessProxy::BluetoothState::STATE_ON) {
        qCWarning(
            QT_BT_OHOS, "%s: bluetooth is turned off. Cannot accept next clients connections",
            Q_FUNC_INFO);
        reportError(QOhosBluetoothErrorCode::BluetoothDisabled);
        close();
        return;
    }

    if (!accept())
        close();
}

void QOhosBluetoothServerProxy::close()
{
    if (!m_serverSocketHandle)
        return;

    m_serverSocketHandle.reset();
    m_registeredServiceName.reset();
    m_optRegisteredServiceId.reset();

    reportAcceptingStopped();
}

bool QOhosBluetoothServerProxy::isServiceRegistered() const
{
    return bool(m_registeredServiceName);
}

std::optional<QString> QOhosBluetoothServerProxy::tryGetRegisteredServiceName() const
{
    return m_registeredServiceName;
}

std::optional<std::uint64_t> QOhosBluetoothServerProxy::tryGetRegisteredServiceId() const
{
    return m_optRegisteredServiceId;
}

void QOhosBluetoothServerProxy::reportError(QOhosBluetoothErrorCode errorCode)
{
    QMetaObject::invokeMethod(
        this,
        [this, errorCode]() {
            Q_EMIT errorOccurred(errorCode);
        },
        Qt::QueuedConnection);
}

void QOhosBluetoothServerProxy::reportAcceptingStopped()
{
    QMetaObject::invokeMethod(
        this, &QOhosBluetoothServerProxy::acceptingStopped, Qt::QueuedConnection);
}

}

QT_END_NAMESPACE

#include "moc_qohosbluetoothserver_p.cpp"

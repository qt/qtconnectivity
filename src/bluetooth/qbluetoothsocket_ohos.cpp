// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothsocket_ohos_p.h"

#include <QtBluetooth/qbluetoothdeviceinfo.h>
#include <QtBluetooth/qbluetoothsocket.h>
#include <QtBluetooth/private/qohosbluetoothlocaldevice_p.h>
#include <QtBluetooth/private/qohosbluetoothsocket_p.h>

#include <QtCore/q26numeric.h>
#include <QtCore/qloggingcategory.h>

#include <cstring>

QT_BEGIN_NAMESPACE

using QtOhosBluetooth::QOhosBluetoothErrorCode;

Q_DECLARE_LOGGING_CATEGORY(QT_BT_OHOS)

namespace {

struct SocketErrorReport
{
    QBluetoothSocket::SocketError error = QBluetoothSocket::SocketError::UnknownSocketError;
    QString message;
};

SocketErrorReport makeSocketErrorReport(QOhosBluetoothErrorCode errorCode)
{
    switch (errorCode) {
    case QOhosBluetoothErrorCode::PermissionDenied:
        return {
            .error = QBluetoothSocket::SocketError::MissingPermissionsError,
            .message = QBluetoothSocket::tr("Missing permissions for socket operation")};
    case QOhosBluetoothErrorCode::InvalidParameter:
        return {
            .error = QBluetoothSocket::SocketError::OperationError,
            .message = QBluetoothSocket::tr("Socket operation rejected by the system")};
    case QOhosBluetoothErrorCode::CapabilityNotSupported:
    case QOhosBluetoothErrorCode::ProfileNotSupported:
        return {
            .error = QBluetoothSocket::SocketError::UnsupportedProtocolError,
            .message = QBluetoothSocket::tr("Socket type not supported")};
    case QOhosBluetoothErrorCode::InputOutput:
        return {
            .error = QBluetoothSocket::SocketError::UnknownSocketError,
            .message = QBluetoothSocket::tr("I/O error on socket")};
    case QOhosBluetoothErrorCode::ServiceStopped:
    case QOhosBluetoothErrorCode::BluetoothDisabled:
        return {
            .error = QBluetoothSocket::SocketError::NetworkError,
            .message = QBluetoothSocket::tr("Bluetooth is not available")};
    case QOhosBluetoothErrorCode::UserDidNotRespond:
    case QOhosBluetoothErrorCode::UserRefused:
    case QOhosBluetoothErrorCode::OperationFailed:
        break;
    }

    return {
        .error = QBluetoothSocket::SocketError::OperationError,
        .message = QBluetoothSocket::tr("Operation failed on socket")};
}

}

QBluetoothSocketPrivateOhos::QBluetoothSocketPrivateOhos()
{
    secFlags = QBluetooth::Security::Secure;

    QObject::connect(
        &m_bluetoothSocketProxy, &QtOhosBluetooth::QOhosBluetoothSocketProxy::connected,
        this,
        [this](int socketDescriptor) {
            if (state != QBluetoothSocket::SocketState::ConnectingState)
                return;

            socket = socketDescriptor;
            q_ptr->setOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered);
            q_ptr->setSocketState(QBluetoothSocket::SocketState::ConnectedState);

            if (!rxBuffer.isEmpty())
                Q_EMIT q_ptr->readyRead();
        });

    QObject::connect(
        &m_bluetoothSocketProxy, &QtOhosBluetooth::QOhosBluetoothSocketProxy::disconnected,
        this,
        [this]() {
            onProxyDisconnected();
        });

    QObject::connect(
        &m_bluetoothSocketProxy, &QtOhosBluetooth::QOhosBluetoothSocketProxy::dataReceived,
        this,
        [this](const QByteArray &receivedData) {
            if (state != QBluetoothSocket::SocketState::ConnectingState
                && state != QBluetoothSocket::SocketState::ConnectedState) {
                return;
            }

            char *target = rxBuffer.reserve(receivedData.size());
            std::memcpy(target, receivedData.constData(), receivedData.size());

            if (state == QBluetoothSocket::SocketState::ConnectedState)
                Q_EMIT q_ptr->readyRead();
        });

    QObject::connect(
        &m_bluetoothSocketProxy, &QtOhosBluetooth::QOhosBluetoothSocketProxy::errorOccurred,
        this,
        [this](QOhosBluetoothErrorCode errorCode) {
            onProxyError(errorCode);
        });
}

QBluetoothSocketPrivateOhos::~QBluetoothSocketPrivateOhos()
{
    abort();
}

void QBluetoothSocketPrivateOhos::onProxyError(QOhosBluetoothErrorCode errorCode)
{
    const auto report = makeSocketErrorReport(errorCode);

    errorString = report.message;
    q_ptr->setSocketError(report.error);

    if (state == QBluetoothSocket::SocketState::ConnectingState
        || state == QBluetoothSocket::SocketState::ClosingState) {
        socket = QtOhosBluetooth::invalidSocketDescriptor;
        q_ptr->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
    }
}

void QBluetoothSocketPrivateOhos::onProxyDisconnected()
{
    socket = QtOhosBluetooth::invalidSocketDescriptor;
    q_ptr->setOpenMode(QIODevice::NotOpen);
    q_ptr->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
    Q_EMIT q_ptr->readChannelFinished();
}

bool QBluetoothSocketPrivateOhos::ensureNativeSocket(QBluetoothServiceInfo::Protocol type)
{
    socketType = type;

    return type == QBluetoothServiceInfo::RfcommProtocol;
}

void QBluetoothSocketPrivateOhos::connectToServiceHelper(
    const QBluetoothAddress &address, const QBluetoothUuid &uuid, QIODevice::OpenMode openMode)
{
    if (openMode != QIODevice::ReadWrite)
        qCWarning(QT_BT_OHOS, "%s: Only read-write socket mode supported.", Q_FUNC_INFO);

    if (state != QBluetoothSocket::SocketState::UnconnectedState) {
        qCWarning(QT_BT_OHOS, "%s: Called on busy socket", Q_FUNC_INFO);
        errorString = QBluetoothSocket::tr("Trying to connect while connection is established");
        q_ptr->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return;
    }

    rxBuffer.clear();
    q_ptr->setSocketState(QBluetoothSocket::SocketState::ConnectingState);

    const auto secure = (secFlags != QBluetooth::Security::NoSecurity);

    const auto connectRequested = m_bluetoothSocketProxy.connect(
        address.toString(), uuid.toString(QUuid::WithoutBraces), secure);

    if (!connectRequested)
        q_ptr->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
}

void QBluetoothSocketPrivateOhos::connectToService(
    const QBluetoothServiceInfo &serviceInfo, QIODevice::OpenMode openMode)
{
    const auto protocol = serviceInfo.socketProtocol() != QBluetoothServiceInfo::UnknownProtocol
        ? serviceInfo.socketProtocol()
        : q_ptr->socketType();

    if (!ensureNativeSocket(protocol)) {
        qCWarning(QT_BT_OHOS, "%s: Only RfcommProtocol supported", Q_FUNC_INFO);
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q_ptr->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
        return;
    }

    connectToServiceHelper(serviceInfo.device().address(), serviceInfo.serviceUuid(), openMode);
}

void QBluetoothSocketPrivateOhos::connectToService(
    const QBluetoothAddress &address, const QBluetoothUuid &uuid, QIODevice::OpenMode openMode)
{
    if (!ensureNativeSocket(q_ptr->socketType())) {
        qCWarning(QT_BT_OHOS, "%s: Only RfcommProtocol supported", Q_FUNC_INFO);
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q_ptr->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
        return;
    }

    connectToServiceHelper(address, uuid, openMode);
}

void QBluetoothSocketPrivateOhos::connectToService(
        const QBluetoothAddress &, quint16, QIODevice::OpenMode)
{
    qCWarning(
        QT_BT_OHOS, "%s: HarmonyOS connects to a service by its UUID, not by an RFCOMM port",
        Q_FUNC_INFO);
    errorString = QBluetoothSocket::tr("Connecting to a port is not supported");
    q_ptr->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
}

void QBluetoothSocketPrivateOhos::abort()
{
    close();
}

QString QBluetoothSocketPrivateOhos::localName() const
{
    const auto optLocalDeviceName =
        QtOhosBluetooth::QOhosBluetoothLocalDeviceProxy::instance()->tryGetLocalDeviceName();
    return optLocalDeviceName
        ? *optLocalDeviceName
        : QString();
}

QBluetoothAddress QBluetoothSocketPrivateOhos::localAddress() const
{
    return {};
}

quint16 QBluetoothSocketPrivateOhos::localPort() const
{
    return 0;
}

QString QBluetoothSocketPrivateOhos::peerName() const
{
    const auto optName = m_bluetoothSocketProxy.tryGetPeerDeviceName();
    return optName
        ? *optName
        : QString();
}

QBluetoothAddress QBluetoothSocketPrivateOhos::peerAddress() const
{
    const auto optAddress = m_bluetoothSocketProxy.tryGetPeerDeviceAddress();
    return optAddress
        ? QBluetoothAddress(*optAddress)
        : QBluetoothAddress();
}

quint16 QBluetoothSocketPrivateOhos::peerPort() const
{
    return 0;
}

qint64 QBluetoothSocketPrivateOhos::writeData(const char *data, qint64 maxSize)
{
    if (state != QBluetoothSocket::SocketState::ConnectedState) {
        errorString = QBluetoothSocket::tr("No SPP connection between devices. Cannot write.");
        q_ptr->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return -1;
    }

    if (!m_bluetoothSocketProxy.write(data, maxSize))
        return -1;

    QMetaObject::invokeMethod(
        q_ptr,
        [this, maxSize]() {
            Q_EMIT q_ptr->bytesWritten(maxSize);
        },
        Qt::QueuedConnection);

    return maxSize;
}

qint64 QBluetoothSocketPrivateOhos::readData(char *data, qint64 maxSize)
{
    if (!rxBuffer.isEmpty())
        return rxBuffer.read(data, q26::saturate_cast<qsizetype>(maxSize));

    if (state != QBluetoothSocket::SocketState::ConnectedState) {
        errorString = QBluetoothSocket::tr("Cannot read while not connected");
        q_ptr->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return -1;
    }

    return 0;
}

void QBluetoothSocketPrivateOhos::close()
{
    m_bluetoothSocketProxy.close();
}

bool QBluetoothSocketPrivateOhos::setSocketDescriptor(
    int socketDescriptor, QBluetoothServiceInfo::Protocol type,
    QBluetoothSocket::SocketState socketState, QBluetoothSocket::OpenMode openMode)
{
    if (openMode != QIODevice::ReadWrite)
        qCWarning(QT_BT_OHOS, "%s: Only read-write socket mode supported.", Q_FUNC_INFO);

    if (!ensureNativeSocket(type)) {
        qCWarning(QT_BT_OHOS, "%s: Only RfcommProtocol supported", Q_FUNC_INFO);
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q_ptr->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
        return false;
    }

    if (socketState != QBluetoothSocket::SocketState::ConnectedState) {
        qCWarning(
            QT_BT_OHOS, "%s: a socket accepted by the server can only be taken over as connected",
            Q_FUNC_INFO);
        errorString = QBluetoothSocket::tr("Taking over the socket failed");
        q_ptr->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return false;
    }

    if (state != QBluetoothSocket::SocketState::UnconnectedState
        || socket != QtOhosBluetooth::invalidSocketDescriptor) {
        qCWarning(QT_BT_OHOS, "%s: Called on busy socket", Q_FUNC_INFO);
        errorString = QBluetoothSocket::tr("Trying to connect while connection is in progress");
        q_ptr->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return false;
    }

    auto socketHandle =
        QtOhosBluetooth::QOhosBluetoothSocketProxy::takePendingSocketHandle(socketDescriptor);
    if (!socketHandle) {
        qCWarning(
            QT_BT_OHOS, "%s: no socket accepted by the server for descriptor %d", Q_FUNC_INFO,
            socketDescriptor);
        errorString = QBluetoothSocket::tr("Taking over the socket failed");
        q_ptr->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
        q_ptr->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        return false;
    }

    rxBuffer.clear();

    if (!m_bluetoothSocketProxy.takeOverSocket(std::move(socketHandle))) {
        errorString = QBluetoothSocket::tr("Obtaining data channel for service failed");
        q_ptr->setSocketError(QBluetoothSocket::SocketError::NetworkError);
        q_ptr->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        return false;
    }

    socket = socketDescriptor;
    q_ptr->setOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered);
    q_ptr->setSocketState(socketState);

    return true;
}

qint64 QBluetoothSocketPrivateOhos::bytesAvailable() const
{
    return rxBuffer.size();
}

bool QBluetoothSocketPrivateOhos::canReadLine() const
{
    return rxBuffer.canReadLine();
}

qint64 QBluetoothSocketPrivateOhos::bytesToWrite() const
{
    return 0;
}

QT_END_NAMESPACE

// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHSOCKET_OHOS_P_H
#define QBLUETOOTHSOCKET_OHOS_P_H

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
#include <QtBluetooth/qbluetoothsocket.h>
#include <QtBluetooth/private/qbluetoothsocketbase_p.h>
#include <QtBluetooth/private/qohosbluetoothsocket_p.h>

#include <QtCore/qtconfigmacros.h>
#include <QtCore/qtypes.h>

QT_BEGIN_NAMESPACE

class QBluetoothSocketPrivateOhos final : public QBluetoothSocketBasePrivate
{
public:
    QBluetoothSocketPrivateOhos();
    ~QBluetoothSocketPrivateOhos() override;

    void connectToServiceHelper(
        const QBluetoothAddress &address, const QBluetoothUuid &uuid,
        QIODevice::OpenMode openMode) override;

    void connectToService(const QBluetoothServiceInfo &service, QIODevice::OpenMode openMode) override;
    void connectToService(
        const QBluetoothAddress &address, const QBluetoothUuid &uuid,
        QIODevice::OpenMode openMode) override;
    void connectToService(
        const QBluetoothAddress &address, quint16 port, QIODevice::OpenMode openMode) override;

    bool ensureNativeSocket(QBluetoothServiceInfo::Protocol type) override;

    QString localName() const override;
    QBluetoothAddress localAddress() const override;
    quint16 localPort() const override;

    QString peerName() const override;
    QBluetoothAddress peerAddress() const override;
    quint16 peerPort() const override;

    void abort() override;
    void close() override;

    qint64 writeData(const char *data, qint64 maxSize) override;
    qint64 readData(char *data, qint64 maxSize) override;

    bool setSocketDescriptor(
        int socketDescriptor, QBluetoothServiceInfo::Protocol type,
        QBluetoothSocket::SocketState socketState = QBluetoothSocket::SocketState::ConnectedState,
        QBluetoothSocket::OpenMode openMode = QBluetoothSocket::ReadWrite) override;

    qint64 bytesAvailable() const override;
    bool canReadLine() const override;
    qint64 bytesToWrite() const override;

private:
    void onProxyError(QtOhosBluetooth::QOhosBluetoothErrorCode errorCode);
    void onProxyDisconnected();

    QtOhosBluetooth::QOhosBluetoothSocketProxy m_bluetoothSocketProxy;
};

QT_END_NAMESPACE

#endif // QBLUETOOTHSOCKET_OHOS_P_H

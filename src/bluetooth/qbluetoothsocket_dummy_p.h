/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef QBLUETOOTHSOCKET_DUMMY_H
#define QBLUETOOTHSOCKET_DUMMY_H

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

#include "qbluetoothsocket.h"
#include "qbluetoothsocketbase_p.h"
#include <QtGlobal>

QT_BEGIN_NAMESPACE

class QBluetoothSocketPrivateDummy final : public QBluetoothSocketBasePrivate
{
    Q_OBJECT
    friend class QBluetoothServerPrivate;

public:
    QBluetoothSocketPrivateDummy();
    ~QBluetoothSocketPrivateDummy() override;

    void connectToServiceHelper(const QBluetoothAddress &address,
                          quint16 port,
                          QIODevice::OpenMode openMode) override;

    void connectToService(const QBluetoothServiceInfo &service,
                          QIODevice::OpenMode openMode) override;
    void connectToService(const QBluetoothAddress &address, const QBluetoothUuid &uuid,
                          QIODevice::OpenMode openMode) override;
    void connectToService(const QBluetoothAddress &address, quint16 port,
                          QIODevice::OpenMode openMode) override;

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

    bool setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType,
                             QBluetoothSocket::SocketState socketState = QBluetoothSocket::ConnectedState,
                             QBluetoothSocket::OpenMode openMode = QBluetoothSocket::ReadWrite) override;

    qint64 bytesAvailable() const override;
    bool canReadLine() const override;
    qint64 bytesToWrite() const override;
};

QT_END_NAMESPACE

#endif // QBLUETOOTHSOCKET_DUMMY_H


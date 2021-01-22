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

#ifndef QBLUETOOTHSOCKET_OSX_P_H
#define QBLUETOOTHSOCKET_OSX_P_H

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

#ifdef QT_OSX_BLUETOOTH

#include "qbluetoothsocketbase_p.h"
#include "qbluetoothserviceinfo.h"
#include "osx/btdelegates_p.h"
#include "qbluetoothsocket.h"
#include "osx/btraii_p.h"

#ifndef QPRIVATELINEARBUFFER_BUFFERSIZE
#define QPRIVATELINEARBUFFER_BUFFERSIZE Q_INT64_C(16384)
#endif
// The order is important: bytearray before buffer:
#include <QtCore/qbytearray.h>
#include "qprivatelinearbuffer_p.h"

#include <QtCore/qscopedpointer.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

class QBluetoothAddress;

class QBluetoothSocketPrivate : public QBluetoothSocketBasePrivate, public DarwinBluetooth::ChannelDelegate
{
    friend class QBluetoothSocket;
    friend class QBluetoothServer;

public:
    QBluetoothSocketPrivate();
    ~QBluetoothSocketPrivate();

    //
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

    qint64 bytesAvailable() const override;
    bool canReadLine() const override;
    qint64 bytesToWrite() const override;

    bool setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType,
                             QBluetoothSocket::SocketState socketState,
                             QBluetoothSocket::OpenMode openMode) override;

    void connectToServiceHelper(const QBluetoothAddress &address, quint16 port,
                                QIODevice::OpenMode openMode) override;
    void connectToService(const QBluetoothServiceInfo &service,
                          QIODevice::OpenMode openMode) override;
    void connectToService(const QBluetoothAddress &address, const QBluetoothUuid &uuid,
                          QIODevice::OpenMode openMode) override;

    void connectToService(const QBluetoothAddress &address, quint16 port,
                          QIODevice::OpenMode openMode) override;

    void _q_writeNotify();

private:
    // Create a socket from an external source (without connectToService).
    bool setRFCOMChannel(void *channel);
    bool setL2CAPChannel(void *channel);

    // L2CAP and RFCOMM delegate
    void setChannelError(IOReturn errorCode) override;
    void channelOpenComplete() override;
    void channelClosed() override;
    void readChannelData(void *data, std::size_t size) override;
    void writeComplete() override;

    QVector<char> writeChunk;

    using L2CAPChannel = DarwinBluetooth::ScopedPointer;
    L2CAPChannel l2capChannel;

    using  RFCOMMChannel = L2CAPChannel;
    RFCOMMChannel rfcommChannel;
    // A trick to deal with channel open too fast (synchronously).
    bool isConnecting = false;
};

QT_END_NAMESPACE

#endif

#endif

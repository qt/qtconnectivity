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

#ifndef QBLUETOOTHTRANSFERREPLY_OSX_P_H
#define QBLUETOOTHTRANSFERREPLY_OSX_P_H

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

#include "qbluetoothservicediscoveryagent.h"
#include "qbluetoothtransferreply.h"

#include <QtCore/qscopedpointer.h>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QBluetoothTransferReplyOSXPrivate;
class QBluetoothServiceInfo;
class QBluetoothAddress;
class QIODevice;

class Q_BLUETOOTH_EXPORT QBluetoothTransferReplyOSX : public QBluetoothTransferReply
{
    Q_OBJECT

public:
    QBluetoothTransferReplyOSX(QIODevice *input, const QBluetoothTransferRequest &request,
                               QBluetoothTransferManager *parent);
    ~QBluetoothTransferReplyOSX();

    TransferError error() const override;
    QString errorString() const override;

    bool isFinished() const override;
    bool isRunning() const override;

Q_SIGNALS:
    void error(QBluetoothTransferReply::TransferError lastError);

public slots:
    bool abort();

private slots:
    bool start();

    void serviceDiscoveryFinished();
    void serviceDiscoveryError(QBluetoothServiceDiscoveryAgent::Error error);

private:
    // OS X private data (not to be seen by moc).
    QScopedPointer<QBluetoothTransferReplyOSXPrivate> osx_d_ptr;
};

QT_END_NAMESPACE

#endif

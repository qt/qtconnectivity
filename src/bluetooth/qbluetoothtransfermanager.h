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

#ifndef QBLUETOOTHTRANSFERMANAGER_H
#define QBLUETOOTHTRANSFERMANAGER_H

#include <QtBluetooth/qtbluetoothglobal.h>
#include <QtBluetooth/QBluetoothAddress>

#include <QtCore/QObject>

QT_FORWARD_DECLARE_CLASS(QIODevice)

QT_BEGIN_NAMESPACE

class QBluetoothTransferReply;
class QBluetoothTransferRequest;
class QBluetoothTranferManagerPrivate;

class Q_BLUETOOTH_EXPORT QBluetoothTransferManager : public QObject
{
    Q_OBJECT

public:
    explicit QBluetoothTransferManager(QObject *parent = nullptr);
    ~QBluetoothTransferManager();

    QBluetoothTransferReply *put(const QBluetoothTransferRequest &request, QIODevice *data);

Q_SIGNALS:
    void finished(QBluetoothTransferReply *reply);

};

QT_END_NAMESPACE

#endif // QBLUETOOTHTRANSFERMANAGER_H

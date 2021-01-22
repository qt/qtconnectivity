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

#ifndef QBLUETOOTHTRANSFERREQUEST_H
#define QBLUETOOTHTRANSFERREQUEST_H

#include <QtBluetooth/qtbluetoothglobal.h>
#include <QtBluetooth/QBluetoothAddress>

#include <QtCore/QtGlobal>
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

class QBluetoothAddress;
class QBluetoothTransferRequestPrivate;

class Q_BLUETOOTH_EXPORT QBluetoothTransferRequest
{
public:
    enum Attribute {
        DescriptionAttribute,
        TimeAttribute,
        TypeAttribute,
        LengthAttribute,
        NameAttribute
    };

    explicit QBluetoothTransferRequest(const QBluetoothAddress &address = QBluetoothAddress());
    QBluetoothTransferRequest(const QBluetoothTransferRequest &other);
    ~QBluetoothTransferRequest();

    QVariant attribute(Attribute code, const QVariant &defaultValue = QVariant()) const;
    void setAttribute(Attribute code, const QVariant &value);

    QBluetoothAddress address() const;

    bool operator!=(const QBluetoothTransferRequest &other) const;
    QBluetoothTransferRequest &operator=(const QBluetoothTransferRequest &other);
    bool operator==(const QBluetoothTransferRequest &other) const;

protected:
    QBluetoothTransferRequestPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(QBluetoothTransferRequest)

};

QT_END_NAMESPACE

#endif // QBLUETOOTHTRANSFERREQUEST_H

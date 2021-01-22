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

#ifndef QBLUETOOTHHOSTINFO_H
#define QBLUETOOTHHOSTINFO_H

#include <QtBluetooth/qtbluetoothglobal.h>
#include <QtBluetooth/QBluetoothAddress>

QT_BEGIN_NAMESPACE

class QBluetoothHostInfoPrivate;
class Q_BLUETOOTH_EXPORT QBluetoothHostInfo
{
public:
    QBluetoothHostInfo();
    QBluetoothHostInfo(const QBluetoothHostInfo &other);
    ~QBluetoothHostInfo();

    QBluetoothHostInfo &operator=(const QBluetoothHostInfo &other);

    bool operator==(const QBluetoothHostInfo &other) const;
    bool operator!=(const QBluetoothHostInfo &other) const;

    QBluetoothAddress address() const;
    void setAddress(const QBluetoothAddress &address);

    QString name() const;
    void setName(const QString &name);

private:
    Q_DECLARE_PRIVATE(QBluetoothHostInfo)
    QBluetoothHostInfoPrivate *d_ptr;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QBluetoothHostInfo)

#endif

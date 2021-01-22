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

#ifndef QBLUETOOTHADDRESS_H
#define QBLUETOOTHADDRESS_H

#include <QtBluetooth/qtbluetoothglobal.h>

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QMetaType>

QT_BEGIN_NAMESPACE

class QBluetoothAddressPrivate;

class Q_BLUETOOTH_EXPORT QBluetoothAddress
{
public:
    QBluetoothAddress();
    explicit QBluetoothAddress(quint64 address);
    explicit QBluetoothAddress(const QString &address);
    QBluetoothAddress(const QBluetoothAddress &other);
    ~QBluetoothAddress();

    QBluetoothAddress &operator=(const QBluetoothAddress &other);

    bool isNull() const;

    void clear();

    bool operator<(const QBluetoothAddress &other) const;
    bool operator==(const QBluetoothAddress &other) const;
    inline bool operator!=(const QBluetoothAddress &other) const
    {
        return !operator==(other);
    }

    quint64 toUInt64() const;
    QString toString() const;

private:
    Q_DECLARE_PRIVATE(QBluetoothAddress)
    QBluetoothAddressPrivate *d_ptr;
};

#ifndef QT_NO_DEBUG_STREAM
Q_BLUETOOTH_EXPORT QDebug operator<<(QDebug, const QBluetoothAddress &address);
#endif

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QBluetoothAddress)

#endif

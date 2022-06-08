// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHADDRESS_H
#define QBLUETOOTHADDRESS_H

#include <QtBluetooth/qtbluetoothglobal.h>

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QMetaType>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

class Q_BLUETOOTH_EXPORT QBluetoothAddress
{
public:
    constexpr QBluetoothAddress() noexcept {};
    constexpr explicit QBluetoothAddress(quint64 address) noexcept : m_address(address) {};
    explicit QBluetoothAddress(const QString &address);
    QBluetoothAddress(const QBluetoothAddress &other);
    ~QBluetoothAddress();

    QBluetoothAddress &operator=(const QBluetoothAddress &other);

    bool isNull() const;

    void clear();

    friend bool operator<(const QBluetoothAddress &a, const QBluetoothAddress &b)
    {
        return a.m_address < b.m_address;
    }
    friend bool operator==(const QBluetoothAddress &a, const QBluetoothAddress &b)
    {
        return a.m_address == b.m_address;
    }
    inline friend bool operator!=(const QBluetoothAddress &a, const QBluetoothAddress &b)
    {
        return a.m_address != b.m_address;
    }

    quint64 toUInt64() const;
    QString toString() const;

private:
    quint64 m_address = { 0 };
#ifndef QT_NO_DEBUG_STREAM
    friend QDebug operator<<(QDebug d, const QBluetoothAddress &a)
    {
        return streamingOperator(d, a);
    }
    static QDebug streamingOperator(QDebug, const QBluetoothAddress &address);
#endif
};

QT_END_NAMESPACE

QT_DECL_METATYPE_EXTERN(QBluetoothAddress, Q_BLUETOOTH_EXPORT)

#endif

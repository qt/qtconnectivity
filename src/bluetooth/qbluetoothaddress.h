// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHADDRESS_H
#define QBLUETOOTHADDRESS_H

#include <QtBluetooth/qtbluetoothglobal.h>

#include <QtCore/QByteArray>
#include <QtCore/qhashfunctions.h>
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
    QT_BLUETOOTH_INLINE_SINCE(6, 6)
    QBluetoothAddress(const QBluetoothAddress &other) noexcept;
    QT_BLUETOOTH_INLINE_SINCE(6, 6)
    ~QBluetoothAddress();

    QT_BLUETOOTH_INLINE_SINCE(6, 6)
    QBluetoothAddress &operator=(const QBluetoothAddress &other) noexcept;
    QBluetoothAddress(QBluetoothAddress &&) noexcept = default;
    QBluetoothAddress &operator=(QBluetoothAddress &&) noexcept = default;

    QT_BLUETOOTH_INLINE_SINCE(6, 6)
    bool isNull() const noexcept;

    QT_BLUETOOTH_INLINE_SINCE(6, 6)
    void clear() noexcept;

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

    QT_BLUETOOTH_INLINE_SINCE(6, 6)
    quint64 toUInt64() const noexcept;
    QString toString() const;

private:
    quint64 m_address = { 0 };
    friend QT7_ONLY(constexpr) size_t qHash(const QBluetoothAddress &key, size_t seed = 0) noexcept
    { return qHash(key.m_address, seed); }
#ifndef QT_NO_DEBUG_STREAM
    friend QDebug operator<<(QDebug d, const QBluetoothAddress &a)
    {
        return streamingOperator(d, a);
    }
    static QDebug streamingOperator(QDebug, const QBluetoothAddress &address);
#endif
};

#if QT_BLUETOOTH_INLINE_IMPL_SINCE(6, 6)
QBluetoothAddress::QBluetoothAddress(const QBluetoothAddress &) noexcept = default;
QBluetoothAddress &QBluetoothAddress::operator=(const QBluetoothAddress &) noexcept = default;
QBluetoothAddress::~QBluetoothAddress() = default;

void QBluetoothAddress::clear() noexcept
{
    m_address = 0;
}

bool QBluetoothAddress::isNull() const noexcept
{
    return m_address == 0;
}

quint64 QBluetoothAddress::toUInt64() const noexcept
{
    return m_address;
}
#endif // QT_BLUETOOTH_INLINE_IMPL_SINCE(6, 6)

QT_END_NAMESPACE

QT_DECL_METATYPE_EXTERN(QBluetoothAddress, Q_BLUETOOTH_EXPORT)

#endif

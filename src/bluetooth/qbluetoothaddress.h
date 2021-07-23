/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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

Q_DECLARE_METATYPE(QBluetoothAddress)

#endif

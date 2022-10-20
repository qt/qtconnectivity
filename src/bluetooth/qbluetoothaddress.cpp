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

#include "qbluetoothaddress.h"

#ifndef QT_NO_DEBUG_STREAM
#include <QDebug>
#endif

QT_BEGIN_NAMESPACE

/*!
    \class QBluetoothAddress
    \inmodule QtBluetooth
    \brief The QBluetoothAddress class assigns an address to the Bluetooth
    device.
    \since 5.2

    This class holds a Bluetooth address in a platform- and protocol-independent manner.
*/

void registerQBluetoothAddress()
{
    qRegisterMetaType<QBluetoothAddress>();
}
Q_CONSTRUCTOR_FUNCTION(registerQBluetoothAddress)

/*!
    \fn QBluetoothAddress::QBluetoothAddress()

    Constructs an null Bluetooth Address.
*/

/*!
    \fn QBluetoothAddress::QBluetoothAddress(quint64 address)

    Constructs a new Bluetooth address and assigns \a address to it.
*/

/*!
    Constructs a new Bluetooth address and assigns \a address to it.

    The format of \a address can be either XX:XX:XX:XX:XX:XX or XXXXXXXXXXXX,
    where X is a hexadecimal digit.  Case is not important.
*/
QBluetoothAddress::QBluetoothAddress(const QString &address)
{
    QString a = address;

    if (a.length() == 17)
        a.remove(QLatin1Char(':'));

    if (a.length() == 12) {
        bool ok;
        m_address = a.toULongLong(&ok, 16);
        if (!ok)
            clear();
    } else {
        m_address = 0;
    }
}

/*!
    Constructs a new Bluetooth address which is a copy of \a other.
*/
QBluetoothAddress::QBluetoothAddress(const QBluetoothAddress &other)
{
    *this = other;
}

/*!
    Destroys the QBluetoothAddress.
*/
QBluetoothAddress::~QBluetoothAddress()
{
}

/*!
    Assigns \a other to this Bluetooth address.
*/
QBluetoothAddress &QBluetoothAddress::operator=(const QBluetoothAddress &other)
{
    m_address = other.m_address;

    return *this;
}

/*!
    Sets the Bluetooth address to 00:00:00:00:00:00.
*/
void QBluetoothAddress::clear()
{
    m_address = 0;
}

/*!
    Returns true if the Bluetooth address is null, otherwise returns false.
*/
bool QBluetoothAddress::isNull() const
{
    return m_address == 0;
}

/*!
    Returns this Bluetooth address as a quint64.
*/
quint64 QBluetoothAddress::toUInt64() const
{
    return m_address;
}

/*!
    Returns the Bluetooth address as a string of the form, XX:XX:XX:XX:XX:XX.
*/
QString QBluetoothAddress::toString() const
{
    QString s(QStringLiteral("%1:%2:%3:%4:%5:%6"));

    for (int i = 5; i >= 0; --i) {
        const quint8 a = (m_address >> (i*8)) & 0xff;
        s = s.arg(a, 2, 16, QLatin1Char('0'));
    }

    return s.toUpper();
}

/*!
    \fn bool QBluetoothAddress::operator<(const QBluetoothAddress &a,
                                          const QBluetoothAddress &b)
    \brief Returns true if the Bluetooth address \a a is less than \a b, otherwise
    returns false.
*/

/*!
    \fn bool QBluetoothAddress::operator==(const QBluetoothAddress &a,
                                           const QBluetoothAddress &b)
    \brief Returns \c true if the two Bluetooth addresses \a a and \a b are equal,
    otherwise returns \c false.
*/

/*!
    \fn bool QBluetoothAddress::operator!=(const QBluetoothAddress &a,
                                           const QBluetoothAddress &b)
    \brief Returns \c true if the two Bluetooth addresses \a a and \a b are not equal,
    otherwise returns \c false.
*/

#ifndef QT_NO_DEBUG_STREAM
QDebug QBluetoothAddress::streamingOperator(QDebug debug, const QBluetoothAddress &address)
{
    debug << address.toString();
    return debug;
}
#endif

QT_END_NAMESPACE

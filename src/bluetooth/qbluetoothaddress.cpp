// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothaddress.h"

#ifndef QT_NO_DEBUG_STREAM
#include <QDebug>
#endif

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN(QBluetoothAddress)

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

    Constructs a null Bluetooth Address.
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

    if (a.size() == 17)
        a.remove(QLatin1Char(':'));

    if (a.size() == 12) {
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

/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbluetoothhostinfo.h"
#include "qbluetoothhostinfo_p.h"

QTBLUETOOTH_BEGIN_NAMESPACE

/*!
    \class QBluetoothHostInfo
    \brief The QBluetoothHostInfo class encapsulates the details of a local
    QBluetooth device.

    \ingroup connectivity-bluetooth
    \inmodule QtBluetooth

    This class holds the name and address of a local Bluetooth device.
*/

/*!
    Constrcuts a null QBluetoothHostInfo object.
*/
QBluetoothHostInfo::QBluetoothHostInfo()
    : d_ptr(new QBluetoothHostInfoPrivate)
{
}

/*!
    Constrcuts a new QBluetoothHostInfo which is a copy of \a other.
*/
QBluetoothHostInfo::QBluetoothHostInfo(const QBluetoothHostInfo &other)
    : d_ptr(new QBluetoothHostInfoPrivate)
{
    Q_D(QBluetoothHostInfo);

    d->m_address = other.d_func()->m_address;
    d->m_name = other.d_func()->m_name;
}

/*!
    Destroys the QBluetoothHostInfo.
*/
QBluetoothHostInfo::~QBluetoothHostInfo()
{
    delete d_ptr;
}

/*!
    Returns the Bluetooth address as a QBluetoothAddress.
*/
QBluetoothAddress QBluetoothHostInfo::getAddress() const
{
    Q_D(const QBluetoothHostInfo);
    return d->m_address;
}

/*!
    Sets the Bluetooth \a address for this Bluetooth host info object.
*/
void QBluetoothHostInfo::setAddress(const QBluetoothAddress &address)
{
    Q_D(QBluetoothHostInfo);
    d->m_address = address;
}

/*!
    Returns the name of the host info object.
*/
QString QBluetoothHostInfo::getName() const
{
    Q_D(const QBluetoothHostInfo);
    return d->m_name;
}

/*!
    Sets the name of the host info object.
*/
void QBluetoothHostInfo::setName(const QString &name)
{
    Q_D(QBluetoothHostInfo);
    d->m_name = name;
}

QTBLUETOOTH_END_NAMESPACE

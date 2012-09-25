/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
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
    \inmodule QtBluetooth
    \brief The QBluetoothHostInfo class encapsulates the details of a local
    QBluetooth device.

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
QBluetoothAddress QBluetoothHostInfo::address() const
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
    Returns the user visible name of the host info object.
*/
QString QBluetoothHostInfo::name() const
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

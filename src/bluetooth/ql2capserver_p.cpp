/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include "ql2capserver.h"
#include "ql2capserver_p.h"
#include "qbluetoothsocket.h"

QTBLUETOOTH_BEGIN_NAMESPACE

QL2capServerPrivate::QL2capServerPrivate()
{
}

QL2capServerPrivate::~QL2capServerPrivate()
{
}

void QL2capServer::close()
{
}

bool QL2capServer::listen(const QBluetoothAddress &address, quint16 port)
{
    Q_UNUSED(address);
    Q_UNUSED(port);

    return false;
}

void QL2capServer::setMaxPendingConnections(int numConnections)
{
    Q_UNUSED(numConnections);
}

bool QL2capServer::hasPendingConnections() const
{
    return false;
}

QBluetoothSocket *QL2capServer::nextPendingConnection()
{
    return 0;
}

QBluetoothAddress QL2capServer::serverAddress() const
{
    return QBluetoothAddress();
}

quint16 QL2capServer::serverPort() const
{
    return 0;
}

void QL2capServer::setSecurityFlags(QBluetooth::SecurityFlags security)
{
}

QBluetooth::SecurityFlags QL2capServer::securityFlags() const
{
    return QBluetooth::NoSecurity;
}



#ifdef QT_BLUEZ_BLUETOOTH
void QL2capServerPrivate::_q_newConnection()
{
}
#endif

QTBLUETOOTH_END_NAMESPACE

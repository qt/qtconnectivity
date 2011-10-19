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

#include "qrfcommserver.h"
#include "qrfcommserver_p.h"
#include "qbluetoothsocket.h"

QTBLUETOOTH_BEGIN_NAMESPACE

QRfcommServerPrivate::QRfcommServerPrivate()
{
}

QRfcommServerPrivate::~QRfcommServerPrivate()
{
}

void QRfcommServer::close()
{
}

bool QRfcommServer::listen(const QBluetoothAddress &address, quint16 port)
{
    Q_UNUSED(address);
    Q_UNUSED(port);
    return false;
}

void QRfcommServer::setMaxPendingConnections(int numConnections)
{
    Q_UNUSED(numConnections);
}

bool QRfcommServer::hasPendingConnections() const
{
    return false;
}

QBluetoothSocket *QRfcommServer::nextPendingConnection()
{
    return 0;
}

QBluetoothAddress QRfcommServer::serverAddress() const
{
    return QBluetoothAddress();
}

quint16 QRfcommServer::serverPort() const
{
    return 0;
}


#ifdef QT_BLUEZ_BLUETOOTH
void QRfcommServerPrivate::_q_newConnection()
{
}
#endif


#ifdef QT_SYMBIAN_BLUETOOTH
void QRfcommServerPrivate::HandleAcceptCompleteL(TInt aErr)
{
}
void QRfcommServerPrivate::HandleActivateBasebandEventNotifierCompleteL(TInt aErr, TBTBasebandEventNotification &aEventNotification)
{
}
void QRfcommServerPrivate::HandleConnectCompleteL(TInt aErr)
{
}
void QRfcommServerPrivate::HandleIoctlCompleteL(TInt aErr)
{
}
void QRfcommServerPrivate::HandleReceiveCompleteL(TInt aErr)
{
}
void QRfcommServerPrivate::HandleSendCompleteL(TInt aErr)
{
}
void QRfcommServerPrivate::HandleShutdownCompleteL(TInt aErr)
{
}
#endif

void QRfcommServer::setSecurityFlags(QBluetooth::SecurityFlags security)
{
}

QBluetooth::SecurityFlags QRfcommServer::securityFlags() const
{
    return QBluetooth::NoSecurity;
}

QTBLUETOOTH_END_NAMESPACE

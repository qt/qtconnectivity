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

#ifndef QRFCOMMSERVER_P_H
#define QRFCOMMSERVER_P_H

#include <QtGlobal>
#include <QList>
#include <qbluetoothsocket.h>

#ifdef QT_SYMBIAN_BLUETOOTH
#include <es_sock.h>
#include <bt_sock.h>
#endif

#ifdef QT_BLUEZ_BLUETOOTH
QT_FORWARD_DECLARE_CLASS(QSocketNotifier)
#endif

QT_BEGIN_HEADER

class QBluetoothAddress;
class QBluetoothSocket;

#ifdef QT_SYMBIAN_BLUETOOTH
class QBluetoothSocketPrivate;
#endif

class QRfcommServer;

class QRfcommServerPrivate
{
    Q_DECLARE_PUBLIC(QRfcommServer)

public:
    QRfcommServerPrivate();
    ~QRfcommServerPrivate();

#ifdef QT_SYMBIAN_BLUETOOTH
    // private slots
    void _q_connected();
    void _q_socketError(QBluetoothSocket::SocketError err);
    void _q_disconnected();
#endif //QT_SYMBIAN_BLUETOOTH

#ifdef QT_BLUEZ_BLUETOOTH
    void _q_newConnection();
#endif

public:
    QBluetoothSocket *socket;

#ifdef QT_SYMBIAN_BLUETOOTH
    mutable QList<QBluetoothSocket *> activeSockets;
    QBluetoothSocketPrivate *ds;
#endif

    int maxPendingConnections;
    QBluetooth::SecurityFlags securityFlags;

protected:
    QRfcommServer *q_ptr;

private:
#ifdef QT_BLUEZ_BLUETOOTH
    QSocketNotifier *socketNotifier;
#endif
};

QT_END_HEADER

#endif

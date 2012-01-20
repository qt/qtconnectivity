/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QL2CAPSERVER_H
#define QL2CAPSERVER_H

#include "qbluetoothglobal.h"

#include <QObject>

#include <qbluetoothaddress.h>
#include <qbluetooth.h>
#include <qbluetoothsocket.h>

QT_BEGIN_HEADER

QTBLUETOOTH_BEGIN_NAMESPACE

class QL2capServerPrivate;
class QBluetoothSocket;

class Q_BLUETOOTH_EXPORT QL2capServer : public QObject
{
    Q_OBJECT

public:
    QL2capServer(QObject *parent = 0);
    ~QL2capServer();

    void close();

    bool listen(const QBluetoothAddress &address = QBluetoothAddress(), quint16 port = 0);
    bool isListening() const;

    void setMaxPendingConnections(int numConnections);
    int maxPendingConnections() const;

    bool hasPendingConnections() const;
    QBluetoothSocket *nextPendingConnection();

    QBluetoothAddress serverAddress() const;
    quint16 serverPort() const;

    void setSecurityFlags(QBluetooth::SecurityFlags security);
    QBluetooth::SecurityFlags securityFlags() const;

signals:
    void newConnection();

protected:
    QL2capServerPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(QL2capServer)

#ifdef QT_SYMBIAN_BLUETOOTH
    Q_PRIVATE_SLOT(d_func(), void _q_connected())
    Q_PRIVATE_SLOT(d_func(), void _q_socketError(QBluetoothSocket::SocketError err))
    Q_PRIVATE_SLOT(d_func(), void _q_disconnected())
#endif //QT_SYMBIAN_BLUETOOTH
    
#ifdef QT_BLUEZ_BLUETOOTH
    Q_PRIVATE_SLOT(d_func(), void _q_newConnection())
#endif

};

QTBLUETOOTH_END_NAMESPACE

QT_END_HEADER

#endif

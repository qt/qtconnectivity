/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#include "qlowenergycontrollernew_p.h"
#include "qbluetoothsocket_p.h"

#include <QtCore/QLoggingCategory>
#include <QtBluetooth/QBluetoothSocket>

#define ATTRIBUTE_CHANNEL_ID 4

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

void QLowEnergyControllerNewPrivate::connectToDevice()
{
    setState(QLowEnergyControllerNew::ConnectingState);
    if (l2cpSocket)
        delete l2cpSocket;

    l2cpSocket = new QBluetoothSocket(QBluetoothServiceInfo::L2capProtocol, this);
    connect(l2cpSocket, SIGNAL(connected()), this, SLOT(l2cpConnected()));
    connect(l2cpSocket, SIGNAL(disconnected()), this, SLOT(l2cpDisconnected()));
    connect(l2cpSocket, SIGNAL(error(QBluetoothSocket::SocketError)),
            this, SLOT(l2cpErrorChanged(QBluetoothSocket::SocketError)));

    l2cpSocket->d_ptr->isLowEnergySocket = true;
    l2cpSocket->connectToService(remoteDevice, ATTRIBUTE_CHANNEL_ID);
}

void QLowEnergyControllerNewPrivate::l2cpConnected()
{
    Q_Q(QLowEnergyControllerNew);

    setState(QLowEnergyControllerNew::ConnectedState);
    emit q->connected();
}

void QLowEnergyControllerNewPrivate::disconnectFromDevice()
{
    setState(QLowEnergyControllerNew::ClosingState);
    l2cpSocket->close();
}

void QLowEnergyControllerNewPrivate::l2cpDisconnected()
{
    Q_Q(QLowEnergyControllerNew);

    setState(QLowEnergyControllerNew::UnconnectedState);
    emit q->disconnected();
}

void QLowEnergyControllerNewPrivate::l2cpErrorChanged(QBluetoothSocket::SocketError e)
{
    switch (e) {
    case QBluetoothSocket::HostNotFoundError:
        setError(QLowEnergyControllerNew::UnknownRemoteDeviceError);
        qCDebug(QT_BT_BLUEZ) << "The passed remote device address cannot be found";
        break;
    case QBluetoothSocket::NetworkError:
        setError(QLowEnergyControllerNew::NetworkError);
        qCDebug(QT_BT_BLUEZ) << "Network IO error while talking to LE device";
        break;
    case QBluetoothSocket::UnknownSocketError:
    case QBluetoothSocket::UnsupportedProtocolError:
    case QBluetoothSocket::OperationError:
    case QBluetoothSocket::ServiceNotFoundError:
    default:
        // these errors shouldn't happen -> as it means
        // the code in this file has bugs
        qCDebug(QT_BT_BLUEZ) << "Unknown l2cp socket error: " << e << l2cpSocket->errorString();
        setError(QLowEnergyControllerNew::UnknownError);
        break;
    }

    setState(QLowEnergyControllerNew::UnconnectedState);
}

QT_END_NAMESPACE

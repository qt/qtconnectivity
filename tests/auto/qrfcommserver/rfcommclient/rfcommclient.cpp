/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtNfc module of the Qt Toolkit.
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

#include "rfcommclient.h"

#include <qbluetoothsocket.h>
#include <qbluetoothlocaldevice.h>

#include <QtCore/QDataStream>
#include <QtCore/QByteArray>

#include <QtCore/QStringList>

void MyThread::sleep(int seconds)
{
    QThread::sleep(seconds);
}

RfCommClient::RfCommClient(QObject *parent)
:   QObject(parent), socket(0), stream(0), elapsed(new QTime), lagTimeout(0), state(listening)
{
    lagTimer.setSingleShot(true);
    lagTimer.setInterval(5000);
}

RfCommClient::~RfCommClient()
{
    stopClient();
}

bool RfCommClient::powerOn()
{
    qDebug() << __PRETTY_FUNCTION__ << ">>";
    // turn on BT in case it is not on
    if (localDevice.hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
        connect(&localDevice, SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)),
                this, SLOT(hostModeStateChanged(QBluetoothLocalDevice::HostMode)));
//        connect(localDevice, SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)),
//                this, SLOT(hostModeStateChanged(QBluetoothLocalDevice::HostMode)));
        qDebug() << __PRETTY_FUNCTION__ << "Turning power on";
        localDevice.powerOn();
    } else {
        qDebug() << __PRETTY_FUNCTION__ << "<< Power already on! returning true";
        return true;
    }
    qDebug() << __PRETTY_FUNCTION__ << "<< returning false";
    return false;
}

void RfCommClient::hostModeStateChanged(QBluetoothLocalDevice::HostMode mode)
{
    qDebug() << __PRETTY_FUNCTION__ << mode;
    if (mode != QBluetoothLocalDevice::HostPoweredOff)
        startClient(serviceInfo);
}
//! [startClient]
void RfCommClient::startClient(const QBluetoothServiceInfo &remoteService)
{
    qDebug() << __PRETTY_FUNCTION__ << ">>";
    serviceInfo = remoteService;

    // make sure preconditions are met
    if (!powerOn() || socket) {
        qDebug() << __PRETTY_FUNCTION__ << "<< power not on or socket already exists!";
        return;
    }

    // Connect to service
    if (state == listening)
        state = pendingConnections;
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    qDebug() << "Create socket";
    socket->connectToService(remoteService);
    qDebug() << "ConnecttoService done";

    connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(socket, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(error(QBluetoothSocket::SocketError)));

    qDebug() << __PRETTY_FUNCTION__ << "<<";
}
//! [startClient]

//! [stopClient]
void RfCommClient::stopClient()
{
    qDebug() << __PRETTY_FUNCTION__ << "closing client!";

    lagTimer.stop();

    delete stream;
    stream = 0;

    socket->deleteLater();
    socket = 0;
}
//! [stopClient]

//! [socketDisconnected]
void RfCommClient::socketDisconnected()
{
    // Note:  it seems that the "disconnected" signal is not emitted by the socket, so this never gets called
    qDebug() << __PRETTY_FUNCTION__ << "Got socketDisconnected";
    emit disconnected();
    stopClient();

    // now reconnect and send text string
    startClient(serviceInfo);
    connect(&lagTimer, SIGNAL(timeout()), this, SLOT(sendText()));
    lagTimer.start();
}
//! [socketDisconnected]

//! [readSocket]
void RfCommClient::readSocket()
{
    if (!socket)
        return;
    QString str;

    while (socket->bytesAvailable()) {
        *stream >> str;
    }
    qDebug() << __PRETTY_FUNCTION__ << "socket read=" << str;
}
//! [readSocket]

//! [connected]
void RfCommClient::connected()
{
    qDebug() << __PRETTY_FUNCTION__ << "connected to " << socket->peerName();
    stream = new QDataStream(socket);
    emit connected(socket->peerName());
}
//! [connected]

void RfCommClient::error(QBluetoothSocket::SocketError err)
{
    qDebug() << __PRETTY_FUNCTION__ << "Got socket error" << err;
    // remote side has closed the socket, effectively disconnecting it
    if (state == pendingConnections) {
        state = dataTransfer;
        emit disconnected();
        stopClient();

        // now reconnect and send text string
        MyThread mythread;
        mythread.sleep(5);
        startClient(serviceInfo);
        connect(&lagTimer, SIGNAL(timeout()), this, SLOT(sendText()));
        lagTimer.start();
    } else {
        qDebug() << __PRETTY_FUNCTION__ << "emitting done";
        emit done();
    }
}

void RfCommClient::sendText()
{
    qDebug() << __PRETTY_FUNCTION__ << ">>";
    lagTimer.stop();
    if (stream) {
        buffer.clear();
        buffer.append(QString("hello\r\n"));  // ideally we would use QDataStream here
        socket->write(buffer);
    }
    // terminate client program
    emit done();
    qDebug() << __PRETTY_FUNCTION__ << "<< Terminating program";
}



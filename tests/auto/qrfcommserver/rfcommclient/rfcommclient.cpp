/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
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
    socket = new QBluetoothSocket(QBluetoothSocket::RfcommSocket);
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



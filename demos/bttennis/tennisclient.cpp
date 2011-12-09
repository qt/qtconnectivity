/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "tennisclient.h"

#include <qbluetoothsocket.h>

#include <QtCore/QDataStream>
#include <QtCore/QByteArray>

#include <QtCore/QStringList>

TennisClient::TennisClient(QObject *parent)
:   QObject(parent), socket(0), stream(0), elapsed(new QTime), lagTimeout(0)
{
    lagTimer.setInterval(1000);
    connect(&lagTimer, SIGNAL(timeout()), this, SLOT(sendEcho()));
}

TennisClient::~TennisClient()
{
    stopClient();
}

//! [startClient]
void TennisClient::startClient(const QBluetoothServiceInfo &remoteService)
{
    if (socket)
        return;

    serviceInfo = remoteService;

    // Connect to service
    socket = new QBluetoothSocket(QBluetoothSocket::L2capSocket);
    qDebug() << "Create socket";
    socket->connectToService(remoteService);
    qDebug() << "ConnecttoService done";

    connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(socket, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(error(QBluetoothSocket::SocketError)));

    lagTimer.start();
}
//! [startClient]

//! [stopClient]
void TennisClient::stopClient()
{
    if (socket == 0) // already stopped
        return;

    qDebug() << Q_FUNC_INFO << "closing client!";

    lagTimer.stop();

    delete stream;
    stream = 0;

    socket->deleteLater();
    socket = 0;
}
//! [stopClient]

//! [socketDisconnected]
void TennisClient::socketDisconnected()
{
    qDebug() << "Got socketDisconnected";
    stopClient();
}
//! [socketDisconnected]

//! [readSocket]
void TennisClient::readSocket()
{
    if (!socket)
        return;

    while (socket->bytesAvailable()) {
        QString str;

        *stream >> str;

        QStringList args = str.split(QChar(' '));
        QString s = args.takeFirst();

        if (s == "m" && args.count() == 2) {
            emit moveBall(args.at(0).toInt(), args.at(1).toInt());
        }
        else if (s == "s" && args.count() == 2){
            emit score(args.at(0).toInt(), args.at(1).toInt());
        }
        else if (s == "l" && args.count() == 1){
            emit moveLeftPaddle(args.at(0).toInt());
        }
        else if (s == "e"){ // echo
            QByteArray b;
            QDataStream s(&b, QIODevice::WriteOnly);
            s << str;
            socket->write(b);
        }
        else if (s == "E"){
            lagTimeout = 0;
            QTime then = QTime::fromString(args.at(0), "hh:mm:ss.zzz");
            if (then.isValid()) {
                emit lag(then.msecsTo(QTime::currentTime()));
//                qDebug() << "RTT: " << then.msecsTo(QTime::currentTime()) << "ms";
            }
        }
        else {
            qDebug() << Q_FUNC_INFO << "Unknown command" << str;
        }
    }
}
//! [readSocket]

//! [moveRightPaddle]
void TennisClient::moveRightPaddle(int y)
{
    int msec = elapsed->elapsed();

    if (stream && msec > 50) {
        QByteArray b;
        QDataStream s(&b, QIODevice::WriteOnly);
        s << QString("r %1").arg(y);
        socket->write(b);
        elapsed->restart();
    }
}
//! [moveRightPaddle]

//! [connected]
void TennisClient::connected()
{
    stream = new QDataStream(socket);
    emit connected(socket->peerName());
}
//! [connected]

void TennisClient::error(QBluetoothSocket::SocketError err)
{
    qDebug() << "Got socket error" <<Q_FUNC_INFO << "error" << err;
    emit disconnected();
}

void TennisClient::sendEcho()
{
    if (lagTimeout) {
        lagTimeout--;
        return;
    }

    if (stream) {
        QByteArray b;
        QDataStream s(&b, QIODevice::WriteOnly);
        s << QString("E %1").arg(QTime::currentTime().toString("hh:mm:ss.zzz"));
        socket->write(b);
        lagTimeout = 10;
    }
}

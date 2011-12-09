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

#include "tennisserver.h"
#include "tennis.h"

#include <ql2capserver.h>
#include <qbluetoothsocket.h>

#include <QDebug>

TennisServer::TennisServer(QObject *parent)
:   QObject(parent), l2capServer(0), clientSocket(0), stream(0), lagReplyTimeout(0)
{
    elapsed.start();
    ballElapsed.start();
    lagTimer.setInterval(1000);
    connect(&lagTimer, SIGNAL(timeout()), this, SLOT(sendEcho()));
}

TennisServer::~TennisServer()
{
    if (stream){
        QByteArray b;
        QDataStream s(&b, QIODevice::WriteOnly);
        s << QString("D");
        clientSocket->write(b);
    }

    stopServer();
}

void TennisServer::startServer()
{
    if (l2capServer)
        return;

    //! [Create the server]
    l2capServer = new QL2capServer(this);
    connect(l2capServer, SIGNAL(newConnection()), this, SLOT(clientConnected()));
    l2capServer->listen();
    //! [Create the server]

    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceRecordHandle, (uint)0x00010010);

    //! [Class ServiceClass must contain at least 1 entry]
    QBluetoothServiceInfo::Sequence classId;
//    classId << QVariant::fromValue(QBluetoothUuid(serviceUuid));
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);
    //! [Class ServiceClass must contain at least 1 entry]


    //! [Service name, description and provider]
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, tr("Example Tennis Server"));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription,
                             tr("Example bluetooth tennis server"));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider, tr("Nokia, QtDF"));
    //! [Service name, description and provider]

    //! [Service UUID set]
    serviceInfo.setServiceUuid(QBluetoothUuid(serviceUuid));
    //! [Service UUID set]

    //! [Service Discoverability]
    serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList,
                             QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));
    //! [Service Discoverability]


    //! [Protocol descriptor list]
    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap))
             << QVariant::fromValue(quint16(l2capServer->serverPort()));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                             protocolDescriptorList);
    //! [Protocol descriptor list]

    //! [Register service]
    serviceInfo.registerService();
    //! [Register service]

}

//! [stopServer]
void TennisServer::stopServer()
{
    qDebug() <<Q_FUNC_INFO;
    // Unregister service
    serviceInfo.unregisterService();

    delete stream;
    stream = 0;

    // Close sockets
    delete clientSocket;
    clientSocket = 0;

    // Close server
    delete l2capServer;
    l2capServer = 0;
}
//! [stopServer]

quint16 TennisServer::serverPort() const
{
    return l2capServer->serverPort();
}

//! [moveBall]
void TennisServer::moveBall(int x, int y)
{
    int msec = ballElapsed.elapsed();

    if (stream && msec > 30){
        QByteArray b;
        QDataStream s(&b, QIODevice::WriteOnly);
        s << QString("m %1 %2").arg(x).arg(y);
        //s << QLatin1String("m") << x << y;
        clientSocket->write(b);
        ballElapsed.restart();
    }
}
//! [moveBall]

void TennisServer::score(int left, int right)
{
    if (stream){
        QByteArray b;
        QDataStream s(&b, QIODevice::WriteOnly);
        s << QString("s %1 %2").arg(left).arg(right);
//        s << QChar('s') << left << right;
        clientSocket->write(b);
    }
}

void TennisServer::moveLeftPaddle(int y)
{

    int msec = elapsed.elapsed();

    if (stream && msec > 50) {
        QByteArray b;
        QDataStream s(&b, QIODevice::WriteOnly);
        s << QString("l %1").arg(y);
//        s << QChar('l') << y;
        clientSocket->write(b);
        elapsed.restart();
    }
}

void TennisServer::readSocket()
{
    if (!clientSocket)
        return;

    while (clientSocket->bytesAvailable()) {

        QString str;
        *stream >> str;
        QStringList args = str.split(QChar(' '));
        QString s = args.takeFirst();

        if (s == "r" && args.count() == 1){
            emit moveRightPaddle(args.at(0).toInt());
        }
        else if (s == "e" && args.count() == 1){
            lagReplyTimeout = 0;
            QTime then = QTime::fromString(args.at(0), "hh:mm:ss.zzz");
            if (then.isValid()) {
                emit lag(then.msecsTo(QTime::currentTime()));
//                qDebug() << "RTT: " << then.msecsTo(QTime::currentTime()) << "ms";
            }
        }
        else if (s == "E"){
            QByteArray b;
            QDataStream st(&b, QIODevice::WriteOnly);
            st << str;
            clientSocket->write(b);
        }
        else if (s == "D"){
            qDebug() << Q_FUNC_INFO << "closing!";
            clientSocket->deleteLater();
            clientSocket = 0;
        }
        else {
            qDebug() << Q_FUNC_INFO << "Unknown command" << str;
        }
    }
}

//! [clientConnected]
void TennisServer::clientConnected()
{
    qDebug() << Q_FUNC_INFO << "connect";

    QBluetoothSocket *socket = l2capServer->nextPendingConnection();
    if (!socket)
        return;

    if (clientSocket){
        qDebug() << Q_FUNC_INFO << "Closing socket!";
        delete socket;
        return;
    }

    connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    connect(socket, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(socketError(QBluetoothSocket::SocketError)));

    stream = new QDataStream(socket);

    clientSocket = socket;

    qDebug() << Q_FUNC_INFO << "started";

    emit clientConnected(clientSocket->peerName());
    lagTimer.start();
}
//! [clientConnected]

void TennisServer::socketError(QBluetoothSocket::SocketError err)
{
    qDebug() << Q_FUNC_INFO << err;
}

//! [sendEcho]
void TennisServer::sendEcho()
{
    if (lagReplyTimeout) {
        lagReplyTimeout--;
        return;
    }

    if (stream) {
        QByteArray b;
        QDataStream s(&b, QIODevice::WriteOnly);
        s << QString("e %1").arg(QTime::currentTime().toString("hh:mm:ss.zzz"));
        clientSocket->write(b);
        lagReplyTimeout = 10;
    }
}
//! [sendEcho]

//! [clientDisconnected]
void TennisServer::clientDisconnected()
{
    qDebug() << Q_FUNC_INFO << "client closing!";

    lagTimer.stop();
    lagReplyTimeout = 0;
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket *>(sender());
    if (!socket)
        return;

    emit clientDisconnected(socket->peerName());

    clientSocket->deleteLater();
    clientSocket = 0;
    delete stream;
    stream = 0;

//    socket->deleteLater();
}
//! [clientDisconnected]


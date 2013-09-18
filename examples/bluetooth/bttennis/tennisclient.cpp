/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Toolkit.
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
    socket = new QBluetoothSocket(QBluetoothServiceInfo::L2capProtocol);
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

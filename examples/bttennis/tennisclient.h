/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Toolkit.
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <qbluetoothserviceinfo.h>
#include <qbluetoothsocket.h>

#include <QtCore/QObject>
#include <QtCore/QTime>
#include <QtCore/QTimer>

QTBLUETOOTH_BEGIN_NAMESPACE
class QBluetoothSocket;
QTBLUETOOTH_END_NAMESPACE

class QDataStream;

QTBLUETOOTH_USE_NAMESPACE

//! [declaration]
class TennisClient : public QObject
{
    Q_OBJECT

public:
    explicit TennisClient(QObject *parent = 0);
    ~TennisClient();

    void startClient(const QBluetoothServiceInfo &remoteService);
    void stopClient();

public slots:
    void moveRightPaddle(int y);
    void error(QBluetoothSocket::SocketError);

signals:
    void moveBall(int x, int y);
    void score(int left, int right);
    void moveLeftPaddle(int y);

    void connected(const QString &name);
    void disconnected();

    void lag(int ms);

private slots:
    void readSocket();
    void connected();
    void sendEcho();
    void socketDisconnected();

private:
    QBluetoothSocket *socket;
    QDataStream *stream;
    QBluetoothServiceInfo serviceInfo;
    QTime *elapsed;
    QTimer lagTimer;
    int lagTimeout;
};
//! [declaration]

#endif // CHATCLIENT_H

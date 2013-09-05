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

#ifndef TENNISSERVER_H
#define TENNISSERVER_H

#include <qbluetoothserviceinfo.h>
#include <qbluetoothsocket.h>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QDataStream>
#include <QtCore/QTime>
#include <QtCore/QTimer>

QT_FORWARD_DECLARE_CLASS(QBluetoothServer)
QT_FORWARD_DECLARE_CLASS(QBluetoothSocket)
QT_FORWARD_DECLARE_CLASS(QBluetoothServiceInfo)

QT_USE_NAMESPACE

//! [declaration]
class TennisServer : public QObject
{
    Q_OBJECT

public:
    explicit TennisServer(QObject *parent = 0);
    ~TennisServer();

    void startServer();
    void stopServer();

    quint16 serverPort() const;

public slots:
    void moveBall(int x, int y);
    void score(int left, int right);
    void moveLeftPaddle(int y);

signals:
    void moveRightPaddle(int y);
    void clientDisconnected(const QString &name);
    void clientConnected(const QString &name);
    void lag(int ms);

private slots:
    void clientConnected();
    void clientDisconnected();
    void readSocket();
    void sendEcho();
    void socketError(QBluetoothSocket::SocketError err);

private:
    QBluetoothServer *l2capServer;
    QBluetoothServiceInfo serviceInfo;
    QBluetoothSocket *clientSocket;
    QDataStream *stream;
    QTime elapsed;
    QTime ballElapsed;
    QTimer lagTimer;
    int lagReplyTimeout;
};
//! [declaration]

#endif // CHATSERVER_H

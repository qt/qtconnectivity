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

#ifndef RFCOMMCLIENT_H
#define RFCOMMCLIENT_H

#include <qbluetoothserviceinfo.h>
#include <qbluetoothsocket.h>

#include <QtCore/QObject>
#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QBluetoothLocalDevice>
#include <QThread>

static const QLatin1String serviceUuid("e8e10f95-1a70-4b27-9ccf-02010264e9c9");

class QBluetoothSocket;

class QDataStream;

class MyThread : public QThread
{
public:
    void sleep(int seconds);
};


//! [declaration]
class RfCommClient : public QObject
{
    Q_OBJECT

    enum States {
        listening,
        pendingConnections,
        dataTransfer
    };
public:
    explicit RfCommClient(QObject *parent = 0);
    ~RfCommClient();

    void startClient(const QBluetoothServiceInfo &remoteService);
    void stopClient();

public slots:
    void error(QBluetoothSocket::SocketError);

signals:
    void connected(const QString &name);
    void disconnected();
    void done();
    void lag(int ms);

private slots:
    void readSocket();
    void connected();
    void sendText();
    void socketDisconnected();
    void hostModeStateChanged(QBluetoothLocalDevice::HostMode);

private:
    bool powerOn();

private:
    QBluetoothSocket *socket;
    QDataStream *stream;
    QByteArray buffer;
    QBluetoothServiceInfo serviceInfo;
    QBluetoothLocalDevice localDevice;
    QTime *elapsed;
    QTimer lagTimer;
    int lagTimeout;
    RfCommClient::States state;
};
//! [declaration]

#endif // RFCOMMCLIENT_H

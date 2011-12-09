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

#include "ui_tennis.h"

#include <QDialog>

#include <QResizeEvent>
#include <QMoveEvent>
#include <QPropertyAnimation>
#include <qbluetoothserviceinfo.h>
#include <qbluetoothsocket.h>
#include <qbluetoothdevicediscoveryagent.h>

#include "board.h"
#include "controller.h"

#include <QDebug>

QTM_BEGIN_NAMESPACE
class QBluetoothServiceDiscoveryAgent;
QTM_END_NAMESPACE

QTM_USE_NAMESPACE

static const QLatin1String serviceUuid("e8e10f95-1a70-4b27-9ccf-02010264e9c9");

class TennisServer;
class TennisClient;
class Handover;

//! [declaration]
class Tennis : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(int paddlePos READ paddlePos WRITE setPaddlePos)

public:
    Tennis(QWidget *parent = 0);
    ~Tennis();

    int paddlePos() { return paddle_pos; }
    void setPaddlePos(int p);

signals:
    void moveLeftPaddle(int y);
    void moveRightPaddle(int y);

protected:
    void wheelEvent ( QWheelEvent * event );
    void keyPressEvent ( QKeyEvent * event );
    void resizeEvent(QResizeEvent *);

private slots:
    void serverConnected(const QString &name);
    void serverDisconnected();

    void clientConnected(const QString &name);
    void clientDisconnected();

    void serviceDiscovered(const QBluetoothServiceInfo &serviceInfo);
    void discoveryFinished();

    void startDiscovery();

    void mouseMove(int x, int y);

    void lagReport(int ms);

    void nearFieldHandover();

    void fps(const QString &f);

private:

    void moveUp(int px = 10);
    void moveDown(int px = 10);

    void move(int px);

    Ui_Tennis *ui;

    Board *board;
    Controller *controller;

    int paddle_pos;
    int endPaddlePos;

    bool isClient;
    bool isConnected;
    bool quickDiscovery;

    QBluetoothSocket *socket;
    TennisServer *server;
    TennisClient *client;

    QPropertyAnimation *paddleAnimation;

    QBluetoothServiceDiscoveryAgent *m_discoveryAgent;
    Handover *m_handover;
};
//! [declaration]

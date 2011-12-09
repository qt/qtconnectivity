/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Mobility Components.
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

#include "tennis.h"

#include <QTimer>
#include <QKeyEvent>
#include <QMouseEvent>

#include <QDebug>

#include <QSettings>
#include <QEvent>
#include <QResizeEvent>

#include "tennisserver.h"
#include "tennisclient.h"
#include "handover.h"

#include <qbluetooth.h>
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothsocket.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothlocaldevice.h>

#include <qnearfieldmanager.h>
#include <qllcpserver.h>
#include <qllcpsocket.h>

Tennis::Tennis(QWidget *parent)
: QDialog(parent), ui(new Ui_Tennis), board(new Board), controller(new Controller), socket(0),
  m_discoveryAgent(new QBluetoothServiceDiscoveryAgent), m_handover(0)
{
    // start Bluetooth if not started
    QBluetoothLocalDevice *device = new QBluetoothLocalDevice();
    device->powerOn();
    delete device;
    device = 0;

    //! [Construct UI]
    ui->setupUi(this);

    isClient = false;
    isConnected = false;
    quickDiscovery = true;

#if defined (Q_OS_SYMBIAN) || defined(Q_OS_WINCE) || defined(Q_WS_MAEMO_5) || defined(Q_WS_MAEMO_6)
    setWindowState(Qt::WindowMaximized);
#endif

    ui->pongView->setScene(board->getScene());

    connect(ui->pongView, SIGNAL(mouseMove(int, int)), this, SLOT(mouseMove(int, int)));
    ui->pongView->setMouseTracking(false);

    connect(board, SIGNAL(ballCollision(Board::Edge)), controller, SLOT(ballCollision(Board::Edge)));
    connect(board, SIGNAL(scored(Board::Edge)), controller, SLOT(scored(Board::Edge)));
    connect(controller, SIGNAL(moveBall(int,int)), board, SLOT(setBall(int,int)));
    connect(this, SIGNAL(moveLeftPaddle(int)), board, SLOT(setLeftPaddle(int)));
    connect(this, SIGNAL(moveRightPaddle(int)), board, SLOT(setRightPaddle(int)));
    connect(controller, SIGNAL(score(int,int)), board, SLOT(setScore(int,int)));
    connect(controller, SIGNAL(fps(const QString&)), this, SLOT(fps(const QString&)));

    setFocusPolicy(Qt::WheelFocus);

    paddle_pos = (Board::Height-12)/2-Board::Paddle/2;
    endPaddlePos = paddle_pos;

    emit moveLeftPaddle(paddle_pos);
    board->setRightPaddle(paddle_pos);

    server = new TennisServer(this);

    connect(controller, SIGNAL(moveBall(int,int)), server, SLOT(moveBall(int,int)));
    connect(controller, SIGNAL(score(int,int)), server, SLOT(score(int,int)));
    connect(this, SIGNAL(moveLeftPaddle(int)), server, SLOT(moveLeftPaddle(int)));
    connect(server, SIGNAL(clientConnected(QString)), this, SLOT(serverConnected(QString)));
    connect(server, SIGNAL(clientDisconnected(QString)), this, SLOT(serverDisconnected()));
    connect(server, SIGNAL(moveRightPaddle(int)), board, SLOT(setRightPaddle(int)));
    connect(server, SIGNAL(lag(int)), this, SLOT(lagReport(int)));

    connect(server, SIGNAL(clientConnected(QString)), controller, SLOT(refresh()));

    server->startServer();

    client = new TennisClient(this);

    connect(client, SIGNAL(moveBall(int,int)), board, SLOT(setBall(int,int)));
    connect(client, SIGNAL(moveLeftPaddle(int)), board, SLOT(setLeftPaddle(int)));
    connect(client, SIGNAL(connected(QString)), this, SLOT(clientConnected(QString)));
    connect(client, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    connect(this, SIGNAL(moveRightPaddle(int)), client, SLOT(moveRightPaddle(int)));
    connect(client, SIGNAL(score(int,int)), board, SLOT(setScore(int,int)));
    connect(client, SIGNAL(lag(int)), this, SLOT(lagReport(int)));

    connect(this, SIGNAL(moveLeftPaddle(int)), controller, SLOT(moveLeftPaddle(int)));
    connect(this, SIGNAL(moveRightPaddle(int)), controller, SLOT(moveRightPaddle(int)));
    connect(server, SIGNAL(moveRightPaddle(int)), controller, SLOT(moveRightPaddle(int)));


//    ui->pongView->setBackgroundBrush(QBrush(Qt::white));
    ui->pongView->setCacheMode(QGraphicsView::CacheBackground);

    QNearFieldManager nearFieldManager;
    if (nearFieldManager.isAvailable()) {
        m_handover = new Handover(server->serverPort(), this);
        connect(m_handover, SIGNAL(bluetoothServiceChanged()), this, SLOT(nearFieldHandover()));

        connect(m_discoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)),
                this, SLOT(serviceDiscovered(QBluetoothServiceInfo)));
        connect(m_discoveryAgent, SIGNAL(finished()), this, SLOT(discoveryFinished()));
    }


    m_discoveryAgent->setUuidFilter(QBluetoothUuid(serviceUuid));


    QString address;
    QString port;
    QStringList args = QCoreApplication::arguments();
    if (args.length() >= 2){
        address = args.at(1);
        if (args.length() >= 3){
            port = args.at(2);
        }
    }

    if (address.isEmpty()){
        QSettings settings("QtDF", "bttennis");
        address = settings.value("lastclient").toString();
    }

    if (!address.isEmpty()){
        qDebug() << "Connect to" << address << port;
        QBluetoothDeviceInfo device = QBluetoothDeviceInfo(QBluetoothAddress(address), "", QBluetoothDeviceInfo::ComputerDevice);
        QBluetoothServiceInfo service;
        if (!port.isEmpty()) {
            QBluetoothServiceInfo::Sequence protocolDescriptorList;
            QBluetoothServiceInfo::Sequence protocol;
            protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap))
                     << QVariant::fromValue(port.toUShort());
            protocolDescriptorList.append(QVariant::fromValue(protocol));
            service.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                                 protocolDescriptorList);
            qDebug() << "port" << port.toUShort() << service.protocolServiceMultiplexer();
        }
        else {
            service.setServiceUuid(QBluetoothUuid(serviceUuid));
        }
        service.setDevice(device);
        client->startClient(service);
        board->setStatus("Connecting", 100, 25);
    } else if (nearFieldManager.isAvailable()) {
        board->setStatus(tr("Touch to play"), 100, 25);
    }

    setEnabled(true);

    paddleAnimation = new QPropertyAnimation(this, "paddlePos", this);
    paddleAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    ui->pongView->installEventFilter(this);

}

Tennis::~Tennis()
{
}

void Tennis::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::MoveToNextLine)) {
        moveDown();
    }
    else if (event->matches(QKeySequence::MoveToPreviousLine)){
        moveUp();
    }
}

void Tennis::wheelEvent(QWheelEvent *event)
{
    if (event->delta() > 0){
        moveUp();
    }
    else {
        moveDown();
    }
}

void Tennis::moveUp(int px)
{
    endPaddlePos -= px;
    if (endPaddlePos <= 0)
        endPaddlePos = 0;
    move(endPaddlePos);
}

void Tennis::moveDown(int px)
{
    endPaddlePos += px;
    if (endPaddlePos > Board::Height-Board::Paddle-24)
        endPaddlePos = Board::Height-Board::Paddle-24;
    move(endPaddlePos);

}

void Tennis::move(int px)
{
    int distance = abs(paddle_pos - endPaddlePos);

    paddleAnimation->stop();
    paddleAnimation->setStartValue(paddle_pos);
    paddleAnimation->setEndValue(px);
    paddleAnimation->setDuration((1000*distance)/350);
    paddleAnimation->start();
}

void Tennis::setPaddlePos(int p)
{
    paddle_pos = p;
    if (isClient)
        emit moveRightPaddle(paddle_pos);
    else
        emit moveLeftPaddle(paddle_pos);
}


void Tennis::mouseMove(int x, int y)
{
    if (isConnected == false){
        // look for clicks in the bt connect icon
        if (x > 440 && x < 540 && y > 200 && y < 300) {
            qDebug() << "Got connect click!";
            if (m_discoveryAgent->isActive()) {
                qDebug() << "stopping!";
                m_discoveryAgent->stop();
                board->animateConnect(false);
            }
            else {
                qDebug() << "starting!";
                startDiscovery();
            }
        }

    }
    y-=12+Board::Paddle/2;
    if (y <= 0)
        y = 0;
    else if (y > Board::Height-Board::Paddle-24)
        y = Board::Height-Board::Paddle-24;

    endPaddlePos = y;
    move(y);
}

void Tennis::clientConnected(const QString &name)
{
    board->setStatus("Connected to " + name, 100, 0);
    controller->stop();
    server->stopServer();
    isClient = true;
    isConnected = true;
    board->animateConnect(false);
    board->fadeConnect(true);
    emit moveRightPaddle(paddle_pos);
}

void Tennis::clientDisconnected()
{
    board->setStatus("Disconnect", 100, 25);
    controller->start();
    server->startServer();
    client->stopClient();
    isClient = false;
    isConnected = false;
    discoveryFinished();
}

void Tennis::serverConnected(const QString &name)
{
    board->setStatus("Server for " + name, 100, 0);
    m_discoveryAgent->stop();
    isConnected = true;
    board->animateConnect(false);
    board->fadeConnect(true);
    emit moveLeftPaddle(paddle_pos);
}

void Tennis::serverDisconnected()
{
    board->setStatus("Disconnected", 100, 25);
    isConnected = false;
    discoveryFinished();
}

void Tennis::serviceDiscovered(const QBluetoothServiceInfo &serviceInfo)
{
    qDebug() << "***** Discovered! " << serviceInfo.device().name() << serviceInfo.serviceName() << serviceInfo.serviceUuid();
    qDebug() << "Found one!" << serviceInfo.protocolServiceMultiplexer();
    m_discoveryAgent->stop();
    client->startClient(serviceInfo);
    QSettings settings("QtDF", "bttennis");
    settings.setValue("lastclient", serviceInfo.device().address().toString());
}

void Tennis::discoveryFinished()
{
    if (!m_discoveryAgent->isActive()) {
        if (!isConnected) {
            board->setStatus("Waiting", 100, 25);
    //        QTimer::singleShot(60000, this, SLOT(startDiscovery()));
            board->animateConnect(false);
            board->fadeConnect(false);
       }
    }
}

void Tennis::startDiscovery()
{
    qDebug() << "startDiscovery() called";
    if (!isConnected) {
        qDebug() << "Scanning!";
        board->setStatus("Scanning", 100, 25);
        board->fadeConnect(false);
        board->animateConnect(true);
        m_discoveryAgent->start(QBluetoothServiceDiscoveryAgent::MinimalDiscovery);
//        if (quickDiscovery)
//            m_discoveryAgent->start(QBluetoothServiceDiscoveryAgent::MinimalDiscovery);
//        else
//            m_discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
//        quickDiscovery = !quickDiscovery;
    }
    else {
        qDebug() << "Stop scanning!";
        board->setStatus("", 0, 0);
        board->animateConnect(false);
        board->fadeConnect(true);
    }
}

void Tennis::resizeEvent(QResizeEvent *re)
{
    if (re->oldSize().height() > 0){
        qreal x, y;
        x = (re->size().width())/qreal(re->oldSize().width());
        y = (re->size().height())/qreal(re->oldSize().height());
        ui->pongView->scale(x, y);
    }
    ui->pongView->resize(re->size());
}

void Tennis::lagReport(int ms)
{
    if (ms > 250){
        board->setStatus(QString("Caution Lag %1ms").arg(ms), 100, 0);
    }
}

void Tennis::nearFieldHandover()
{
    qDebug() << "Connecting to NFC provided address" << m_handover->bluetoothAddress().toString();

    QBluetoothDeviceInfo device = QBluetoothDeviceInfo(m_handover->bluetoothAddress(), QString(),
                                                       QBluetoothDeviceInfo::ComputerDevice);

    QBluetoothServiceInfo service;
    service.setServiceUuid(QBluetoothUuid(serviceUuid));
    service.setDevice(device);

    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap))
             << QVariant::fromValue(m_handover->serverPort());
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    service.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                         protocolDescriptorList);

    client->startClient(service);
    board->setStatus(tr("Connecting: %1 %2").arg(m_handover->bluetoothAddress().toString()).arg(m_handover->serverPort()), 100, 25);
}

void Tennis::fps(const QString &f)
{
  board->setStatus(f, 100, 100);
}


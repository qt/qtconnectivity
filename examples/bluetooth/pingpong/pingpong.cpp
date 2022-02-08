/***************************************************************************
**
** Copyright (C) 2014 BlackBerry Limited. All rights reserved.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
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
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "pingpong.h"
#include <QDebug>
#include <QCoreApplication>

PingPong::PingPong()
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &PingPong::update);
}

PingPong::~PingPong()
{
    m_timer->stop();
    delete socket;
    delete discoveryAgent;
}

void PingPong::startGame()
{
    m_resultLeft = m_resultRight = 0;
    emit resultChanged();
    m_showDialog = false;
    emit showDialogChanged();
    //! [Start the game]
    m_timer->start(10);
    //! [Start the game]
}

void PingPong::update()
{
    QByteArray size;
    // Server is only updating the coordinates
    //! [Updating coordinates]
    if (m_role == 1) {
        checkBoundaries();
        m_ballY += m_speedy;
        m_ballX += m_speedx;

        size.setNum(m_ballX);
        size.append(' ');
        QByteArray size1;
        size1.setNum(m_ballY);
        size.append(size1);
        size.append(' ');
        size1.setNum(m_leftBlockY);
        size.append(size1);
        size.append(" \n");
        socket->write(size.constData());
        emit ballChanged();
    }
    else if (m_role == 2) {
        size.setNum(m_rightBlockY);
        size.append(" \n");
        socket->write(size.constData());
    }
    //! [Updating coordinates]
}

void PingPong::updateBall(const float &bX, const float &bY)
{
    m_ballX = bX;
    m_ballY = bY;
}

void PingPong::updateLeftBlock(const float &lY)
{
    m_leftBlockY = lY;
}

void PingPong::updateRightBlock(const float &rY)
{
    m_rightBlockY = rY;
}

void PingPong::checkBoundaries()
{
    float ballWidth = 1.f / 27;
    float blockSize = 1.f / 27;
    float blockHeight = 1.f / 5;

    float ballCenterY = m_ballY + ballWidth / 2.;
    //! [Checking the boundaries]
    if (((m_ballX + ballWidth) > (1. - blockSize)) && (ballCenterY < (m_rightBlockY + blockHeight))
        && (ballCenterY > m_rightBlockY)) {
        // right paddle collision
        // simulating paddle surface to be a quarter of a circle
        float paddlecenter = m_rightBlockY + blockHeight / 2.;
        float relpos = (ballCenterY - paddlecenter) / (blockHeight / 2.); // [-1 : 1]
        float surfaceangle = M_PI_4 * relpos;

        // calculation of surface normal
        float normalx = -cos(surfaceangle);
        float normaly = sin(surfaceangle);

        // calculation of surface tangent
        float tangentx = sin(surfaceangle);
        float tangenty = cos(surfaceangle);

        // calculation of tangentialspeed
        float tangentialspeed = tangentx * m_speedx + tangenty * m_speedy;
        // calculation of normal speed
        float normalspeed = normalx * m_speedx + normaly * m_speedy;

        // speed increase of 10%
        normalspeed *= 1.1f;

        if (normalspeed < 0) { // if we are coming from the left. To avoid double reflections
            m_speedx = tangentialspeed * tangentx - normalspeed * normalx;
            m_speedy = tangentialspeed * tangenty - normalspeed * normaly;
        }
    } else if ((m_ballX < blockSize) && (ballCenterY < (m_leftBlockY + blockHeight))
               && (ballCenterY > m_leftBlockY)) {
        // left paddle collision
        // simulating paddle surface to be a quarter of a circle
        float paddlecenter = m_leftBlockY + blockHeight / 2.;
        float relpos = (ballCenterY - paddlecenter) / (blockHeight / 2.); // [-1 : 1]
        float surfaceangle = M_PI_4 * relpos;

        // calculation of surface normal
        float normalx = cos(surfaceangle);
        float normaly = sin(surfaceangle);

        // calculation of surface tangent
        float tangentx = -sin(surfaceangle);
        float tangenty = cos(surfaceangle);

        // calculation of tangentialspeed
        float tangentialspeed = tangentx * m_speedx + tangenty * m_speedy;
        // calculation of normal speed
        float normalspeed = normalx * m_speedx + normaly * m_speedy;

        // speed increase of 10%
        normalspeed *= 1.1f;

        if (normalspeed < 0) { // if we are coming from the left. To avoid double reflections
            m_speedx = tangentialspeed * tangentx - normalspeed * normalx;
            m_speedy = tangentialspeed * tangenty - normalspeed * normaly;
        }
    } else if (m_ballY < 0) {
        m_speedy = std::abs(m_speedy);
    } else if (m_ballY + ballWidth > 1.f) {
        m_speedy = -std::abs(m_speedy);
    }
    //! [Checking the boundaries]
    else if ((m_ballX + ballWidth) > 1.f) {
        m_resultLeft++;
        m_speedx = -0.002f;
        m_speedy = -0.002f;
        m_ballX = 0.5f;
        m_ballY = 0.9f;

        checkResult();
        QByteArray result;
        result.append("result ");
        QByteArray res;
        res.setNum(m_resultLeft);
        result.append(res);
        result.append(' ');
        res.setNum(m_resultRight);
        result.append(res);
        result.append(" \n");
        socket->write(result);
        qDebug() << result;
        emit resultChanged();
    } else if (m_ballX < 0) {
        m_resultRight++;
        m_speedx = 0.002f;
        m_speedy = -0.002f;
        m_ballX = 0.5f;
        m_ballY = 0.9f;

        checkResult();
        QByteArray result;
        result.append("result ");
        QByteArray res;
        res.setNum(m_resultLeft);
        result.append(res);
        result.append(' ');
        res.setNum(m_resultRight);
        result.append(res);
        result.append(" \n");
        socket->write(result);
        emit resultChanged();
    }
}

void PingPong::checkResult()
{
    if (m_resultRight < 10 && m_resultLeft < 10)
        return;

    if (m_resultRight == 10 && m_role == 2) {
        setMessage("Game over. You win! Next game starts in 10s");
    }
    else if (m_resultRight == 10 && m_role == 1) {
        setMessage("Game over. You lose! Next game starts in 10s");
    }
    else if (m_resultLeft == 10 && m_role == 1) {
        setMessage("Game over. You win! Next game starts in 10s");
    }
    else if (m_resultLeft == 10 && m_role == 2) {
        setMessage("Game over. You lose! Next game starts in 10s");
    }
    m_timer->stop();
    QTimer::singleShot(10000, this, SLOT(startGame()));
}

void PingPong::startServer()
{
    setMessage(QStringLiteral("Starting the server"));

    // make discoverable
    QBluetoothLocalDevice().setHostMode(QBluetoothLocalDevice::HostDiscoverable);

    //! [Starting the server]
    m_serverInfo = new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(m_serverInfo, &QBluetoothServer::newConnection,
            this, &PingPong::clientConnected);
    connect(m_serverInfo, &QBluetoothServer::errorOccurred, this, &PingPong::serverError);
    const QBluetoothUuid uuid(serviceUuid);

    m_serviceInfo = m_serverInfo->listen(uuid, QStringLiteral("PingPong server"));
    //! [Starting the server]
    setMessage(QStringLiteral("Server started, waiting for the client. You are the left player."));
    // m_role is set to 1 if it is a server
    m_role = 1;
    emit roleChanged();
}

void PingPong::startClient()
{
    //! [Searching for the service]
    discoveryAgent = new QBluetoothServiceDiscoveryAgent(QBluetoothAddress());

    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered,
            this, &PingPong::addService);
    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished,
            this, &PingPong::done);
    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::errorOccurred, this,
            &PingPong::serviceScanError);
#ifdef Q_OS_ANDROID //see QTBUG-61392
    if (QNativeInterface::QAndroidApplication::sdkVersion() >= 23)
        discoveryAgent->setUuidFilter(QBluetoothUuid(androidUuid));
    else
        discoveryAgent->setUuidFilter(QBluetoothUuid(serviceUuid));
#else
    discoveryAgent->setUuidFilter(QBluetoothUuid(serviceUuid));
#endif
    discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);

    //! [Searching for the service]
    setMessage(QStringLiteral("Starting server discovery. You are the right player"));
    // m_role is set to 2 if it is a client
    m_role = 2;
    emit roleChanged();
}

void PingPong::clientConnected()
{
    //! [Initiating server socket]
    if (!m_serverInfo->hasPendingConnections()) {
        setMessage("FAIL: expected pending server connection");
        return;
    }
    socket = m_serverInfo->nextPendingConnection();
    if (!socket)
        return;
    socket->setParent(this);
    connect(socket, &QBluetoothSocket::readyRead,
            this, &PingPong::readSocket);
    connect(socket, &QBluetoothSocket::disconnected,
            this, &PingPong::clientDisconnected);
    connect(socket, &QBluetoothSocket::errorOccurred, this, &PingPong::socketError);

    //! [Initiating server socket]
    setMessage(QStringLiteral("Client connected. Get ready!"));
    QTimer::singleShot(3000, this, SLOT(startGame()));
}

void PingPong::clientDisconnected()
{
    setMessage(QStringLiteral("Client disconnected"));
    m_timer->stop();
}

void PingPong::socketError(QBluetoothSocket::SocketError error)
{
    Q_UNUSED(error);
    m_timer->stop();
}

void PingPong::serverError(QBluetoothServer::Error error)
{
    Q_UNUSED(error);
    m_timer->stop();
}

void PingPong::done()
{
    qDebug() << "Service scan done";
    if (!m_serviceFound)
        setMessage("PingPong service not found");
}

void PingPong::addService(const QBluetoothServiceInfo &service)
{
    if (m_serviceFound)
        return;
    m_serviceFound = true;

    setMessage("Service found. Stopping discovery and creating connection...");
    discoveryAgent->stop();
    //! [Connecting the socket]
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    socket->connectToService(service);

    connect(socket, &QBluetoothSocket::readyRead, this, &PingPong::readSocket);
    connect(socket, &QBluetoothSocket::connected, this, &PingPong::serverConnected);
    connect(socket, &QBluetoothSocket::disconnected, this, &PingPong::serverDisconnected);
    //! [Connecting the socket]
}

void PingPong::serviceScanError(QBluetoothServiceDiscoveryAgent::Error error)
{
    setMessage(QStringLiteral("Scanning error") + QVariant::fromValue(error).toString());
}

bool PingPong::showDialog() const
{
    return m_showDialog;
}

QString PingPong::message() const
{
    return m_message;
}

void PingPong::serverConnected()
{
    setMessage("Server Connected. Get ready!");
    QTimer::singleShot(3000, this, SLOT(startGame()));
}

void PingPong::serverDisconnected()
{
    setMessage("Server Disconnected");
    m_timer->stop();
}

void PingPong::readSocket()
{
    if (!socket)
        return;
    const char sep = ' ';
    QByteArray line;
    while (socket->canReadLine()) {
        line = socket->readLine();
        //qDebug() << QString::fromUtf8(line.constData(), line.length());
        if (line.contains("result")) {
            QList<QByteArray> result = line.split(sep);
            if (result.size() > 2) {
                QByteArray leftSide = result.at(1);
                QByteArray rightSide = result.at(2);
                m_resultLeft = leftSide.toInt();
                m_resultRight = rightSide.toInt();
                emit resultChanged();
                checkResult();
            }
        }
    }
    if (m_role == 1) {
        QList<QByteArray> boardSize = line.split(sep);
        if (boardSize.size() > 1) {
            QByteArray rightBlockY = boardSize.at(0);
            m_rightBlockY = rightBlockY.toFloat();
            emit rightBlockChanged();
        }
    } else if (m_role == 2) {
        QList<QByteArray> boardSize = line.split(sep);
        if (boardSize.size() > 2) {
            QByteArray ballX = boardSize.at(0);
            QByteArray ballY = boardSize.at(1);
            QByteArray leftBlockY = boardSize.at(2);
            m_ballX = ballX.toFloat();
            m_ballY = ballY.toFloat();
            m_leftBlockY = leftBlockY.toFloat();
            emit leftBlockChanged();
            emit ballChanged();
        }
    }
}

void PingPong::setMessage(const QString &message)
{
    m_showDialog = true;
    m_message = message;
    emit showDialogChanged();
}

int PingPong::role() const
{
    return m_role;
}

int PingPong::leftResult() const
{
    return m_resultLeft;
}

int PingPong::rightResult() const
{
    return m_resultRight;
}

float PingPong::ballX() const
{
    return m_ballX;
}

float PingPong::ballY() const
{
    return m_ballY;
}


float PingPong::leftBlockY() const
{
    return m_leftBlockY;
}

float PingPong::rightBlockY() const
{
    return m_rightBlockY;
}

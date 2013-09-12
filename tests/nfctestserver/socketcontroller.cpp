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

#include "socketcontroller.h"
#include "servicenames.h"

SocketController::SocketController(ConnectionType type, QObject *parent)
:   QObject(parent), m_manager(0), m_socket(new QLlcpSocket(this)), m_connectionType(type),
    m_timerId(-1)
{
    connect(m_socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_socket, SIGNAL(error(QLlcpSocket::SocketError)),
            this, SLOT(error(QLlcpSocket::SocketError)));
    connect(m_socket, SIGNAL(stateChanged(QLlcpSocket::SocketState)),
            this, SLOT(stateChanged(QLlcpSocket::SocketState)));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

    switch (m_connectionType) {
    case StreamConnection:
        m_service = helloServer + streamSuffix;
        qDebug() << "Client connecting to" << m_service;
        m_socket->connectToService(0, m_service);
        break;
    case DatagramConnection:
        m_service = helloServer + datagramSuffix;
        qDebug() << "Client connecting to" << m_service;
        m_socket->connectToService(0, m_service);
        break;
    case BoundSocket:
        m_port = boundSocketPort;
        qDebug() << "Client binding to" << m_port;
        if (!m_socket->bind(m_port))
            qDebug() << "Failed to bind to port" << m_port;
        break;
    case ConnectionlessSocket:
        qDebug() << "Client binding to arbitrary port";
        if (!m_socket->bind(0)) {
            qDebug() << "Failed to bind to arbitrary port";
        } else {
            m_manager = new QNearFieldManager(this);
            connect(m_manager, SIGNAL(targetDetected(QNearFieldTarget*)),
                    this, SLOT(targetDetected(QNearFieldTarget*)));
            connect(m_manager, SIGNAL(targetLost(QNearFieldTarget*)),
                    this, SLOT(targetLost(QNearFieldTarget*)));
            m_manager->startTargetDetection();
        }
        break;
    default:
        qFatal("Unknown connection type");
    }
}

SocketController::~SocketController()
{
    delete m_socket;
}

void SocketController::connected()
{
    qDebug() << "Client connected";
    const QString data = QLatin1String("HELLO ") + m_service;
    switch (m_connectionType) {
    case StreamConnection:
        m_socket->write(data.toUtf8() + '\n');
        break;
    case DatagramConnection:
        m_socket->writeDatagram(data.toUtf8());
        break;
    default:
        ;
    }
}

void SocketController::disconnected()
{
    qDebug() << "Client disconnected, reconnecting";

    m_socket->connectToService(0, m_service);
}

void SocketController::error(QLlcpSocket::SocketError socketError)
{
    qDebug() << "Client got error:" << socketError;
}

void SocketController::stateChanged(QLlcpSocket::SocketState socketState)
{
    qDebug() << "Client state changed to" << socketState;
}

void SocketController::readyRead()
{
    switch (m_connectionType) {
    case StreamConnection:
        while (m_socket->canReadLine()) {
            const QByteArray line = m_socket->readLine().trimmed();

            qDebug() << "Client read line:" << line;

            if (line == "DISCONNECT") {
                m_socket->disconnectFromService();
                break;
            } else if (line == "CLOSE") {
                m_socket->close();
                break;
            }
        }
        break;
    case DatagramConnection:
        while (m_socket->hasPendingDatagrams()) {
            qint64 size = m_socket->pendingDatagramSize();
            QByteArray data;
            data.resize(size);
            m_socket->readDatagram(data.data(), data.size());

            if (data == "DISCONNECT") {
                m_socket->disconnectFromService();
                break;
            } else if (data == "CLOSE") {
                m_socket->close();
                break;
            }
        }
    case BoundSocket:
    case ConnectionlessSocket:
        while (m_socket->hasPendingDatagrams()) {
            qint64 size = m_socket->pendingDatagramSize();
            QByteArray data;
            data.resize(size);
            m_socket->readDatagram(data.data(), data.size());

            qDebug() << data;
        }
    }
}

void SocketController::targetDetected(QNearFieldTarget *target)
{
    Q_UNUSED(target);

    m_timerId = startTimer(500);
}

void SocketController::targetLost(QNearFieldTarget *target)
{
    Q_UNUSED(target);

    killTimer(m_timerId);
}

void SocketController::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    m_socket->writeDatagram("Test message", 12, 0, boundSocketPort);
}

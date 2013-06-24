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

#include "servercontroller.h"
#include "servicenames.h"

ServerController::ServerController(ConnectionType type, QObject *parent)
:   QObject(parent), m_server(new QLlcpServer(this)), m_socket(0), m_connectionType(type)
{
    connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));

    switch (m_connectionType) {
    case StreamConnection:
        m_service = commandServer + streamSuffix;
        break;
    case DatagramConnection:
        m_service = commandServer + datagramSuffix;
        break;
    }

    m_server->listen(m_service);

    if (m_server->isListening())
        qDebug() << "Server listening on" << m_service;
}

ServerController::~ServerController()
{
    delete m_socket;
    delete m_server;
}

void ServerController::newConnection()
{
    m_socket = m_server->nextPendingConnection();

    qDebug() << "Server got new connection";

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
    connect(m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(socketBytesWritten(qint64)));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
}

void ServerController::socketReadyRead()
{
    switch (m_connectionType) {
    case StreamConnection:
        while (m_socket->canReadLine()) {
            const QByteArray line = m_socket->readLine().trimmed();

            qDebug() << "Server read line:" << line;

            QByteArray command;
            QByteArray parameter;

            int index = line.indexOf(' ');
            if (index >= 0) {
                command = line.left(index);
                parameter = line.mid(index + 1);
            } else {
                command = line;
            }

            if (command == "ECHO") {
                m_socket->write(parameter + '\n');
            } else if (command == "DISCONNECT") {
                m_socket->disconnectFromService();
                break;
            } else if (command == "CLOSE") {
                m_socket->close();
                break;
            } else if (command == "URI") {
                m_socket->write(m_service.toLatin1());
                m_socket->write("\n");
            }
        }
        break;
    case DatagramConnection:
        while (m_socket->hasPendingDatagrams()) {
            qint64 size = m_socket->pendingDatagramSize();
            QByteArray data;
            data.resize(size);
            m_socket->readDatagram(data.data(), data.size());

            QByteArray command;
            QByteArray parameter;

            int index = data.indexOf(' ');
            if (index >= 0) {
                command = data.left(index);
                parameter = data.mid(index + 1);
            } else {
                command = data;
            }

            if (command == "ECHO") {
                m_socket->writeDatagram(parameter);
            } else if (command == "DISCONNECT") {
                m_socket->disconnectFromService();
                break;
            } else if (command == "CLOSE") {
                m_socket->close();
                break;
            } else if (command == "URI") {
                m_socket->writeDatagram(m_service.toLatin1());
            }
        }
        break;
    }
}

void ServerController::socketBytesWritten(qint64 bytes)
{
    Q_UNUSED(bytes);
}

void ServerController::socketDisconnected()
{
    m_socket->deleteLater();
    m_socket = 0;
}

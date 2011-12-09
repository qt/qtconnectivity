/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#include "handover.h"

#include <qnearfieldmanager.h>
#include <qllcpserver.h>
#include <qllcpsocket.h>
#include <qbluetoothlocaldevice.h>

#include <QtCore/QStringList>

static const QLatin1String tennisUri("urn:nfc:sn:com.nokia.qtmobility.tennis");

Handover::Handover(quint16 serverPort, QObject *parent)
:   QObject(parent), m_server(new QLlcpServer(this)),
    m_client(new QLlcpSocket(this)), m_remote(0), m_serverPort(0), m_localServerPort(serverPort)
{
    connect(m_server, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
    m_server->listen(tennisUri);

    connect(m_client, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    connect(m_client, SIGNAL(readyRead()), this, SLOT(readBluetoothService()));

    m_client->connectToService(0, tennisUri);
}

Handover::~Handover()
{
}

QBluetoothAddress Handover::bluetoothAddress() const
{
    return m_address;
}

quint16 Handover::serverPort() const
{
    return m_serverPort;
}

void Handover::handleNewConnection()
{
    if (m_remote)
        m_remote->deleteLater();

    m_remote = m_server->nextPendingConnection();

    connect(m_remote, SIGNAL(disconnected()), this, SLOT(remoteDisconnected()));

    sendBluetoothService();
}

void Handover::remoteDisconnected()
{
    m_remote->deleteLater();
    m_remote = 0;
}

void Handover::clientDisconnected()
{
    m_client->connectToService(0, tennisUri);
}

void Handover::readBluetoothService()
{
    QByteArray rawData = m_client->readAll();
    QString data = QString::fromUtf8(rawData.constData(), rawData.size());
    QStringList split = data.split(QLatin1Char(' '));

    QBluetoothAddress address = QBluetoothAddress(split.at(0));
    quint16 port = split.at(1).toUInt();

    QBluetoothLocalDevice localDevice;
    QBluetoothAddress localAddress = localDevice.address();

    if (localAddress < address) {
        m_address = address;
        m_serverPort = port;
        emit bluetoothServiceChanged();
    }
}

void Handover::sendBluetoothService()
{
    QBluetoothLocalDevice localDevice;
    const QString data = localDevice.address().toString() + QLatin1Char(' ') +
                         QString::number(m_localServerPort);

    m_remote->write(data.toUtf8());
}


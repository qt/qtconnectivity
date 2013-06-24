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

#include "handover.h"

#include <qnearfieldmanager.h>
#include <qllcpserver.h>
#include <qllcpsocket.h>
#include <qbluetoothlocaldevice.h>

#include <QtCore/QStringList>

static const QLatin1String tennisUri("urn:nfc:sn:com.nokia.qtnfc.tennis");

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


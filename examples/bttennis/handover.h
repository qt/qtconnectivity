/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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

#ifndef HANDOVER_H
#define HANDOVER_H

#include <QtCore/QObject>

#include <qbluetoothaddress.h>
#include <qbluetoothuuid.h>
#include <qnfcglobal.h>

QTNFC_BEGIN_NAMESPACE
class QNearFieldManager;
class QNearFieldTarget;
class QLlcpServer;
class QLlcpSocket;
QTNFC_END_NAMESPACE

QTNFC_USE_NAMESPACE
QTBLUETOOTH_USE_NAMESPACE

class Handover : public QObject
{
    Q_OBJECT

public:
    explicit Handover(quint16 serverPort, QObject *parent = 0);
    ~Handover();

    QBluetoothAddress bluetoothAddress() const;
    quint16 serverPort() const;

private slots:
    void handleNewConnection();
    void remoteDisconnected();

    void clientDisconnected();

    void readBluetoothService();
    void sendBluetoothService();

signals:
    void bluetoothServiceChanged();

private:
    QLlcpServer *m_server;
    QLlcpSocket *m_client;
    QLlcpSocket *m_remote;

    QBluetoothAddress m_address;
    quint16 m_serverPort;
    quint16 m_localServerPort;
};

#endif // HANDOVER_H

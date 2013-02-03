/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#ifndef QRFCOMMSERVER_P_H
#define QRFCOMMSERVER_P_H

#include <QtGlobal>
#include <QList>
#include <qbluetoothsocket.h>
#include "qbluetooth.h"

#ifdef QTM_QNX_BLUETOOTH
#include "qnx/ppshelpers_p.h"
#endif

#ifdef QT_BLUEZ_BLUETOOTH
QT_FORWARD_DECLARE_CLASS(QSocketNotifier)
#endif

QT_BEGIN_NAMESPACE_BLUETOOTH

class QBluetoothAddress;
class QBluetoothSocket;

class QRfcommServer;

class QRfcommServerPrivate
#ifdef QTM_QNX_BLUETOOTH
: public QObject
{
    Q_OBJECT
#else
{
#endif
    Q_DECLARE_PUBLIC(QRfcommServer)

public:
    QRfcommServerPrivate();
    ~QRfcommServerPrivate();

#ifdef QT_BLUEZ_BLUETOOTH
    void _q_newConnection();
#endif

public:
    QBluetoothSocket *socket;

    int maxPendingConnections;
    QBluetooth::SecurityFlags securityFlags;

#ifdef QTM_QNX_BLUETOOTH
    QList<QBluetoothSocket *> activeSockets;
#endif

protected:
    QRfcommServer *q_ptr;

private:
#ifdef QTM_QNX_BLUETOOTH
    QBluetoothUuid m_uuid;
    bool serverRegistered;
    QString nextClientAddress;

private Q_SLOTS:
    void controlReply(ppsResult result);
    void controlEvent(ppsResult result);
#elif defined(QT_BLUEZ_BLUETOOTH)
    QSocketNotifier *socketNotifier;
#endif
};

QT_END_NAMESPACE_BLUETOOTH

#endif

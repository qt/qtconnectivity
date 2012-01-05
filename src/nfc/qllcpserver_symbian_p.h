/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtNfc module of the Qt Toolkit.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QLLCPSERVER_SYMBIAN_P_H
#define QLLCPSERVER_SYMBIAN_P_H

#include <qconnectivityglobal.h>
#include "qllcpserver.h"
#include <QList>
#include <QPointer>

class CLlcpServer;
class CLlcpSocketType2;
class QLlcpSocket;

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

class QLlcpServerPrivate
{
    Q_DECLARE_PUBLIC(QLlcpServer)
public:
    QLlcpServerPrivate(QLlcpServer *q);
    ~QLlcpServerPrivate();

    bool listen(const QString &serviceUri);
    bool isListening() const;

    void close();

    QString serviceUri() const;
    quint8 serverPort() const;

    bool hasPendingConnections() const;
    QLlcpSocket *nextPendingConnection();
    QLlcpSocket::SocketError serverError() const;

public:
    void invokeNewConnection();

private:
    CLlcpServer* m_symbianbackend;
    QLlcpServer *q_ptr;
    QList<QPointer<QLlcpSocket> > m_pendingConnections;
};

QTNFC_END_NAMESPACE

QT_END_HEADER

#endif // QLLCPSERVER_P_H

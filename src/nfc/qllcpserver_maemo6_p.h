/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef QLLCPSERVER_MAEMO6_P_H
#define QLLCPSERVER_MAEMO6_P_H

#include <qconnectivityglobal.h>

#include "qllcpserver.h"

#include <QtDBus/QDBusConnection>

QT_FORWARD_DECLARE_CLASS(QDBusObjectPath)
QT_FORWARD_DECLARE_CLASS(QDBusVariant)

class AccessRequestorAdaptor;
class LLCPRequestorAdaptor;

QTNFC_BEGIN_NAMESPACE

class SocketRequestor;

class QLlcpServerPrivate : public QObject
{
    Q_OBJECT

    Q_DECLARE_PUBLIC(QLlcpServer)

public:
    QLlcpServerPrivate(QLlcpServer *q);

    bool listen(const QString &serviceUri);
    bool isListening() const;

    void close();

    QString serviceUri() const;
    quint8 serverPort() const;

    bool hasPendingConnections() const;
    QLlcpSocket *nextPendingConnection();

    QLlcpSocket::SocketError serverError() const;

private slots:
    // com.nokia.nfc.AccessRequestor
    void AccessFailed(const QDBusObjectPath &targetPath, const QString &kind,
                      const QString &error);
    void AccessGranted(const QDBusObjectPath &targetPath, const QString &accessKind);

    // com.nokia.nfc.LLCPRequestor
    void Accept(const QDBusVariant &lsap, const QDBusVariant &rsap, int fd,
                const QVariantMap &properties);
    void Connect(const QDBusVariant &lsap, const QDBusVariant &rsap, int fd,
                 const QVariantMap &properties);
    void Socket(const QDBusVariant &lsap, int fd, const QVariantMap &properties);

private:
    QLlcpServer *q_ptr;

    QDBusConnection m_connection;

    QString m_serviceUri;

    QString m_requestorPath;
    SocketRequestor *m_socketRequestor;

    QList<QPair<int, QVariantMap> > m_pendingSockets;

    QLlcpSocket::SocketError m_error;
};

QTNFC_END_NAMESPACE

#endif // QLLCPSERVER_MAEMO6_P_H

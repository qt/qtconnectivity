/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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

#include "qllcpserver_maemo6_p.h"

#include "qllcpsocket_maemo6_p.h"
#include "maemo6/manager_interface.h"
#include "maemo6/adapter_interface_p.h"
#include "maemo6/accessrequestor_adaptor.h"
#include "maemo6/llcprequestor_adaptor.h"

using namespace com::nokia::nfc;

QT_BEGIN_NAMESPACE_NFC

static QAtomicInt requestorId = 0;
static const char * const requestorBasePath = "/com/nokia/nfc/llcpserver/";

QLlcpServerPrivate::QLlcpServerPrivate(QLlcpServer *q)
:   q_ptr(q),
    m_connection(QDBusConnection::connectToBus(QDBusConnection::SystemBus, QUuid::createUuid().toString())),
    m_accessAgent(0), m_llcpAgent(0)
{
}

bool QLlcpServerPrivate::listen(const QString &serviceUri)
{
    if (m_requestorPath.isEmpty()) {
        m_requestorPath = QLatin1String(requestorBasePath) +
                          QString::number(requestorId.fetchAndAddOrdered(1));
    }

    Manager manager(QLatin1String("com.nokia.nfc"), QLatin1String("/"), m_connection);
    QDBusObjectPath defaultAdapterPath = manager.DefaultAdapter();
    m_adapter = new Adapter(QStringLiteral("com.nokia.nfc"), defaultAdapterPath.path(),
                            m_connection, this);

    if (!m_adapter) {
        Manager manager(QStringLiteral("com.nokia.nfc"), QStringLiteral("/"), m_connection);
        QDBusObjectPath defaultAdapterPath = manager.DefaultAdapter();
        m_adapter = new Adapter(QStringLiteral("com.nokia.nfc"), defaultAdapterPath.path(),
                                m_connection, this);
    }


    if (!m_accessAgent) {
        m_accessAgent = new AccessRequestorAdaptor(this);
        if (!m_connection.registerObject(m_requestorPath, this)) {
            delete m_accessAgent;
            m_accessAgent = 0;
            m_error = QLlcpSocket::SocketResourceError;
            m_serviceUri.clear();
            return false;
        }
    }

    if (!m_llcpAgent)
        m_llcpAgent = new LLCPRequestorAdaptor(this);

    m_serviceUri = serviceUri;

    QString accessKind(QLatin1String("device.llcp.co.server:") + serviceUri);
    m_adapter->RequestAccess(QDBusObjectPath(m_requestorPath), accessKind);

    return true;
}

bool QLlcpServerPrivate::isListening() const
{
    return !m_serviceUri.isEmpty();
}

void QLlcpServerPrivate::close()
{
    QString accessKind(QLatin1String("device.llcp.co.server:") + m_serviceUri);

    m_adapter->CancelAccessRequest(QDBusObjectPath(m_requestorPath), accessKind);

    m_serviceUri.clear();
}

QString QLlcpServerPrivate::serviceUri() const
{
    return m_serviceUri;
}

quint8 QLlcpServerPrivate::serverPort() const
{
    return 0;
}

bool QLlcpServerPrivate::hasPendingConnections() const
{
    return !m_pendingSockets.isEmpty();
}

QLlcpSocket *QLlcpServerPrivate::nextPendingConnection()
{
    if (m_pendingSockets.isEmpty())
        return 0;

    QPair<int, QVariantMap> parameters = m_pendingSockets.takeFirst();

    QLlcpSocketPrivate *socketPrivate =
        new QLlcpSocketPrivate(m_connection, parameters.first, parameters.second);

    QLlcpSocket *socket = new QLlcpSocket(socketPrivate, 0);

    return socket;
}

QLlcpSocket::SocketError QLlcpServerPrivate::serverError() const
{
    return QLlcpSocket::UnknownSocketError;
}

void QLlcpServerPrivate::AccessFailed(const QDBusObjectPath &targetPath, const QString &kind,
                                      const QString &error)
{
    Q_UNUSED(targetPath);
    Q_UNUSED(kind);
    Q_UNUSED(error);

    m_serviceUri.clear();

    m_error = QLlcpSocket::SocketAccessError;
}

void QLlcpServerPrivate::AccessGranted(const QDBusObjectPath &targetPath,
                                       const QString &accessKind)
{
    Q_UNUSED(targetPath);
    Q_UNUSED(accessKind);
}

void QLlcpServerPrivate::Accept(const QDBusVariant &lsap, const QDBusVariant &rsap,
                                int fd, const QVariantMap &properties)
{
    Q_UNUSED(lsap);
    Q_UNUSED(rsap);
    Q_UNUSED(properties);

    Q_Q(QLlcpServer);

    m_pendingSockets.append(qMakePair(fd, properties));

    emit q->newConnection();
}

void QLlcpServerPrivate::Connect(const QDBusVariant &lsap, const QDBusVariant &rsap,
                                 int readFd, const QVariantMap &properties)
{
    Q_UNUSED(lsap);
    Q_UNUSED(rsap);
    Q_UNUSED(readFd);
    Q_UNUSED(properties);
}

void QLlcpServerPrivate::Socket(const QDBusVariant &lsap, int fd, const QVariantMap &properties)
{
    Q_UNUSED(lsap);
    Q_UNUSED(fd);
    Q_UNUSED(properties);
}

QT_END_NAMESPACE_NFC

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

#include "qllcpserver_symbian_p.h"
#include "symbian/llcpserver_symbian.h"
#include "symbian/llcpsockettype2_symbian.h"
#include "symbian/nearfieldutility_symbian.h"

#include "symbian/debug.h"

QTNFC_BEGIN_NAMESPACE

QLlcpServerPrivate::QLlcpServerPrivate(QLlcpServer *q)
    :q_ptr(q)
{
    BEGIN
    QT_TRAP_THROWING(m_symbianbackend = CLlcpServer::NewL(*this));
    END
}
QLlcpServerPrivate::~QLlcpServerPrivate()
{
    BEGIN
    close();
    delete m_symbianbackend;
    END
}

bool QLlcpServerPrivate::listen(const QString &serviceUri)
{
    BEGIN
    LOG(serviceUri);
    HBufC8* uri = NULL;
    TRAPD(err, uri = QNFCNdefUtility::QString2HBufC8L(serviceUri));
    if(err != KErrNone)
        {
        return false;
        }
    bool ret =  m_symbianbackend->Listen(*uri);

    delete uri;
    END
    return ret;
}

bool QLlcpServerPrivate::isListening() const
{
    BEGIN
    END
    return m_symbianbackend->isListening();
}

/*!
    Stops listening for incoming connections.
*/
void QLlcpServerPrivate::close()
{
    BEGIN
    m_symbianbackend->StopListening();
    qDeleteAll(m_pendingConnections);
    m_pendingConnections.clear();
    END
}

QString QLlcpServerPrivate::serviceUri() const
{
    BEGIN
    const TDesC8& uri= m_symbianbackend->serviceUri();

    QString ret = QNFCNdefUtility::TDesC82QStringL(uri);
    LOG("QLlcpServerPrivate::serviceUri() ret="<<ret);
    END
    return ret;

}

quint8 QLlcpServerPrivate::serverPort() const
{
    BEGIN
    END
    return 0;
}

bool QLlcpServerPrivate::hasPendingConnections() const
{
    BEGIN
    END
    return m_symbianbackend->hasPendingConnections();
}

void QLlcpServerPrivate::invokeNewConnection()
{
    BEGIN
    Q_Q(QLlcpServer);
    emit q->newConnection();
    END
}

QLlcpSocket *QLlcpServerPrivate::nextPendingConnection()
{
    BEGIN
    QLlcpSocket* qSocket  = NULL;
    CLlcpSocketType2* socket_symbian = m_symbianbackend->nextPendingConnection();
    if (socket_symbian)
    {
        QLlcpSocketPrivate *qSocket_p = new QLlcpSocketPrivate(socket_symbian);
        qSocket = new QLlcpSocket(qSocket_p,NULL);
        qSocket_p->attachCallbackHandler(qSocket);
        socket_symbian->AttachCallbackHandler(qSocket_p);
        QPointer<QLlcpSocket> p(qSocket);
        m_pendingConnections.append(p);
    }
    END
    return qSocket;
}

QLlcpSocket::SocketError QLlcpServerPrivate::serverError() const
{
    BEGIN
    END
    return QLlcpSocket::UnknownSocketError;
}

QTNFC_END_NAMESPACE

//EOF


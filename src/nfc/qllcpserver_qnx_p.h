/***************************************************************************
**
** Copyright (C) 2013 Research In Motion
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

#ifndef QLLCPSERVER_QNX_P_H
#define QLLCPSERVER_QNX_P_H

#include "qllcpserver.h"
#include "nfc/nfc.h"

QTNFC_BEGIN_NAMESPACE

class QLlcpServerPrivate : public QObject
{
    Q_OBJECT
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

    Q_INVOKABLE void connected(nfc_target_t *);

private:
    QLlcpServer *q_ptr;
    QLlcpSocket *m_llcpSocket;
    //We can not use m_conListener for the connection state
    bool m_connected;
    nfc_llcp_connection_listener_t m_conListener;
    QString m_serviceUri;
    nfc_target_t *m_target;
};

QTNFC_END_NAMESPACE

#endif // QLLCPSERVER_QNX_P_H

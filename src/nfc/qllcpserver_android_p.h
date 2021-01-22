/****************************************************************************
**
** Copyright (C) 2016 Centria research and development
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef QLLCPSERVER_ANDROID_P_H
#define QLLCPSERVER_ANDROID_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qllcpserver_p.h"
//#include "nfc/nfc.h"

QT_BEGIN_NAMESPACE

class QLlcpServerPrivate : public QObject
{
    Q_OBJECT
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

    //Q_INVOKABLE void connected(nfc_target_t *);

private:
    QLlcpServer *q_ptr;
    QLlcpSocket *m_llcpSocket;
    //We can not use m_conListener for the connection state
    bool m_connected;
    //nfc_llcp_connection_listener_t m_conListener;
    QString m_serviceUri;
    //nfc_target_t *m_target;
};

QT_END_NAMESPACE

#endif // QLLCPSERVER_ANDROID_P_H

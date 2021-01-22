/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#ifndef QLLCPSERVER_H
#define QLLCPSERVER_H

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

#include <QtCore/QObject>
#include <QtNfc/qtnfcglobal.h>
#include "qllcpsocket_p.h"

QT_BEGIN_NAMESPACE

class QLlcpServerPrivate;

class Q_NFC_EXPORT QLlcpServer : public QObject
{
    Q_OBJECT

    Q_DECLARE_PRIVATE(QLlcpServer)

public:
    explicit QLlcpServer(QObject *parent = 0);
    virtual ~QLlcpServer();

    bool listen(const QString &serviceUri);
    bool isListening() const;

    void close();

    QString serviceUri() const;
    quint8 serverPort() const;

    virtual bool hasPendingConnections() const;
    virtual QLlcpSocket *nextPendingConnection();

    QLlcpSocket::SocketError serverError() const;

signals:
    void newConnection();

private:
    QLlcpServerPrivate *d_ptr;
};

QT_END_NAMESPACE

#endif // QLLCPSERVER_H

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

#include "qllcpserver_p_p.h"

QT_BEGIN_NAMESPACE

QLlcpServerPrivate::QLlcpServerPrivate(QLlcpServer *q)
:   q_ptr(q)
{
}

QLlcpServerPrivate::~QLlcpServerPrivate()
{
}

bool QLlcpServerPrivate::listen(const QString &serviceUri)
{
    Q_UNUSED(serviceUri);

    return false;
}

bool QLlcpServerPrivate::isListening() const
{
    return false;
}

void QLlcpServerPrivate::close()
{
}

QString QLlcpServerPrivate::serviceUri() const
{
    return QString();
}

quint8 QLlcpServerPrivate::serverPort() const
{
    return 0;
}

bool QLlcpServerPrivate::hasPendingConnections() const
{
    return false;
}

QLlcpSocket *QLlcpServerPrivate::nextPendingConnection()
{
    return 0;
}

QLlcpSocket::SocketError QLlcpServerPrivate::serverError() const
{
    return QLlcpSocket::UnknownSocketError;
}

QT_END_NAMESPACE

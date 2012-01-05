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

#ifndef QDECLARATIVENEARFIELDSOCKET_P_H
#define QDECLARATIVENEARFIELDSOCKET_P_H

#include <QtCore/QObject>
#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative/QDeclarativeParserStatus>

#include <qllcpsocket.h>

QTNFC_USE_NAMESPACE

class QDeclarativeNearFieldSocketPrivate;

class QDeclarativeNearFieldSocket : public QObject, public QDeclarativeParserStatus
{
    Q_OBJECT

    Q_PROPERTY(QString uri READ uri WRITE setUri NOTIFY uriChanged)
    Q_PROPERTY(bool connected READ connected WRITE setConnected NOTIFY connectedChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool listening READ listening WRITE setListening NOTIFY listeningChanged)
    Q_PROPERTY(QString stringData READ stringData WRITE sendStringData NOTIFY dataAvailable)

    Q_INTERFACES(QDeclarativeParserStatus)

    Q_DECLARE_PRIVATE(QDeclarativeNearFieldSocket)

public:
    explicit QDeclarativeNearFieldSocket(QObject *parent = 0);
    ~QDeclarativeNearFieldSocket();

    QString uri() const;
    bool connected() const;
    QString error() const;
    QString state() const;
    bool listening() const;

    QString stringData();

    // From QDeclarativeParserStatus
    void classBegin() {}
    void componentComplete();

signals:
    void uriChanged();
    void connectedChanged();
    void errorChanged();
    void stateChanged();
    void listeningChanged();
    void dataAvailable();

public slots:
    void setUri(const QString &service);
    void setConnected(bool connected);
    void setListening(bool listen);
    void sendStringData(const QString &data);

private slots:
    void socket_connected();
    void socket_disconnected();
    void socket_error(QLlcpSocket::SocketError);
    void socket_state(QLlcpSocket::SocketState);
    void socket_readyRead();

    void llcp_connection();

private:
    QDeclarativeNearFieldSocketPrivate *d_ptr;
};

QML_DECLARE_TYPE(QDeclarativeNearFieldSocket)

#endif // QDECLARATIVENEARFIELDSOCKET_P_H

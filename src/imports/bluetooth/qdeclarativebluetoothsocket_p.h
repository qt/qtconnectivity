/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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

#ifndef QDECLARATIVEBLUETOOTHSOCKET_P_H
#define QDECLARATIVEBLUETOOTHSOCKET_P_H

#include <QObject>
#include <qqml.h>
#include <QQmlParserStatus>

#include <QtBluetooth/QBluetoothSocket>

#include "qdeclarativebluetoothservice_p.h"

QT_USE_NAMESPACE

class QDeclarativeBluetoothSocketPrivate;
class QDeclarativeBluetoothService;

class QDeclarativeBluetoothSocket : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QDeclarativeBluetoothService *service READ service WRITE setService NOTIFY serviceChanged)
    Q_PROPERTY(bool connected READ connected WRITE setConnected NOTIFY connectedChanged)
    Q_PROPERTY(Error error READ error NOTIFY errorChanged)
    Q_PROPERTY(SocketState socketState READ state NOTIFY stateChanged)
    Q_PROPERTY(QString stringData READ stringData WRITE sendStringData NOTIFY dataAvailable)
    Q_INTERFACES(QQmlParserStatus)
    Q_ENUMS(Error)
    Q_ENUMS(SocketState)

public:

    enum Error {
        NoError = QBluetoothSocket::NoSocketError,
        UnknownSocketError = QBluetoothSocket::UnknownSocketError,
        HostNotFoundError = QBluetoothSocket::HostNotFoundError,
        ServiceNotFoundError = QBluetoothSocket::ServiceNotFoundError,
        NetworkError = QBluetoothSocket::NetworkError,
        UnsupportedProtocolError = QBluetoothSocket::UnsupportedProtocolError
    };

    enum SocketState {
        Unconnected = QBluetoothSocket::UnconnectedState,
        ServiceLookup = QBluetoothSocket::ServiceLookupState,
        Connecting = QBluetoothSocket::ConnectingState,
        Connected = QBluetoothSocket::ConnectedState,
        Bound = QBluetoothSocket::BoundState,
        Closing = QBluetoothSocket::ClosingState,
        Listening = QBluetoothSocket::ListeningState,
        NoServiceSet = 100 //Leave gap for future enums and to avoid collision with QBluetoothSocket enums
    };

    explicit QDeclarativeBluetoothSocket(QObject *parent = 0);
    explicit QDeclarativeBluetoothSocket(QDeclarativeBluetoothService *service,
                                         QObject *parent = 0);
    explicit QDeclarativeBluetoothSocket(QBluetoothSocket *socket, QDeclarativeBluetoothService *service,
                                         QObject *paprent = 0);
    ~QDeclarativeBluetoothSocket();

    QDeclarativeBluetoothService *service();
    bool connected() const;
    Error error() const;
    SocketState state() const;

    QString stringData();

    // From QDeclarativeParserStatus
    void classBegin() {}
    void componentComplete();

    void newSocket(QBluetoothSocket *socket, QDeclarativeBluetoothService *service);

signals:
    void serviceChanged();
    void connectedChanged();
    void errorChanged();
    void stateChanged();
    void dataAvailable();

public slots:
    void setService(QDeclarativeBluetoothService *service);
    void setConnected(bool connected);
    void sendStringData(const QString& data);

private slots:
    void socket_connected();
    void socket_disconnected();
    void socket_error(QBluetoothSocket::SocketError);
    void socket_state(QBluetoothSocket::SocketState);
    void socket_readyRead();

private:
    QDeclarativeBluetoothSocketPrivate* d;
    friend class QDeclarativeBluetoothSocketPrivate;

};

QML_DECLARE_TYPE(QDeclarativeBluetoothSocket)

#endif // QDECLARATIVEBLUETOOTHSOCKET_P_H

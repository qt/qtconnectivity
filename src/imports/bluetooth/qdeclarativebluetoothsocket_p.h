/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDECLARATIVEBLUETOOTHSOCKET_P_H
#define QDECLARATIVEBLUETOOTHSOCKET_P_H

#include <QObject>
#include <qdeclarative.h>
#include <QDeclarativeParserStatus>

#include <qbluetoothsocket.h>

#include "qdeclarativebluetoothservice_p.h"

QTCONNECTIVITY_USE_NAMESPACE

class QDeclarativeBluetoothSocketPrivate;
class QDeclarativeBluetoothService;

class QDeclarativeBluetoothSocket : public QObject, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QDeclarativeBluetoothService *service READ service WRITE setService NOTIFY serviceChanged)
    Q_PROPERTY(bool connected READ connected WRITE setConnected NOTIFY connectedChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString stringData READ stringData WRITE sendStringData NOTIFY dataAvailable)
    Q_INTERFACES(QDeclarativeParserStatus)

public:
    explicit QDeclarativeBluetoothSocket(QObject *parent = 0);
    explicit QDeclarativeBluetoothSocket(QDeclarativeBluetoothService *service,
                                         QObject *parent = 0);
    explicit QDeclarativeBluetoothSocket(QBluetoothSocket *socket, QDeclarativeBluetoothService *service,
                                         QObject *paprent = 0);
    ~QDeclarativeBluetoothSocket();

    QDeclarativeBluetoothService *service();
    bool connected();
    QString error();
    QString state();

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
    void sendStringData(QString data);

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

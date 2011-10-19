/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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

#ifndef QLLCPSOCKETSTATE_P_H
#define QLLCPSOCKETSTATE_P_H

#include <qconnectivityglobal.h>
#include "qllcpsocket.h"

#include <QtCore/QObject>

QT_BEGIN_HEADER

QTCONNECTIVITY_BEGIN_NAMESPACE

class QLlcpSocketPrivate;

/*!
    \QLLCPSocketState
*/
class QLLCPSocketState
{
public:
    explicit QLLCPSocketState(QLlcpSocketPrivate*);

public:
    virtual QLlcpSocket::SocketState state() const = 0;
    virtual bool WaitForBytesWritten(int) = 0;

public:
    virtual qint64 ReadDatagram(char *data, qint64 maxSize,
                        QNearFieldTarget **target = NULL, quint8 *port = NULL);
    virtual qint64 WriteDatagram(const char *data, qint64 size,
                                 QNearFieldTarget *target, quint8 port);
    virtual qint64 WriteDatagram(const char *data, qint64 size);

    virtual bool Bind(quint8 port);
    virtual void ConnectToService(QNearFieldTarget *target, const QString &serviceUri);
    virtual void DisconnectFromService();
    virtual bool WaitForReadyRead(int);
    virtual bool WaitForConnected(int);
    virtual bool WaitForDisconnected(int);

protected:
    bool WaitForBytesWrittenType1Private(int);
    void DisconnectFromServiceType2Private();
public:
    QLlcpSocketPrivate* m_socket;
};

/*!
    \QLLCPUnconnected
*/
class QLLCPUnconnected: public QLLCPSocketState
{
public:
    explicit QLLCPUnconnected(QLlcpSocketPrivate*);

public: // from base class
    QLlcpSocket::SocketState state() const {return QLlcpSocket::UnconnectedState;}
    bool Bind(quint8 port);
    void ConnectToService(QNearFieldTarget *target, const QString &serviceUri);
    void DisconnectFromService();
    qint64 WriteDatagram(const char *data, qint64 size,
                         QNearFieldTarget *target, quint8 port);
    bool WaitForBytesWritten(int);
    bool WaitForDisconnected(int);

public:
public:
    enum SocketType
    {
       connectionType1 = 1, // ConnectionLess mode
       connectionType2 = 2, // ConnectionOriented mode
       connectionUnknown = -1
    };
    SocketType m_socketType;
};

/*!
    \QLLCPConnecting
*/
class QLLCPConnecting: public QLLCPSocketState
{
public:
    explicit QLLCPConnecting(QLlcpSocketPrivate*);

public:
     QLLCPSocketState* Instance(QLlcpSocketPrivate* aSocket);

public: // from base class
    QLlcpSocket::SocketState state() const {return QLlcpSocket::ConnectingState;}
    void ConnectToService(QNearFieldTarget *target, const QString &serviceUri);
    void DisconnectFromService();
    bool WaitForConnected(int);
    bool WaitForBytesWritten(int);
};

/*!
    \QLLCPConnecting
*/
class QLLCPConnected: public QLLCPSocketState
{
public:
    explicit QLLCPConnected(QLlcpSocketPrivate*);

public: // from base class
    QLlcpSocket::SocketState state() const {return QLlcpSocket::ConnectedState;}
    void ConnectToService(QNearFieldTarget *target, const QString &serviceUri);
    void DisconnectFromService();
    qint64 WriteDatagram(const char *data, qint64 size);
    qint64 ReadDatagram(char *data, qint64 maxSize, QNearFieldTarget **target = NULL, quint8 *port = NULL);
    bool WaitForBytesWritten(int msecs);
    bool WaitForReadyRead(int msecs);
};

/*!
    \QLLCPBind
*/
class QLLCPBind: public QLLCPSocketState
    {
public:
    explicit QLLCPBind(QLlcpSocketPrivate*);

public://from base class
    QLlcpSocket::SocketState state() const {return QLlcpSocket::BoundState;}
    qint64 WriteDatagram(const char *data, qint64 size,QNearFieldTarget *target, quint8 port);
    qint64 ReadDatagram(char *data, qint64 maxSize,QNearFieldTarget **target = 0, quint8 *port = 0);
    bool WaitForBytesWritten(int msecs);
    bool WaitForReadyRead(int msecs);
    };

QTCONNECTIVITY_END_NAMESPACE

QT_END_HEADER

#endif //QLLCPSTATE_P_H

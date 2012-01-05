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

#include "qllcpsocket_symbian_p.h"
#include "qllcpstate_symbian_p.h"
#include "symbian/llcpsockettype1_symbian.h"
#include "symbian/llcpsockettype2_symbian.h"
#include "symbian/nearfieldutility_symbian.h"
#include "symbian/debug.h"

QTNFC_BEGIN_NAMESPACE

/*!
    Constructor
*/
QLlcpSocketPrivate::QLlcpSocketPrivate(QLlcpSocket *q)
       : m_symbianSocketType1(NULL),
         m_symbianSocketType2(NULL),
         m_error(QLlcpSocket::UnknownSocketError),
         q_ptr(q),
         m_emittedReadyRead(false),
         m_emittedBytesWritten(false),
         m_writeDatagramRefCount(0)
{
    BEGIN
    m_unconnectedState = new QLLCPUnconnected(this);
    m_connectingState = new QLLCPConnecting(this);
    m_connectedState = new QLLCPConnected(this);
    m_bindState = new QLLCPBind(this);

    m_state = m_unconnectedState;
    END
}


/*!
    Destructor
*/
QLlcpSocketPrivate::~QLlcpSocketPrivate()
{
    BEGIN
    Q_Q(QLlcpSocket);
    if (q->isOpen()) {
        q->close();
    }

    delete m_unconnectedState;
    delete m_connectingState;
    delete m_connectedState;
    delete m_bindState;
    delete m_symbianSocketType1;
    delete m_symbianSocketType2;
    END
}

/*!
    Construct the socket and set as connected state from llcp server side
*/
QLlcpSocketPrivate::QLlcpSocketPrivate(CLlcpSocketType2* socketType2_symbian)
    : m_symbianSocketType1(NULL),
      m_symbianSocketType2(socketType2_symbian),
      m_error(QLlcpSocket::UnknownSocketError),
      m_emittedReadyRead(false),
      m_emittedBytesWritten(false),
      m_writeDatagramRefCount(0)
{
    BEGIN
    m_unconnectedState = new QLLCPUnconnected(this);
    m_connectingState = new QLLCPConnecting(this);
    m_connectedState = new QLLCPConnected(this);
    m_bindState = new QLLCPBind(this);

    m_state = m_connectedState;

    END
}

void QLlcpSocketPrivate::connectToService(QNearFieldTarget *target, const QString &serviceUri)
{
    BEGIN
    Q_Q(QLlcpSocket);
    if (!q->isOpen())
    {
       bool ret = q->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
       if (false == ret)
           invokeError();
    }
    m_state->ConnectToService(target,serviceUri);
    END
}

void QLlcpSocketPrivate::disconnectFromService()
{
    BEGIN
    m_state->DisconnectFromService();

    Q_Q(QLlcpSocket);
    if (q->isOpen())
    {
        q->close();
    }
    END
}

void QLlcpSocketPrivate::invokeConnected()
{
    BEGIN
    Q_Q(QLlcpSocket);

    if (m_state->state() == QLlcpSocket::ConnectingState)
    {
      changeState(getConnectedState());
      invokeStateChanged(QLlcpSocket::ConnectedState);
      emit q->connected();
    }
    END
}

void QLlcpSocketPrivate::invokeReadyRead()
{
    BEGIN
    Q_Q(QLlcpSocket);
    //If called from within a slot connected to the readyRead() signal,
    //readyRead() will not be reemitted.
    LOG("m_emittedReadyRead: " << m_emittedReadyRead);

    if (!m_emittedReadyRead){
        m_emittedReadyRead = true;
        LOG("readyRead signal has been emitted!");
        emit q->readyRead();
        m_emittedReadyRead = false;
    }
    END
}

void QLlcpSocketPrivate::attachCallbackHandler(QLlcpSocket *q)
{
    BEGIN
    q_ptr = q;
    if (!q->isOpen())
    {
       bool ret = q->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
       if (false == ret)
          invokeError();
    }
    END
}

qint64 QLlcpSocketPrivate::bytesAvailable() const
{
    BEGIN
    qint64 bytes = 0;
    if (m_symbianSocketType2 != NULL)
    {
        bytes = m_symbianSocketType2->BytesAvailable();
    }
    END
    return bytes;
}

bool QLlcpSocketPrivate::canReadLine() const
{
    return false;
}

void QLlcpSocketPrivate::invokeBytesWritten(qint64 bytes)
{
    BEGIN
    Q_Q(QLlcpSocket);
    if (!m_emittedBytesWritten){
        m_emittedBytesWritten = true;
        emit q->bytesWritten(bytes);
        m_emittedBytesWritten = false;
    }
    END
}

void QLlcpSocketPrivate::invokeStateChanged(QLlcpSocket::SocketState socketState)
{
    BEGIN
    Q_Q(QLlcpSocket);
    emit q->stateChanged(socketState);
    END
}

void QLlcpSocketPrivate::invokeError()
{
    BEGIN
    Q_Q(QLlcpSocket);
    emit q->error(QLlcpSocket::UnknownSocketError);
    END
}

void QLlcpSocketPrivate::invokeDisconnected()
{
    BEGIN
    Q_Q(QLlcpSocket);
    emit q->disconnected();
    END
}

/*!
    Only used at connectless mode, create type1 socket if necessary.
*/
bool QLlcpSocketPrivate::bind(quint8 port)
{
    BEGIN_END
    return m_state->Bind(port);
}

/*!
    Returns true if at least one datagram is waiting to be read;
    otherwise returns false.
*/
bool QLlcpSocketPrivate::hasPendingDatagrams() const
{
    BEGIN
    bool val = false;
    if (m_symbianSocketType1)
    {
        val = m_symbianSocketType1->HasPendingDatagrams();
    }

    END
    return val;
}

/*!
    Returns the size of the first pending connectionless datagram. If there is
    no datagram available, this function returns -1.
*/
qint64 QLlcpSocketPrivate::pendingDatagramSize() const
{
    BEGIN
    qint64 val = -1;
    if (m_symbianSocketType1)
    {
        val = m_symbianSocketType1->PendingDatagramSize();
    }

    END
    return val;
}

CLlcpSocketType1* QLlcpSocketPrivate::socketType1Instance()
{
    if (NULL == m_symbianSocketType1)
    {
        TRAPD(err, m_symbianSocketType1 = CLlcpSocketType1::NewL(*this));
        if (KErrNone != err) {
           m_symbianSocketType1 = NULL;
        }
    }
    return m_symbianSocketType1;
}

CLlcpSocketType2* QLlcpSocketPrivate::socketType2Instance()
{
    if (NULL == m_symbianSocketType2)
    {
       TRAPD(err, m_symbianSocketType2 = CLlcpSocketType2::NewL(this));
       if (KErrNone != err) {
          m_symbianSocketType2 = NULL;
       }
    }
    return m_symbianSocketType2;
}

/*!
    Connection-Oriented Style
*/
qint64 QLlcpSocketPrivate::writeDatagram(const char *data, qint64 size)
{
    BEGIN
    qint64 val = -1;
    val = m_state->WriteDatagram(data,size);
    END
    return val;
}

/*!
    Connection-Oriented Style
*/
qint64 QLlcpSocketPrivate::writeDatagram(const QByteArray &datagram)
{
    BEGIN_END
    return writeDatagram(datagram.constData(),datagram.size());
}

/*!
    Used for Both Connection-Less & Connection-Oriented Style
    As for Connection-Oriented mode, can not pass in the port & target parameters.
*/
qint64 QLlcpSocketPrivate::readDatagram(char *data, qint64 maxSize,
                                        QNearFieldTarget **target, quint8 *port)
{
    BEGIN
    qint64 val = m_state->ReadDatagram(data,maxSize,target,port);
    END
    return val;
}

qint64 QLlcpSocketPrivate::writeDatagram(const char *data, qint64 size,
                                         QNearFieldTarget *target, quint8 port)
{
    BEGIN
    qint64 val = m_state->WriteDatagram(data,size,target,port);
    END

    return val;
}

qint64 QLlcpSocketPrivate::writeDatagram(const QByteArray &datagram,
                                         QNearFieldTarget *target, quint8 port)
{
    BEGIN_END
    return writeDatagram(datagram.constData(),datagram.size(),target,port);
}

QLlcpSocket::SocketError QLlcpSocketPrivate::error() const
{
    BEGIN_END
    return m_error;
}

QLlcpSocket::SocketState QLlcpSocketPrivate::state() const
{
    BEGIN_END
    return m_state->state();
}

qint64 QLlcpSocketPrivate::readData(char *data, qint64 maxlen)
{
    BEGIN_END
    return readDatagram(data,maxlen);
}

qint64 QLlcpSocketPrivate::writeData(const char *data, qint64 len)
{
    BEGIN_END
    return writeDatagram(data,len);
}

bool QLlcpSocketPrivate::waitForReadyRead(int msecs)
{
    BEGIN_END
    return m_state->WaitForReadyRead(msecs);
}

bool QLlcpSocketPrivate::waitForBytesWritten(int msecs)
{
    BEGIN_END
    return m_state->WaitForBytesWritten(msecs);
}

bool QLlcpSocketPrivate::waitForConnected(int msecs)
{
    BEGIN_END
    return m_state->WaitForConnected(msecs);
}

bool QLlcpSocketPrivate::waitForDisconnected(int msecs)
{
    BEGIN_END
    return m_state->WaitForDisconnected(msecs);
}

void QLlcpSocketPrivate::changeState(QLLCPSocketState* state)
{
    m_state = state;
    BEGIN_END
}

QTNFC_END_NAMESPACE

/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

QLLCPSocketState::QLLCPSocketState(QLlcpSocketPrivate* socket)
      :m_socket(socket)
{}

/*!
     Connection-Less Mode
*/
bool QLLCPUnconnected::Bind(quint8 port)
{
    BEGIN
    bool ret = false;
    if (connectionType2 == m_socketType) {
        return ret;
    }

    CLlcpSocketType1* socketHandler =  m_socket->socketType1Instance();
    if (socketHandler != NULL )
    {
        if (connectionUnknown == m_socketType)
        {
            m_socketType = connectionType1;
        }

        if (socketHandler->Bind(port))
        {
            ret = true;
            m_socket->changeState(m_socket->getBindState());
            m_socket->invokeStateChanged(QLlcpSocket::BoundState);
        }
    }
    END
    return ret;
}

/*!
    Connection-Less Mode
*/
bool QLLCPUnconnected::WaitForBytesWritten(int msecs)
{
   BEGIN
   bool ret = false;
   if (connectionType1 != m_socketType) {
       return ret;
   }

   ret = WaitForBytesWrittenType1Private(msecs);
   END
   return ret;
}

bool QLLCPUnconnected::WaitForDisconnected(int msecs)
{
    Q_UNUSED(msecs);
    BEGIN_END
    return true;
}

/*!
    Connection-Less Mode
*/
bool QLLCPBind::WaitForBytesWritten(int msecs)
{
   BEGIN
   bool ret = WaitForBytesWrittenType1Private(msecs);
   END
   return ret;
}

/*!
    Failitator function for Connection-Less Mode
*/
bool QLLCPSocketState::WaitForBytesWrittenType1Private(int msecs)
{
    bool ret = false;

    qDebug() <<  "m_writeDatagramRefCount: " << m_socket->m_writeDatagramRefCount;
    if (m_socket->m_writeDatagramRefCount <= 0)
    {
        return ret;
    }

    CLlcpSocketType1* socketHandler =  m_socket->socketType1Instance();
    if (socketHandler != NULL)
    {
        ret = socketHandler->WaitForBytesWritten(msecs);
    }
    return ret;
}

/*!
    Connection-Less Mode
*/
bool QLLCPBind::WaitForReadyRead(int msecs)
{
    BEGIN
    bool ret = false;
    CLlcpSocketType1* socketHandler = m_socket->socketType1Instance();
    if (socketHandler != NULL && socketHandler->HasPendingDatagrams())
    {
       ret = true;
    }
    END
    return ret;
}

/*!
    Connection-Less Mode
*/
qint64 QLLCPUnconnected::WriteDatagram(const char *data, qint64 size,
                                       QNearFieldTarget *target, quint8 port)
{
    BEGIN
    Q_UNUSED(target);
    qint64 val = -1;

    if (connectionType2 == m_socketType){
        return val;
    }

    CLlcpSocketType1* socketHandler =  m_socket->socketType1Instance();

    if (socketHandler != NULL)
    {
        if (connectionUnknown == m_socketType)
        {
          m_socketType = connectionType1;
        }
        TPtrC8 myDescriptor((const TUint8*)data, size);
        val = socketHandler->StartWriteDatagram(myDescriptor, port);
    }
    END

    return val;
}

/*!
    Connection-Less Mode
*/
qint64 QLLCPBind::WriteDatagram(const char *data, qint64 size,
                              QNearFieldTarget *target, quint8 port)
{
    BEGIN
    Q_UNUSED(target);
    qint64 val = -1;
    CLlcpSocketType1* socketHandler = m_socket->socketType1Instance();
    if (socketHandler != NULL)
    {
        TPtrC8 myDescriptor((const TUint8*)data, size);
        val = socketHandler->StartWriteDatagram(myDescriptor, port);
    }
    END
    return val;
}

/*!
    Connection-Oriented Mode - Client side socket write
*/
qint64 QLLCPConnected::WriteDatagram(const char *data, qint64 size)
{
    BEGIN
    qint64 val = -1;
    CLlcpSocketType2* socketHandler = m_socket->socketType2Handler();
    if (socketHandler != NULL)
    {
        TPtrC8 myDescriptor((const TUint8*)data, size);
        val = socketHandler->StartWriteDatagram(myDescriptor);
    }
    END
    return val;
}

/*!
    Connection-Oriented Mode
*/
bool QLLCPConnected::WaitForBytesWritten(int msecs)
{
   BEGIN
   bool ret = false;
   CLlcpSocketType2* socketHandler = m_socket->socketType2Handler();
   if (socketHandler != NULL)
   {
       ret = socketHandler->WaitForBytesWritten(msecs);
   }
   END
   return ret;
}

/*!
    Connection-Oriented Mode
*/
bool QLLCPConnected::WaitForReadyRead(int msecs)
{
    BEGIN
    bool ret = false;
    CLlcpSocketType2* socketHandler = m_socket->socketType2Handler();
    if (socketHandler != NULL)
    {
        ret = socketHandler->WaitForReadyRead(msecs);
    }
    END
    return ret;
}

/*!
    Connection-Less Mode
*/
qint64 QLLCPBind::ReadDatagram(char *data, qint64 maxSize,
                               QNearFieldTarget **target, quint8 *port)
{
    BEGIN

    Q_UNUSED(target);
    qint64 val = -1;

    CLlcpSocketType1* socketHandler = m_socket->socketType1Instance();

    if (socketHandler != NULL)
    {
        TPtr8 ptr((TUint8*)data, (TInt)maxSize);
        if (port == NULL){
           val = socketHandler->ReadDatagram(ptr);
        }
        else {
           val = socketHandler->ReadDatagram(ptr, *port);
        }
    }

    END
    return val;
}


/*!
    Connection-Oriented Mode - Client side socket read
*/
qint64 QLLCPConnected::ReadDatagram(char *data, qint64 maxSize,QNearFieldTarget **target, quint8 *port )
{
    BEGIN
    qint64 val = -1;

    if ( port != NULL || ( target != NULL && *target != NULL ))
    {
       return val;
    }

    CLlcpSocketType2* socketHandler = m_socket->socketType2Handler();
    if (socketHandler != NULL)
    {
        // The length of the descriptor is set to zero
        // and its maximum length is set to maxSize
        TPtr8 ptr((TUint8*)data, (TInt)maxSize);

        bool ret = socketHandler->ReceiveData(ptr);
        if(ret) {
            val = ptr.Length();
        }
    }

    END
    return val;
}

/*!
    Connection-Oriented Mode
*/
void QLLCPConnected::DisconnectFromService()
{
   BEGIN
   DisconnectFromServiceType2Private();
   END
   return;
}

/*!
    Connection-Oriented Mode
*/
void QLLCPConnecting::DisconnectFromService()
{
   BEGIN
   DisconnectFromServiceType2Private();
   END
   return;
}

void QLLCPSocketState::DisconnectFromServiceType2Private()
{
    BEGIN
    CLlcpSocketType2* socketHandler = m_socket->socketType2Handler();
    if (NULL != socketHandler)
    {
        if (socketHandler->DisconnectFromService() == KErrNone)
        {
           m_socket->changeState(m_socket->getUnconnectedState());
           m_socket->invokeStateChanged(QLlcpSocket::UnconnectedState);
        }
        else
        {
           m_socket->invokeError();
        }
    }
    else
    {
       m_socket->invokeError();
    }
    END
}

/*!
    Connection-Oriented Mode
*/
void QLLCPUnconnected::DisconnectFromService()
{
   BEGIN_END
   return;
}

/*!
    Connection-Oriented Mode
*/
bool QLLCPConnecting::WaitForConnected(int msecs)
{
    BEGIN
    bool ret = false;
    CLlcpSocketType2* socketHandler = m_socket->socketType2Handler();
    if (socketHandler != NULL)
    {
        ret = socketHandler->WaitForConnected(msecs);
    }
    END
    return ret;
}

/*!
    Connection-Oriented Mode
*/
bool QLLCPConnecting::WaitForBytesWritten(int)
{
   return false;
}

/*!
    Connection-Oriented Mode
*/
void QLLCPUnconnected::ConnectToService(QNearFieldTarget *target, const QString &serviceUri)
{
    BEGIN
    if (connectionType1 == m_socketType){
        m_socket->invokeError();
        END
        return;
    }

    CLlcpSocketType2* socketHandler =  m_socket->socketType2Instance();

    if (socketHandler != NULL)
    {
        if (connectionUnknown == m_socketType)
        {
           m_socketType = connectionType2;
        }

        TRAPD(err, socketHandler->ConnectToServiceL(serviceUri));
        if (KErrNone == err)
        {
            m_socket->changeState(m_socket->getConnectingState());
            m_socket->invokeStateChanged(QLlcpSocket::ConnectingState);
        }
        else
        {
            m_socket->invokeError();
        }

    }
    else
    {
       m_socket->invokeError();
    }

    END
}

/*!
    Connection-Oriented Mode
*/
void QLLCPConnecting::ConnectToService(QNearFieldTarget *target, const QString &serviceUri)
    {
    Q_UNUSED(target);
    Q_UNUSED(serviceUri);
    m_socket->invokeError();
    qWarning("QLlcpSocket::connectToService() called when already connecting");
    BEGIN_END
    return;
    }

/*!
    Connection-Oriented Mode
*/
void QLLCPConnected::ConnectToService(QNearFieldTarget *target, const QString &serviceUri)
    {
    Q_UNUSED(target);
    Q_UNUSED(serviceUri);
    m_socket->invokeError();
    qWarning("QLlcpSocket::connectToService() called when already connected");
    BEGIN_END
    return;
    }

/*!
    Constructors
*/
QLLCPUnconnected::QLLCPUnconnected(QLlcpSocketPrivate* aSocket)
    :QLLCPSocketState(aSocket),
     m_socketType(connectionUnknown)
{}


QLLCPBind::QLLCPBind(QLlcpSocketPrivate* aSocket)
    :QLLCPSocketState(aSocket)
{}

QLLCPConnecting::QLLCPConnecting(QLlcpSocketPrivate* aSocket)
    :QLLCPSocketState(aSocket)
{}

QLLCPConnected::QLLCPConnected(QLlcpSocketPrivate* aSocket)
    :QLLCPSocketState(aSocket)
{}

qint64 QLLCPSocketState::ReadDatagram(char *data, qint64 maxSize,
        QNearFieldTarget **target, quint8 *port)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);
    Q_UNUSED(target);
    Q_UNUSED(port);
    BEGIN_END
    return -1;
}

/*!
    State base class default implementation
*/
qint64 QLLCPSocketState::WriteDatagram(const char *data, qint64 size,
                                         QNearFieldTarget *target, quint8 port)
{
    Q_UNUSED(data);
    Q_UNUSED(size);
    Q_UNUSED(target);
    Q_UNUSED(port);
    BEGIN_END
    return -1;
}

qint64 QLLCPSocketState::WriteDatagram(const char *data, qint64 size)
{
    Q_UNUSED(data);
    Q_UNUSED(size);
    BEGIN_END
    return -1;
}

bool QLLCPSocketState::Bind(quint8 port)
{
    Q_UNUSED(port);
    BEGIN_END
    return false;
}

 bool QLLCPSocketState::WaitForReadyRead(int msecs)
 {
    Q_UNUSED(msecs);
    BEGIN_END
    return false;
 }

bool QLLCPSocketState::WaitForConnected(int msecs)
{
    Q_UNUSED(msecs);
    BEGIN_END
    return false;
}

bool QLLCPSocketState::WaitForDisconnected(int msecs)
{
    Q_UNUSED(msecs);
    BEGIN_END
    return false;
}

void QLLCPSocketState::ConnectToService(QNearFieldTarget *target, const QString &serviceUri)
{
   Q_UNUSED(target);
   Q_UNUSED(serviceUri);
   m_socket->invokeError();
   BEGIN_END
}

void QLLCPSocketState::DisconnectFromService()
{
   m_socket->invokeError();
   BEGIN_END
}

QTNFC_END_NAMESPACE

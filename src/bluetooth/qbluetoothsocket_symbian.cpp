/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#include "qbluetoothsocket_p.h"
#include "qbluetoothsocket.h"
#include "qbluetoothlocaldevice_p.h"
#include "symbian/utils_symbian_p.h"

#include <QCoreApplication>

#include <QDebug>

#include <limits.h>
#include <bt_sock.h>
#include <es_sock.h>

QTBLUETOOTH_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QSocketServerPrivate, getSocketServer)

QBluetoothSocketPrivate::QBluetoothSocketPrivate()
    : socket(0)
    , socketType(QBluetoothSocket::UnknownSocketType)
    , state(QBluetoothSocket::UnconnectedState)
    , readNotifier(0)
    , connectWriteNotifier(0)
    , discoveryAgent(0)
    , iSocket(0)
    , rxDescriptor(0, 0)
    , txDescriptor(0, 0)
    , recvMTU(0)
    , txMTU(0)
    , transmitting(false)
    , writeSize(0)
{
}

QBluetoothSocketPrivate::~QBluetoothSocketPrivate()
{
    delete iSocket;
}

void QBluetoothSocketPrivate::connectToService(const QBluetoothAddress &address, quint16 port, QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);
    Q_UNUSED(openMode);
    
    TBTSockAddr a;

    if(address.isNull())
        {
        socketError = QBluetoothSocket::UnknownSocketError;
        emit q->error(socketError);
        return;
        }
    TInt err = KErrNone;
    a.SetPort(port);
    // Trap TBTDevAddr constructor which may panic
    TRAP(err, a.SetBTAddr(TBTDevAddr(address.toUInt64())));
    if(err == KErrNone)
        err = iSocket->Connect(a);
    if (err == KErrNone) {
        q->setSocketState(QBluetoothSocket::ConnectingState);
    } else {
        socketError = QBluetoothSocket::UnknownSocketError;
        emit q->error(socketError);
    }
}

bool QBluetoothSocketPrivate::ensureNativeSocket(QBluetoothSocket::SocketType type)
{
    if (iSocket) {
        if (socketType == type)
            {
            return true;
            }
        else
            {
            delete iSocket;
            iSocket = 0;
            }
    }

    socketType = type;

    switch (type) {
    case QBluetoothSocket::L2capSocket: {
        TRAPD(err, iSocket = CBluetoothSocket::NewL(*this, getSocketServer()->socketServer, _L("L2CAP")));
        Q_UNUSED(err);
        break;
    }
    case QBluetoothSocket::RfcommSocket: {
        TRAPD(err, iSocket = CBluetoothSocket::NewL(*this, getSocketServer()->socketServer, _L("RFCOMM")));
        Q_UNUSED(err);
        break;
    }
    default:
        iSocket = 0;
    }

    if (iSocket)
        return true;
    else
        return false;
}

bool QBluetoothSocketPrivate::ensureBlankNativeSocket(QBluetoothSocket::SocketType type)
{
    if (iSocket) {
        delete iSocket;
        iSocket = 0;
    }
    socketType = type;
    TRAPD(err, iSocket = CBluetoothSocket::NewL(*this, getSocketServer()->socketServer));
    Q_UNUSED(err);
    if(iSocket)
        return true;
    else
        return false;
}

void QBluetoothSocketPrivate::startReceive()
{
    Q_Q(QBluetoothSocket);
    if (!iSocket || state != QBluetoothSocket::ConnectedState) {
        emit q->error(QBluetoothSocket::UnknownSocketError);
        return;
    }
    TInt err = iSocket->GetOpt(KL2CAPInboundMTU, KSolBtL2CAP, recvMTU);
    if(err != KErrNone)
        {
        emit q->error(QBluetoothSocket::UnknownSocketError);
        return;
        }
    err = iSocket->GetOpt(KL2CAPNegotiatedOutboundMTU, KSolBtL2CAP, txMTU);
    if(err != KErrNone)
        {
        emit q->error(QBluetoothSocket::UnknownSocketError);
        return;
    }
    bufPtr = buffer.reserve(recvMTU);
    // enable automatic power saving mode
    iSocket->SetAutomaticSniffMode(ETrue);
    // enable link down and link error notifications for proper disconnection
    err = iSocket->ActivateBasebandEventNotifier(ENotifyPhysicalLinkDown |ENotifyPhysicalLinkError );
    if(err != KErrNone)
        {
        emit q->error(QBluetoothSocket::UnknownSocketError);
        return;
        }
    receive();
}

void QBluetoothSocketPrivate::receive()
{
    Q_Q(QBluetoothSocket);
    TInt err = KErrNone;
    
    if(!iSocket || bufPtr == 0 || recvMTU == 0 || socketType < 0 || state != QBluetoothSocket::ConnectedState)
        {
        emit q->error(QBluetoothSocket::UnknownSocketError);
        return;
        }
    
    TRAP(err, rxDescriptor.Set(reinterpret_cast<unsigned char *>(bufPtr), 0,recvMTU));
    if(err != KErrNone)
        {
        emit q->error(QBluetoothSocket::UnknownSocketError);
        return;
        }
    if (socketType == QBluetoothSocket::RfcommSocket) {
            // cancel any pending recv operation
            iSocket->CancelRecv();
            err = iSocket->RecvOneOrMore(rxDescriptor, 0, rxLength);
            if (err != KErrNone) {
                socketError = QBluetoothSocket::UnknownSocketError;
                emit q->error(socketError);
            }
        } else if (socketType == QBluetoothSocket::L2capSocket) {
            // cancel any pending read operation
            iSocket->CancelRead();
            err = iSocket->Read(rxDescriptor);
            if (err != KErrNone) {
                socketError = QBluetoothSocket::UnknownSocketError;
                emit q->error(socketError);
            }
        }    
    }

void QBluetoothSocketPrivate::HandleAcceptCompleteL(TInt aErr)
{
    qDebug() << __PRETTY_FUNCTION__ << ">> aErr=" << aErr;
    Q_Q(QBluetoothSocket);
    if(aErr != KErrNone)
        {
        socketError = QBluetoothSocket::UnknownSocketError;
        emit q->error(socketError);
        return;
        }
    emit q->connected();
}

void QBluetoothSocketPrivate::HandleActivateBasebandEventNotifierCompleteL(TInt aErr, TBTBasebandEventNotification& aEventNotification)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_Q(QBluetoothSocket);
    if(aErr != KErrNone)
        {
        socketError = QBluetoothSocket::UnknownSocketError;
        emit q->error(socketError);
        }
    if(ENotifyPhysicalLinkDown & aEventNotification.EventType() || ENotifyPhysicalLinkError & aEventNotification.EventType())
        {
        q->setSocketState(QBluetoothSocket::ClosingState);
        emit q->disconnected();
        }
}

void QBluetoothSocketPrivate::HandleConnectCompleteL(TInt aErr)
{
    qDebug() << __PRETTY_FUNCTION__ << ">> aErr=" << aErr;
    Q_Q(QBluetoothSocket);
    if (aErr == KErrNone) {
        q->setSocketState(QBluetoothSocket::ConnectedState);

        emit q->connected();

        startReceive();
    } else {
        q->setSocketState(QBluetoothSocket::UnconnectedState);

        switch (aErr) {
        case KErrCouldNotConnect:
            socketError = QBluetoothSocket::ConnectionRefusedError;
            break;
        default:
            qDebug() << __PRETTY_FUNCTION__ << aErr;
            socketError = QBluetoothSocket::UnknownSocketError;
        }
        emit q->error(socketError);
    }
}

void QBluetoothSocketPrivate::HandleIoctlCompleteL(TInt aErr)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_Q(QBluetoothSocket);
    if(aErr != KErrNone)
        {
        socketError = QBluetoothSocket::UnknownSocketError;
        emit q->error(socketError);
        }
}

void QBluetoothSocketPrivate::HandleReceiveCompleteL(TInt aErr)
{
    Q_Q(QBluetoothSocket);
    if (aErr == KErrNone) {
        if(buffer.size() == 0)
            bufPtr = buffer.reserve(recvMTU);
        if(bufPtr <= 0)
            {
            socketError = QBluetoothSocket::UnknownSocketError;
            emit q->error(socketError);
            return;
            }
        buffer.chop(recvMTU - rxDescriptor.Length());
        emit q->readyRead();
    } else {
        // ignore disconnection it will be handled in baseband notification
        if(aErr != KErrDisconnected)
            {
            socketError = QBluetoothSocket::UnknownSocketError;
            writeSize = 0;
            emit q->error(socketError);
            }
    }
}

void QBluetoothSocketPrivate::HandleSendCompleteL(TInt aErr)
{
    Q_Q(QBluetoothSocket);
    transmitting = false;
    if (aErr == KErrNone) {
        txArray.remove(0, writeSize);
        emit q->bytesWritten(writeSize);
        if (state == QBluetoothSocket::ClosingState)
            {
            writeSize = 0;
            q->close();
            }
        else
            {
            if(!tryToSend())
                {
                socketError = QBluetoothSocket::UnknownSocketError;
                writeSize = 0;
                emit q->error(socketError);
                }
            }
    } else {
            socketError = QBluetoothSocket::UnknownSocketError;
            writeSize = 0;
            emit q->error(socketError);
    }
}

void QBluetoothSocketPrivate::HandleShutdownCompleteL(TInt aErr)
{
    Q_Q(QBluetoothSocket);
    // It doesn't matter if aErr is KErrNone or something else
    // we consider the socket to be closed.
    q->setSocketState(QBluetoothSocket::UnconnectedState);
    transmitting = false;
    writeSize = 0;
    iSocket->AsyncDelete();
    emit q->disconnected();
}

QSocketServerPrivate::QSocketServerPrivate()
{
    /* connect to socket server */
    TInt result = socketServer.Connect();
    if (result != KErrNone) {
        qWarning("%s: RSocketServ.Connect() failed with error %d", __PRETTY_FUNCTION__, result);
        return;
    }
}

QSocketServerPrivate::~QSocketServerPrivate()
{
    if (socketServer.Handle() != 0)
        socketServer.Close();
}

QString QBluetoothSocketPrivate::localName() const
{
    return QBluetoothLocalDevicePrivate::name();
}

QBluetoothAddress QBluetoothSocketPrivate::localAddress() const
{
    TBTSockAddr address;
    if(!iSocket)
        {
        // need to return something anyway
        return QBluetoothAddress();
        }
    iSocket->LocalName(address);
    return qTBTDevAddrToQBluetoothAddress(address.BTAddr());
}

quint16 QBluetoothSocketPrivate::localPort() const
{
    if(!iSocket)
        {
        // need to return something anyway
        return 0;
        }
    return iSocket->LocalPort();
}

QString QBluetoothSocketPrivate::peerName() const
{
	RHostResolver resolver;
	
	if(getSocketServer()->socketServer.Handle()== 0 || !iSocket || state != QBluetoothSocket::ConnectedState )
	    {
	    // need to return something anyway
	    return QString();
	    }
    TInt err = resolver.Open(getSocketServer()->socketServer, KBTAddrFamily, KBTLinkManager);
    if (err==KErrNone)
        {
        TNameEntry nameEntry;
        TBTSockAddr sockAddr;
        iSocket->RemoteName(sockAddr);
        TInquirySockAddr address(sockAddr);
        address.SetBTAddr(sockAddr.BTAddr());
        address.SetAction(KHostResName|KHostResIgnoreCache); // ignore name stored in cache
        err = resolver.GetByAddress(address, nameEntry);
        if(err == KErrNone)
          {
          TNameRecord name = nameEntry();
          QString qString((QChar*)name.iName.Ptr(),name.iName.Length());
          m_peerName = qString;
          }
        }
    resolver.Close();
    
    if(err != KErrNone)
        {
        // What is best? return an empty string or return the MAC address?
        // In Symbian if we can't get the remote name we usually replace it with the MAC address
        // but since Bluez implementation return an empty string we do the same here.
        return QString();
        }
    
    return m_peerName;
}

QBluetoothAddress QBluetoothSocketPrivate::peerAddress() const
{
    TBTSockAddr address;
    if(!iSocket)
        {
        // need to return something anyway
        return QBluetoothAddress();
        }
    iSocket->RemoteName(address);
    return qTBTDevAddrToQBluetoothAddress(address.BTAddr());
}

quint16 QBluetoothSocketPrivate::peerPort() const
{
    TBTSockAddr address;
    if(!iSocket)
        {
        // need to return something anyway
        return 0;
        }
    iSocket->RemoteName(address);
    return address.Port();
}

void QBluetoothSocketPrivate::close()
{
    Q_Q(QBluetoothSocket);
    if(!iSocket || (state != QBluetoothSocket::ConnectedState && state != QBluetoothSocket::ListeningState))
        {
        socketError = QBluetoothSocket::UnknownSocketError;
        emit q->error(socketError);
        return;
        }
    iSocket->CancelBasebandEventNotifier();
    q->setSocketState(QBluetoothSocket::ClosingState);
    iSocket->Shutdown(RSocket::ENormal);
}

void QBluetoothSocketPrivate::abort()
{
    Q_Q(QBluetoothSocket);
    if(!iSocket)
        {
        socketError = QBluetoothSocket::UnknownSocketError;
        emit q->error(socketError);
        return;
        }
    iSocket->CancelWrite();
    transmitting = false;
    writeSize = 0;
    iSocket->CancelBasebandEventNotifier();
    iSocket->Shutdown(RSocket::EImmediate);
    // force active object to run and shutdown socket.
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

qint64 QBluetoothSocketPrivate::readData(char *data, qint64 maxSize)
{
    Q_Q(QBluetoothSocket);
    if(data == 0 || maxSize <= 0)
        {
        return -1;
        }
    qint64 size = buffer.read(data, maxSize);
    QMetaObject::invokeMethod(q, "_q_startReceive", Qt::QueuedConnection);
    return size;
}

qint64 QBluetoothSocketPrivate::writeData(const char *data, qint64 maxSize)
{    
    if(!iSocket || data == 0 || maxSize <= 0 || txMTU <= 0) {
        return -1;
    }
    if (!txArray.isEmpty())
        {
        txArray.append(QByteArray(data, maxSize));
        }
    else
        {
        txArray = QByteArray(data, maxSize);
        }
    // we try to send the data to the remote device
    if(tryToSend())
        {
        // Warning : It doesn't mean the data have been sent
        // to the remote device, it means that the data was 
        // at least stored in a local buffer.
        return maxSize;
        }
    else
        {
        writeSize = 0;
        return -1;
        }
}

void QBluetoothSocketPrivate::_q_readNotify()
{
}

void QBluetoothSocketPrivate::_q_writeNotify()
{

}

bool QBluetoothSocketPrivate::setSocketDescriptor(int socketDescriptor, QBluetoothSocket::SocketType socketType,
    QBluetoothSocket::SocketState socketState, QBluetoothSocket::OpenMode openMode)
{
    Q_UNUSED(socketDescriptor);
    Q_UNUSED(socketType);
    Q_UNUSED(socketState);
    Q_UNUSED(openMode);
    return false;
}

void QBluetoothSocketPrivate::_q_startReceive()
{
    receive();
}

qint64 QBluetoothSocketPrivate::bytesAvailable() const
{
    return buffer.size();
}

bool QBluetoothSocketPrivate::tryToSend()
{
    if(txArray.isEmpty())
        return true;
    
    if(transmitting)
        return transmitting;

    // we cannot write more than txMTU otherwise the extra data will just be lost
    TInt dataLen = qMin(txArray.length(),txMTU);
    
    TRAPD(err, txDescriptor.Set(reinterpret_cast<const unsigned char *>(txArray.constData()),dataLen));
    if(err != KErrNone) 
        {
        transmitting = false;
        }
    else
        {
        err = iSocket->Write(txDescriptor);
        if (err != KErrNone) 
            {
            transmitting = false;
            }
        writeSize = dataLen;
        transmitting = true;
        }
    return transmitting;
}

QTBLUETOOTH_END_NAMESPACE

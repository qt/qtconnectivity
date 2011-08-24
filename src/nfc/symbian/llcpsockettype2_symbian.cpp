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

#include "nearfieldutility_symbian.h"
#include "llcpsockettype2_symbian.h"

#include "debug.h"

/*
    CLlcpSocketType2::ContructL()
*/
void CLlcpSocketType2::ConstructL()
    {
    BEGIN
    User::LeaveIfError(iNfcServer.Open());
    iLlcp = CLlcpProvider::NewL( iNfcServer );
    iWait = new (ELeave) CActiveSchedulerWait;
    END
    }

/*
    CLlcpSocketType2::CLlcpSocketType2()
*/
CLlcpSocketType2::CLlcpSocketType2(MLlcpConnOrientedTransporter* aTransporter, QLlcpSocketPrivate* aCallback)
    : iLlcp( NULL ),
      iTransporter(aTransporter),
      iWaitStatus(ENone),
      iCallback(aCallback)
    {
    }

CLlcpSocketType2* CLlcpSocketType2::NewL(QLlcpSocketPrivate* aCallback)
    {
    BEGIN
    CLlcpSocketType2* self = new (ELeave) CLlcpSocketType2(NULL,aCallback);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    END
    return self;
    }

CLlcpSocketType2* CLlcpSocketType2::NewL(MLlcpConnOrientedTransporter* aTransporter, QLlcpSocketPrivate* aCallback)
    {
    BEGIN
    CLlcpSocketType2* self = new (ELeave) CLlcpSocketType2(aTransporter,aCallback);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    END
    return self;
    }

/*
    Destroys the LLCP socket.
*/
CLlcpSocketType2::~CLlcpSocketType2()
    {
    BEGIN
    delete iConnecter;
    delete iSender;
    delete iReceiver;
    delete iTransporter;
    delete iLlcp;
    delete iWait;
    delete iTimer;
    iReceiveBufArray.ResetAndDestroy();
    iReceiveBufArray.Close();
    iNfcServer.Close();
    END
    }

/*
    Connects to the service identified by the URI \a serviceUri (on \a target).
*/
void CLlcpSocketType2::ConnectToServiceL( const QString &serviceUri)
    {
    BEGIN
    HBufC8* serviceName = QNFCNdefUtility::QString2HBufC8L(serviceUri);

    CleanupStack::PushL(serviceName);
    ConnectToServiceL(serviceName->Des()) ;
    CleanupStack::PopAndDestroy(serviceName);
    END
    }

void CLlcpSocketType2::ConnectToServiceL( const TDesC8& aServiceName)
    {
    BEGIN
    if ( !iConnecter && !iTransporter)
        {
        iTransporter = iLlcp->CreateConnOrientedTransporterL( aServiceName );
        iConnecter = CLlcpConnecterAO::NewL( *iTransporter, *this );
        }
    iConnecter->ConnectL( aServiceName );
    END
    }

/*
    Disconnects the socket.
*/

TInt CLlcpSocketType2::DisconnectFromService()
    {
    BEGIN
    if (iSender && iSender->IsActive())
        {
        WaitForBytesWritten(3000);//wait 3 seconds
        }
    if (iSender)
        {
        delete iSender;
        iSender = NULL;
        }
    if (iReceiver)
        {
        delete iReceiver;
        iReceiver = NULL;
        }

    if (iConnecter)
        {
        iConnecter->Disconnect();
        delete iConnecter;
        iConnecter = NULL;
        }
    if (iTransporter)
        {
        LOG("delete iTransporter;");
        delete iTransporter;
        iTransporter = NULL;
        }
    END
    return KErrNone;
    }

/*
    Sends the datagram at aData  to the service that this socket is connected to.
    Returns the number of bytes sent on success; otherwise return -1;
*/
TInt CLlcpSocketType2::StartWriteDatagram(const TDesC8& aData)
    {
    BEGIN
    TInt val = -1;
    if (!iTransporter)
        {
        END
        return val;
        }

    if (!iSender)
        {
        TRAPD(err,iSender = CLlcpSenderAO::NewL(*iTransporter, *this));
        if (err != KErrNone)
            {
            END
            return val;
            }
        }
    TInt error = KErrNone;
    //asynchronous transfer
    error = iSender->Send( aData);
    if (KErrNone == error)
        {
        val =  0;
        }
    END
    return val;
    }

TBool CLlcpSocketType2::ReceiveData(TDes8& aData)
    {
    //fetch data from internal buffer
    BEGIN
    TBool ret = EFalse;
    HBufC8* buf = NULL;
    TInt extBufferLength = aData.Length();
    TInt extBufferMaxLength = aData.MaxLength();
    while ( iReceiveBufArray.Count() > 0 )
        {
        buf = iReceiveBufArray[ 0 ];
        if (buf->Length() - iBufferOffset <=  extBufferMaxLength - extBufferLength )
            {//internal buffer's size <= available space of the user specified buffer
            TPtrC8 ptr(buf->Ptr() + iBufferOffset, buf->Length() - iBufferOffset);
            aData.Append( ptr );
            iReceiveBufArray.Remove( 0 );
            extBufferLength += buf->Length() - iBufferOffset;
            delete buf;
            buf = NULL;
            iBufferOffset = 0;
            }
        else
            {
            TPtrC8 ptr(buf->Ptr() + iBufferOffset, extBufferMaxLength - extBufferLength);
            aData.Append( ptr );
            iBufferOffset += extBufferMaxLength - extBufferLength;
            ret = ETrue;
            break;
            }
        ret = ETrue;
        }
    END
    return ret;
    }

TInt64 CLlcpSocketType2::BytesAvailable()
    {
    BEGIN
    TInt64 ret = 0;
    for (TInt i = 0; i < iReceiveBufArray.Count(); ++i)
        {
        HBufC8* buf = iReceiveBufArray[ i ];
        if (!buf)
            {
                continue;
            }
        if ( i == 0)
            {
            ret += buf->Length() - iBufferOffset;
            }
        else
            {
            ret += buf->Length();
            }
        }
    END
    return ret;
    }

TBool CLlcpSocketType2::WaitForOperationReady(TWaitStatus aWaitStatus,TInt aMilliSeconds)
    {
    BEGIN
    TBool ret = EFalse;
    if (iWaitStatus != ENone || iWait->IsStarted())
        {
        END
        return ret;
        }
    iWaitStatus = aWaitStatus;

    if (iTimer)
        {
        delete iTimer;
        iTimer = NULL;
        }
    if (aMilliSeconds > 0)
        {
        TRAPD(err, iTimer = CLlcpTimer::NewL(*iWait));
        if (err != KErrNone)
            {
            END
            return ret;
            }
        iTimer->Start(aMilliSeconds);
        }
    iWait->Start();
    //control is back here when iWait->AsyncStop() is called by the timer or the callback function
    iWaitStatus = ENone;

    if (!iTimer)
        {
        //iTimer == NULL means this CActiveSchedulerWait
        //AsyncStop is fired by the call back of ReadyRead
        ret = ETrue;
        }
    else
        {
        delete iTimer;
        iTimer = NULL;
        }
    END
    return ret;
    }

/**
 * Blocks until data is available for reading and the readyRead()
 * signal has been emitted, or until msecs milliseconds have
 * passed. If msecs is -1, this function will not time out.
 * Returns true if data is available for reading; otherwise
 * returns false (if the operation timed out or if an error
 * occurred).
 */
TBool CLlcpSocketType2::WaitForReadyRead(TInt aMilliSeconds)
    {
    BEGIN
    END
    return WaitForOperationReady(EWaitForReadyRead, aMilliSeconds);
    }

TBool CLlcpSocketType2::WaitForBytesWritten(TInt aMilliSeconds)
    {
    BEGIN
    END
    return WaitForOperationReady(EWaitForBytesWritten, aMilliSeconds);
    }
TBool CLlcpSocketType2::WaitForConnected(TInt aMilliSeconds)
    {
    BEGIN
    END
    return WaitForOperationReady(EWaitForConnected, aMilliSeconds);
    }

void CLlcpSocketType2::AttachCallbackHandler(QLlcpSocketPrivate* aCallback)
    {
    BEGIN
    iCallback = aCallback;
    if (iTransporter && iTransporter->IsConnected())//has connected llcp transporter
        {
        LOG("A server llcp type2 socket");
        if (!iReceiver)
            {
            TRAPD(err,iReceiver = CLlcpReceiverAO::NewL( *iTransporter, *this ));
            if (err != KErrNone)
                {
                END
                return;
                }
            }
        if (!iConnecter)
            {
            TRAP_IGNORE(iConnecter = CLlcpConnecterAO::NewL( *iTransporter, *this ));
            }
        if (iReceiver->StartReceiveDatagram() != KErrNone)
            {
            Error(QLlcpSocket::UnknownSocketError);
            }
        }
    END
    }

void CLlcpSocketType2::Error(QLlcpSocket::SocketError /*aSocketError*/)
    {
    BEGIN
    //emit error
    if ( iCallback )
        {
        TInt error = KErrNone;
        QT_TRYCATCH_ERROR(error,iCallback->invokeError());
        //can do nothing if there is an error,so just ignore it
        Q_UNUSED(error);
        }
    END
    }
void CLlcpSocketType2::StateChanged(QLlcpSocket::SocketState aSocketState)
    {
    BEGIN
    if (aSocketState == QLlcpSocket::ConnectedState && iWaitStatus == EWaitForConnected)
        {
        StopWaitNow(EWaitForConnected);
        }
    TInt error = KErrNone;
    if (aSocketState == QLlcpSocket::ConnectedState)
        {
        if ( iCallback)
            {
            QT_TRYCATCH_ERROR(error, iCallback->invokeConnected());
            Q_UNUSED(error);
            }
        if (!iReceiver)
            {
            TRAPD(err,iReceiver = CLlcpReceiverAO::NewL( *iTransporter, *this ));
            if (err != KErrNone)
                {
                Error(QLlcpSocket::UnknownSocketError);
                return;
                }
            }
        if(iReceiver->StartReceiveDatagram() != KErrNone)
            {
            Error(QLlcpSocket::UnknownSocketError);
            }
        }

    if (aSocketState == QLlcpSocket::ClosingState && iCallback)
        {
        QT_TRYCATCH_ERROR(error, iCallback->invokeDisconnected());
        Q_UNUSED(error);
        }

    END
    }

void CLlcpSocketType2::StopWaitNow(TWaitStatus aWaitStatus)
    {
    BEGIN
    if ( iWaitStatus == aWaitStatus )
        {
        if (iTimer)//stop the timer
            {
            delete iTimer;
            iTimer = NULL;
            }
        if (iWait->IsStarted())
            {
            iWait->AsyncStop();
            }
        }
    END
    }
void CLlcpSocketType2::ReadyRead()
    {
    BEGIN
    if (iWaitStatus == EWaitForReadyRead)
        {
        StopWaitNow(EWaitForReadyRead);
        }
    //emit readyRead()
    if ( iCallback )
        {
        TInt error = KErrNone;
        QT_TRYCATCH_ERROR(error,iCallback->invokeReadyRead());
        //can do nothing if there is an error,so just ignore it
        Q_UNUSED(error);
        }
    END
    }
void CLlcpSocketType2::BytesWritten(qint64 aBytes)
    {
    BEGIN
    if (iWaitStatus == EWaitForBytesWritten)
        {
        StopWaitNow(EWaitForBytesWritten);
        }
    //emit bytesWritten signal;
    if ( iCallback )
        {
        TInt error = KErrNone;
        QT_TRYCATCH_ERROR(error,iCallback->invokeBytesWritten(aBytes));
        //can do nothing if there is an error,so just ignore it
        Q_UNUSED(error);
        }
    END
    }

// connecter implementation
CLlcpConnecterAO* CLlcpConnecterAO::NewL( MLlcpConnOrientedTransporter& aConnection, CLlcpSocketType2& aSocket )
    {
    BEGIN
    CLlcpConnecterAO* self = new (ELeave) CLlcpConnecterAO( aConnection, aSocket );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    END
    return self;
    }

CLlcpConnecterAO::CLlcpConnecterAO( MLlcpConnOrientedTransporter& aConnection, CLlcpSocketType2& aSocket )
    : CActive( EPriorityStandard ),
      iConnection( aConnection ),
      iSocket( aSocket ),
      iConnState( ENotConnected )
    {
    }
/*
    ConstructL
*/
void CLlcpConnecterAO::ConstructL()
    {
    BEGIN
    CActiveScheduler::Add( this );
    if ( iConnection.IsConnected() )
        {
        LOG("a LLCP server side socket");
        iConnState = EConnected;
        // Starting listening disconnect event
        iConnection.WaitForDisconnection( iStatus );
        SetActive();
        }
    END
    }

/*
 * Destructor.
 */
CLlcpConnecterAO::~CLlcpConnecterAO()
    {
    BEGIN
    Cancel();
    if ( iConnState == EConnected )
        {
        iConnection.Disconnect();
        }
    END
    }
/*
 * Connect to remote peer as given service uri.
 */
void CLlcpConnecterAO::ConnectL(const TDesC8& /*aServiceName*/)
    {
    BEGIN
    if ( iConnState == ENotConnected )
        {
        // Starting connecting if is in idle state
        iConnection.Connect( iStatus );
        SetActive();
        iConnState = EConnecting;
        //emit connecting signal
        iSocket.StateChanged(QLlcpSocket::ConnectingState);
        }
    END
    }

/*
 * Disconnect with remote peer.
 */
void CLlcpConnecterAO::Disconnect()
    {
    BEGIN
    if ( iConnState == ENotConnected )
        {
        END
        return;
        }
    Cancel();
    if ( iConnState == EConnected )
        {
        iConnection.Disconnect();
        }
    iConnState = ENotConnected;

    //emit QAbstractSocket::ClosingState;
    iSocket.StateChanged(QLlcpSocket::ClosingState);
    END
    }
void CLlcpConnecterAO::RunL()
    {
    BEGIN
    TInt error = iStatus.Int();

    switch ( iConnState )
        {
        // Handling connecting request
        case EConnecting:
            {
            if ( error == KErrNone )
                {
                LOG("Connected to LLCP server");
                // Updating state
                iConnState = EConnected;
                //emit connected signal
                iSocket.StateChanged(QLlcpSocket::ConnectedState);
                // Starting listening disconnect event
                iConnection.WaitForDisconnection( iStatus );
                SetActive();
                }
            else
                {
                LOG("!!Failed to Connected to LLCP server");
                //KErrNotSupported when remote peer has lost from the near field.
                iConnState = ENotConnected;
                //emit error signal
                iSocket.Error(QLlcpSocket::UnknownSocketError);
                }
            }
            break;
        case EConnected:
            {
            //handling disconnect event
            if ( error == KErrNone )
                {
                LOG("Disconnected event received");
                // Updating state
                iConnState = ENotConnected;
                //emit disconnected signal
                iSocket.StateChanged(QLlcpSocket::ClosingState);
                }
            else
                {
                iConnState = ENotConnected;
                //emit error signal
                iSocket.Error(QLlcpSocket::UnknownSocketError);
                }
            }
            break;
        default:
            {
            // Do nothing
            }
            break;
        }
    END
    }
void CLlcpConnecterAO::DoCancel()
    {
    BEGIN
    switch ( iConnState )
        {
        case EConnecting:
            {
            iConnection.ConnectCancel();
            }
            break;

        case EConnected:
            {
            iConnection.WaitForDisconnectionCancel();
            }
            break;

        default:
            {
            // Do nothing
            }
            break;
        }
    END
    }
//sender AO implementation

CLlcpSenderAO* CLlcpSenderAO::NewL( MLlcpConnOrientedTransporter& aConnection, CLlcpSocketType2& aSocket )
    {
    BEGIN
    CLlcpSenderAO* self = new (ELeave) CLlcpSenderAO( aConnection, aSocket );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    END
    return self;
    }

CLlcpSenderAO::CLlcpSenderAO( MLlcpConnOrientedTransporter& aConnection, CLlcpSocketType2& aSocket )
    : CActive( EPriorityStandard ),
      iConnection( aConnection ),
      iSocket( aSocket )
    {
    }
/*
    ConstructL
*/
void CLlcpSenderAO::ConstructL()
    {
    BEGIN
    CActiveScheduler::Add( this );
    END
    }

/*
 * Destructor.
 */
CLlcpSenderAO::~CLlcpSenderAO()
    {
    BEGIN
    Cancel();
    iSendBuf0.Close();
    iSendBuf1.Close();
    //todo
    iCurrentSendBuf.Close();
    END
    }
/*
 * Transfer given data to remote device.
 */
TInt CLlcpSenderAO::Send( const TDesC8& aData )
    {
    BEGIN
    TInt error = KErrNone;
    if (aData.Length() == 0)
        {
        END
        return KErrArgument;
        }
    TInt supportedDataLength = iConnection.SupportedDataLength();
    if (supportedDataLength <= 0)
        {
        END
        return KErrNotReady;
        }
    if ( !IsActive() )
        {
        // Copying data to internal buffer.
        iSendBuf0.Zero();
        iCurrentPos = 0;
        error = iSendBuf0.ReAlloc( aData.Length() );

        if ( error == KErrNone )
          {
          iSendBuf0.Append( aData );

          if (iSendBuf0.Length() > supportedDataLength)
              {
              iCurrentSendPtr.Set(iSendBuf0.Ptr(), supportedDataLength);
              }
          else
              {
              iCurrentSendPtr.Set(iSendBuf0.Ptr(), iSendBuf0.Length());
              }
          // Sending data
          //TODO defect in NFC server
          iCurrentSendBuf.Close();
          iCurrentSendBuf.Create(iCurrentSendPtr);
          iConnection.Transmit( iStatus, iCurrentSendBuf );
//          iConnection.Transmit( iStatus, iCurrentSendPtr );
          SetActive();
          iCurrentBuffer = EBuffer0;
          }
        }
    else
      {
        if (iCurrentBuffer == EBuffer0)
            {
            error = iSendBuf1.ReAlloc( iSendBuf1.Length() + aData.Length() );
            if (error == KErrNone)
                {
                iSendBuf1.Append(aData);
                }
            }
        else
            {
            error = iSendBuf0.ReAlloc( iSendBuf0.Length() + aData.Length() );
            if (error == KErrNone)
                {
                iSendBuf0.Append(aData);
                }
            }
      }
    END
    return error;
    }

void CLlcpSenderAO::SendRestDataAndSwitchBuffer(RBuf8& aWorkingBuffer, RBuf8& aNextBuffer)
    {
    BEGIN
    TInt supportedDataLength = iConnection.SupportedDataLength();
    if (iCurrentPos == aWorkingBuffer.Length())
        {
        LOG("Current working buffer write finished");

        aWorkingBuffer.Zero();
        if(aNextBuffer.Length() > 0)
            {

            if (&aNextBuffer == &iSendBuf0)
                {
                iCurrentBuffer = EBuffer0;
                LOG("Start switch to buffer 0");
                }
            else
                {
                iCurrentBuffer = EBuffer1;
                LOG("Start switch to buffer 1");
                }

            iCurrentPos = 0;

            if (supportedDataLength > 0)
                {
                if (aNextBuffer.Length() > supportedDataLength)
                    {
                    iCurrentSendPtr.Set(aNextBuffer.Ptr(), supportedDataLength);
                    }
                else
                    {
                    iCurrentSendPtr.Set(aNextBuffer.Ptr(), aNextBuffer.Length());
                    }
                //TODO
                iCurrentSendBuf.Close();
                iCurrentSendBuf.Create(iCurrentSendPtr);
                iConnection.Transmit( iStatus, iCurrentSendBuf );
//              iConnection.Transmit( iStatus, iCurrentSendPtr );
                SetActive();
                }
            else
                {
                LOG("SupportedDataLength is invalid, value="<<supportedDataLength);
                iSendBuf0.Zero();
                iSendBuf1.Zero();
                iCurrentBuffer = EBuffer0;
                iSocket.Error(QLlcpSocket::UnknownSocketError);
                }
            }
        }
    else //means current working buffer still have data need to be sent
        {
        LOG("Current working buffer still has data need to be sent");
        if (supportedDataLength > 0)
            {
            if (aWorkingBuffer.Length() - iCurrentPos > supportedDataLength)
                {
                iCurrentSendPtr.Set(aWorkingBuffer.Ptr() + iCurrentPos, supportedDataLength);
                }
            else
                {
                iCurrentSendPtr.Set(aWorkingBuffer.Ptr() + iCurrentPos, aWorkingBuffer.Length() - iCurrentPos);
                }
            // Sending data
            //TODO
            iCurrentSendBuf.Close();
            iCurrentSendBuf.Create(iCurrentSendPtr);
            iConnection.Transmit( iStatus, iCurrentSendBuf );

//                    iConnection.Transmit( iStatus, iCurrentSendPtr );
            SetActive();
            }
        else
            {
            LOG("SupportedDataLength is invalid, value="<<supportedDataLength);
            iSendBuf0.Zero();
            iSendBuf1.Zero();
            iCurrentBuffer = EBuffer0;
            iSocket.Error(QLlcpSocket::UnknownSocketError);
            }
        }
    END
    }
void CLlcpSenderAO::RunL()
    {
    BEGIN
    TInt error = iStatus.Int();
    if ( error == KErrNone )
        {
        TInt bytesWritten = iCurrentSendPtr.Length();

        iCurrentPos += iCurrentSendPtr.Length();
        if (iCurrentBuffer == EBuffer0)
            {
            SendRestDataAndSwitchBuffer(iSendBuf0, iSendBuf1);
            }//if (iCurrentBuffer == EBuffer0)
        else //current working buffer is buffer1
            {
            SendRestDataAndSwitchBuffer(iSendBuf1, iSendBuf0);
            }
        //emit BytesWritten signal
        iSocket.BytesWritten(bytesWritten);
        }//if ( error == KErrNone )
    else
        {
        LOG("iStatus.Int() = "<<error);
        iSendBuf0.Zero();
        iSendBuf1.Zero();
        iCurrentBuffer = EBuffer0;
        //emit error() signal
        iSocket.Error(QLlcpSocket::UnknownSocketError);
        }
    END
    }
void CLlcpSenderAO::DoCancel()
    {
    BEGIN
    iConnection.TransmitCancel();
    END
    }
//receiver implementation
CLlcpReceiverAO* CLlcpReceiverAO::NewL( MLlcpConnOrientedTransporter& aConnection, CLlcpSocketType2& aSocket )
    {
    BEGIN
    CLlcpReceiverAO* self = new (ELeave) CLlcpReceiverAO( aConnection, aSocket );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    END
    return self;
    }

CLlcpReceiverAO::CLlcpReceiverAO( MLlcpConnOrientedTransporter& aConnection, CLlcpSocketType2& aSocket )
    : CActive( EPriorityStandard ),
      iConnection( aConnection ),
      iSocket( aSocket )
    {
    }
/*
    ConstructL
*/
void CLlcpReceiverAO::ConstructL()
    {
    BEGIN
    CActiveScheduler::Add( this );
    END
    }

/*
 * Destructor.
 */
CLlcpReceiverAO::~CLlcpReceiverAO()
    {
    BEGIN
    Cancel();
    iReceiveBuf.Close();
    END
    }

TInt CLlcpReceiverAO::StartReceiveDatagram()
    {
    BEGIN
    TInt length = 0;
    TInt error = KErrNone;
    length = iConnection.SupportedDataLength();

    if ( length > 0 )
        {
        iReceiveBuf.Zero();
        error = iReceiveBuf.ReAlloc( length );

        if ( error == KErrNone )
            {
            iConnection.Receive( iStatus, iReceiveBuf );
            SetActive();
            }
        }
    else
        {
        // if length is 0 or negative, LLCP link is destroyed.
        LOG("Error: length is"<<length);
        error = KErrNotReady;
        }
    END
    return error;
    }

void CLlcpReceiverAO::RunL()
    {
    BEGIN
    TInt error = iStatus.Int();
    if ( error == KErrNone )
        {
        //append to buffer
        HBufC8* buf = NULL;
        buf = HBufC8::NewLC( iReceiveBuf.Length() );
        buf->Des().Copy( iReceiveBuf );
        iSocket.iReceiveBufArray.AppendL( buf );
        CleanupStack::Pop( buf );

        //emit readyRead() signal
        iSocket.ReadyRead();
        //resend the Receive request to NFC server
        if (StartReceiveDatagram() != KErrNone)
            {
            iSocket.Error(QLlcpSocket::UnknownSocketError);
            }
        }
    else if ( error == KErrCancel )
        {
        //just omit the KErrCancel
        LOG(" iStatus = KErrCancel");
        }
    else
        {
        //emit error() signal
        iSocket.Error(QLlcpSocket::UnknownSocketError);
        }
    END
    }
void CLlcpReceiverAO::DoCancel()
    {
    BEGIN
    iConnection.ReceiveCancel();
    END
    }
//EOF

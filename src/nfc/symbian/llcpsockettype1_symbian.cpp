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

#include <llcpprovider.h>                   // CLlcpProvider
#include <llcpconnlesstransporter.h>        // MLlcpConnLessTransporter

#include "nearfieldutility_symbian.h"
#include "llcpsockettype1_symbian.h"
#include "debug.h"


/*
    CLlcpSocketType1::NewL()
*/
CLlcpSocketType1* CLlcpSocketType1::NewL(QLlcpSocketPrivate& aCallback)
    {
    CLlcpSocketType1* self = CLlcpSocketType1::NewLC(aCallback);
    CleanupStack::Pop(self);
    return self;
    }

/*
    CLlcpSocketType1::NewLC()
*/
CLlcpSocketType1* CLlcpSocketType1::NewLC(QLlcpSocketPrivate& aCallback)
    {
    CLlcpSocketType1* self = new (ELeave) CLlcpSocketType1(aCallback);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

/*
    CLlcpSocketType1::CLlcpSocketType1()
*/
CLlcpSocketType1::CLlcpSocketType1(QLlcpSocketPrivate& aCallback)
    : iWaitStatus(ENone),
      iPortBinded(EFalse),
      iCallback(aCallback)
    {
    }

/*
    CLlcpSocketPrivate::ContructL()
*/
void CLlcpSocketType1::ConstructL()
    {
    User::LeaveIfError(iNfcServer.Open());
    iLlcp = CLlcpProvider::NewL(iNfcServer);
    iWait = new (ELeave) CActiveSchedulerWait;
    }

/*
    Destroys the LLCP socket.
*/
CLlcpSocketType1::~CLlcpSocketType1()
    {
    BEGIN
    // Destroy connection
    Cleanup();

    if (iLlcp)
        {
        iLlcp->StopListeningConnLessRequest(iLocalPort);
        delete iLlcp;
        }
    iNfcServer.Close();
    delete iTimer;
    delete iWait;

    END
    }

/*
    Cancel the Receive/Transfer and destroy the local/remote connection.
*/
void CLlcpSocketType1::Cleanup()
    {
    BEGIN
    // Deleting connection
    if (iConnectionWrapper)
        {
        iConnectionWrapper->TransferCancel();
        iConnectionWrapper->ReceiveCancel();
        delete iConnectionWrapper;
        iConnectionWrapper = NULL;
        }
    END
    }

/*
    Start to listen the port as given, set as local port which is used to read datagram
*/
TBool CLlcpSocketType1::Bind(TUint8 aPortNum)
    {
    BEGIN
    TBool bindOK = EFalse;
    if (!iPortBinded)
        {
        TInt error = KErrNone;
        TRAP(error, iLlcp->StartListeningConnLessRequestL(*this,aPortNum));
        if (KErrNone == error)
            {
            iPortBinded = ETrue;
            iLocalPort = aPortNum;
            bindOK = ETrue;
            }
        }
    END
    return bindOK;
    }

/*
    Sends the datagram at aData  to the service that this socket is connected to.
    Returns the number of bytes sent on success; otherwise return -1;
*/
TInt CLlcpSocketType1::StartWriteDatagram(const TDesC8& aData,TUint8 aPortNum)
    {
    BEGIN
    TInt val = -1;

    if (iConnectionWrapper != NULL && iRemotePort != aPortNum)
        {
        return val;
        }

    if (KErrNone == CreateConnection(aPortNum))
        {
        TInt error = KErrNone;
        QT_TRYCATCH_ERROR(error , iConnectionWrapper->TransferL(aData));

        if (KErrNone == error)
            {
            iCallback.m_writeDatagramRefCount++;
            val =  0;
            }
        }
    END
    return val;
    }


TInt CLlcpSocketType1::ReadDatagram(TDes8& aData, TUint8& aRemotePortNum)
    {
    BEGIN
    aRemotePortNum = iRemotePort;
    TInt val = ReadDatagram(aData);
    END
    return val;
    }

TInt CLlcpSocketType1::ReadDatagram(TDes8& aData)
    {
    BEGIN
    TInt readSize = -1;
    if (NULL != iConnectionWrapper)
        {
        readSize = iConnectionWrapper->ReceiveDataFromBuf(aData);

        // Start receiving data again
        TInt error = KErrNone;
        error = iConnectionWrapper->Receive();
        if (KErrNone != error)
            {
            readSize =  -1;
            }
        }
    END
    return readSize;
    }

TBool CLlcpSocketType1::HasPendingDatagrams() const
    {
    BEGIN
    TBool val = EFalse;
    if (NULL != iConnectionWrapper)
        {
        val = iConnectionWrapper->HasPendingDatagrams();
        }
    END
    return val;
    }

TInt64 CLlcpSocketType1::PendingDatagramSize() const
    {
    BEGIN
    TInt64 val = -1;
    if (NULL != iConnectionWrapper)
        {
        val = iConnectionWrapper->PendingDatagramSize();
        }
    END
    return val;
    }

/*
    Call back from MLlcpConnLessListener
*/
void CLlcpSocketType1::FrameReceived(MLlcpConnLessTransporter* aConnection)
    {
    BEGIN
    iRemotePort = aConnection->SsapL();
//    StartTransportAndReceive(aConnection);
    // Only accepting one incoming remote connection
    TInt error = KErrNone;
    if (iConnectionWrapper)
        {
        delete iConnectionWrapper;
        iConnectionWrapper = NULL;
        }
    // Creating wrapper for connection.
    TRAP(error, iConnectionWrapper = COwnLlcpConnectionWrapper::NewL(aConnection, *this));

    if (error == KErrNone && iConnectionWrapper != NULL)
        {
        error = iConnectionWrapper->Receive();
        }
    if (error != KErrNone)
        {
        QT_TRYCATCH_ERROR(error,iCallback.invokeError());
        }
    END
    }

/*
    Call back from MLlcpReadWriteCb
*/
void CLlcpSocketType1::ReceiveComplete(TInt aError)
    {
    BEGIN
    TInt err = KErrNone;
    if (KErrNone == aError)
        {
        QT_TRYCATCH_ERROR(err,iCallback.invokeReadyRead());
        }
    else
        {
        LOG("err = "<<err);
        QT_TRYCATCH_ERROR(err,iCallback.invokeError());
        }
    Q_UNUSED(err);
    END
    }

/*
    Call back from MLlcpReadWriteCb
*/
void CLlcpSocketType1::WriteComplete(TInt aError, TInt aSize)
    {
    BEGIN
    if (iWaitStatus == EWaitForBytesWritten)
        {
        StopWaitNow(EWaitForBytesWritten);
        }

    TInt err = KErrNone;
    if (KErrNone == aError)
        {
        iCallback.m_writeDatagramRefCount--;
        QT_TRYCATCH_ERROR(err,iCallback.invokeBytesWritten(aSize));
        }
    else
        {
        QT_TRYCATCH_ERROR(err,iCallback.invokeError());
        }

    if (err == aError && iConnectionWrapper != NULL
        && iConnectionWrapper->HasQueuedWrittenDatagram())
        {
        iConnectionWrapper->TransferQueued();
        }
    END
    }

void CLlcpSocketType1::StopWaitNow(TWaitStatus aWaitStatus)
    {
    BEGIN
    if (iWaitStatus == aWaitStatus)
        {
        if (iWait->IsStarted())
            {
            iWait->AsyncStop();
            }
        if (iTimer)//stop the timer
            {
            delete iTimer;
            iTimer = NULL;
            }
        }
    END
    }

/*
    Creating MLlcpConnLessTransporter object if  connection type connectionless,
    Creating Creating wrapper for local peer connection.
*/
TInt CLlcpSocketType1::CreateConnection(TUint8 portNum)
    {
    BEGIN
    TInt error = KErrNone;
    MLlcpConnLessTransporter* llcpConnection = NULL;

    if (iConnectionWrapper)
        {
        return error;
        }

    TRAP(error, llcpConnection = iLlcp->CreateConnLessTransporterL(portNum));

    if (error == KErrNone)
        {
          iRemotePort = portNum;
          error = StartTransportAndReceive(llcpConnection);
        }
    END
    return error;
    }

TInt CLlcpSocketType1::StartTransportAndReceive(MLlcpConnLessTransporter* aConnection)
    {
    BEGIN
    TInt error = KErrNone;

     // Only accepting one incoming remote connection
     if (!iConnectionWrapper)
         {
         // Creating wrapper for connection.
         TRAP(error, iConnectionWrapper = COwnLlcpConnectionWrapper::NewL(aConnection, *this));
         }

     if (error == KErrNone && iConnectionWrapper != NULL)
         {
         error = iConnectionWrapper->Receive();
         }
     END
     return error;
    }


TBool CLlcpSocketType1::WaitForBytesWritten(TInt aMilliSeconds)
    {
    BEGIN_END
    return WaitForOperationReady(EWaitForBytesWritten, aMilliSeconds);
    }

TBool CLlcpSocketType1::WaitForOperationReady(TWaitStatus aWaitStatus,TInt aMilliSeconds)
    {
    BEGIN
    TBool ret = EFalse;
    if (iWaitStatus != ENone || iWait->IsStarted())
        {
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


/*
    Construct the wrapper for connectionLess transport.
*/
COwnLlcpConnectionWrapper* COwnLlcpConnectionWrapper::NewL(MLlcpConnLessTransporter* aConnection
                                                           , MLlcpReadWriteCb& aCallBack)
    {
    COwnLlcpConnectionWrapper* self = COwnLlcpConnectionWrapper::NewLC(aConnection, aCallBack);
    CleanupStack::Pop(self);
    return self;
    }

/*
    Construct the new wrapper for connectionLess transport.
*/
COwnLlcpConnectionWrapper* COwnLlcpConnectionWrapper::NewLC(MLlcpConnLessTransporter* aConnection
                                                            , MLlcpReadWriteCb& aCallBack)
    {
    COwnLlcpConnectionWrapper* self = new (ELeave) COwnLlcpConnectionWrapper(aConnection);
    CleanupStack::PushL(self);
    self->ConstructL(aCallBack);
    return self;
    }

/*
    Constructor
*/
COwnLlcpConnectionWrapper::COwnLlcpConnectionWrapper(MLlcpConnLessTransporter* aConnection)
    :  iConnection(aConnection)
    {
    }

/*
    ConstructL
*/
void COwnLlcpConnectionWrapper::ConstructL(MLlcpReadWriteCb& aCallBack)
    {
    if (NULL == iConnection)
        {
        User::Leave(KErrArgument);
        }
    // Create the transmitter AO
    iSenderAO = CLlcpSenderType1::NewL(*iConnection, aCallBack);
     // Create the receiver AO
    iReceiverAO = CLlcpReceiverType1::NewL(*iConnection, aCallBack);
    }

/*
    Destroy the new wrapper for connectionLess transport.
*/
COwnLlcpConnectionWrapper::~COwnLlcpConnectionWrapper()
    {
    BEGIN
    iSendBufArray.ResetAndDestroy();
    iSendBufArray.Close();

    delete iSenderAO;
    delete iReceiverAO;

    if (iConnection)
        {
        delete iConnection;
        iConnection = NULL;
        }
    END
    }

/*
    Send data from queued buffer
*/
bool COwnLlcpConnectionWrapper::TransferQueued()
    {
    BEGIN
    bool ret = false;
    if (iSendBufArray.Count() == 0 || iSenderAO->IsActive())
        return ret;

    HBufC8* bufRef = iSendBufArray[0];
    if (NULL == bufRef)
        return ret;

    TPtrC8 ptr(bufRef->Ptr(), bufRef->Length());
    if(!iSenderAO->IsActive() && iSenderAO->Transfer(ptr) == KErrNone)
        {
        ret = true;
        }
    iSendBufArray.Remove(0);
    delete bufRef;
    bufRef = NULL;

    END
    return ret;
    }

/*
    Send data from local peer to remote peer via connectionLess transport
*/
TInt COwnLlcpConnectionWrapper::TransferL(const TDesC8& aData)
    {
    BEGIN
    TInt error = KErrNone;
    // Pass message into transmitter AO
    if (!iSenderAO->IsActive())
        {
        error = iSenderAO->Transfer(aData);
        }
    else if (aData.Length() > 0)
        {
        HBufC8* buf = HBufC8::NewLC(aData.Length());
        buf->Des().Copy(aData);
        error = iSendBufArray.Append(buf);
        CleanupStack::Pop(buf);
        }
    END
    return error;
    }


/*
 *  Trigger the receiver AO to start receive datagram
    Receive data from remote peer to local peer via connectionLess transport
*/
TInt COwnLlcpConnectionWrapper::Receive()
    {
    BEGIN
    TInt error = KErrInUse;
    // Pass message on to transmit AO
    if (!iReceiverAO->IsActive())
        {
        error = iReceiverAO->Receive();
        }

    END
    return error;
    }

/*
    Retrieve data from the buffer of the connection less socket
*/
TInt COwnLlcpConnectionWrapper::ReceiveDataFromBuf(TDes8& aData)
    {
    return iReceiverAO->ReceiveDataFromBuf(aData);
    }

bool COwnLlcpConnectionWrapper::HasPendingDatagrams() const
    {
    return iReceiverAO->HasPendingDatagrams();
    }

bool COwnLlcpConnectionWrapper::HasQueuedWrittenDatagram() const
    {
    bool hasData = iSendBufArray.Count() > 0 ? true : false;
    return hasData;
    }

TInt64 COwnLlcpConnectionWrapper::PendingDatagramSize() const
    {
    return iReceiverAO->PendingDatagramSize();
    }

/*
    Cancel data transfer from local peer to remote peer via connectionLess transport
*/
void COwnLlcpConnectionWrapper::TransferCancel()
    {
    BEGIN
    if (iSenderAO->IsActive())
        {
        iSenderAO->Cancel();
        }
    END
    }

/*
    Cancel data receive from local peer to remote peer via connectionLess transport
*/
void COwnLlcpConnectionWrapper::ReceiveCancel()
    {
    BEGIN
    if (iReceiverAO->IsActive())
        {
        iReceiverAO->Cancel();
        }
    END
    }

/*
    Start of implementation of Sender AO & Receiver AO for connection less mode (type1)
*/

/*
    Constructor
*/
CLlcpSenderType1::CLlcpSenderType1(MLlcpConnLessTransporter& aConnection, MLlcpReadWriteCb& aCallBack)
                       : CActive(CActive::EPriorityStandard)
                       , iConnection(aConnection)
                       , iSendObserver(aCallBack)
    {
    }

/*
    Constructor
*/
CLlcpSenderType1* CLlcpSenderType1::NewL(MLlcpConnLessTransporter& iConnection, MLlcpReadWriteCb& aCallBack)
    {
    CLlcpSenderType1* self = new(ELeave) CLlcpSenderType1(iConnection, aCallBack);
    CActiveScheduler::Add(self);
    return self;
    }

/*
    Destructor
*/
CLlcpSenderType1::~CLlcpSenderType1()
    {
    BEGIN
    Cancel();   // Cancel ANY outstanding request at time of destruction
    iTransmitBuf.Close();
    iTempSendBuf.Close();
    END
    }

TInt CLlcpSenderType1::Transfer(const TDesC8& aData)
    {
    BEGIN

    if (aData.Length() == 0)
        {
        return KErrArgument;
        }

    TInt supportedDataLength = iConnection.SupportedDataLength();
    if (supportedDataLength <= 0)
        {
        return KErrNotReady;
        }
    // Reset pos to start
    iCurrentSendBufPos = 0;
    TInt error = KErrNone;
    // Copying data to internal buffer.
    iTransmitBuf.Zero();
    error = iTransmitBuf.ReAlloc(aData.Length());

    if (error == KErrNone)
        {
        iTransmitBuf.Append(aData);

        if (iTransmitBuf.Length() > supportedDataLength)
            {
            iCurrentSendBufPtr.Set(iTransmitBuf.Ptr(), supportedDataLength);
            }
        else
            {
            iCurrentSendBufPtr.Set(iTransmitBuf.Ptr(), iTransmitBuf.Length());
            }
		iCurrentSendBufPos = iCurrentSendBufPtr.Length();
        // Sending data, don't need check active, external func has checked it
        iTempSendBuf.Close();
        iTempSendBuf.Create(iCurrentSendBufPtr);
        iConnection.Transmit(iStatus, iTempSendBuf);
        SetActive();
        }
    else
        {
        error = KErrNoMemory;
        }
    END
    return error;
    }

void CLlcpSenderType1::RunL(void)
    {
    BEGIN
    TInt error = iStatus.Int();
    // Sending error, notify user
    if (KErrNone != error)
        {
        // Return buffer's length which has been sent successfully.
        iSendObserver.WriteComplete(error, iCurrentSendBufPos - iCurrentSendBufPtr.Length());
        return;
        }

    TInt bytesWritten =  iCurrentSendBufPtr.Length();

    // Still have some buffer need send, don't stop
    if (iCurrentSendBufPos < iTransmitBuf.Length())
        {
        TInt supportedDataLength = iConnection.SupportedDataLength();
        if (supportedDataLength <= 0)
            {
            iSendObserver.WriteComplete(KErrGeneral, iCurrentSendBufPtr.Length());
            return;
            }

        if (iTransmitBuf.Length() > iCurrentSendBufPos + supportedDataLength)
            {
            // Still left some buffer
            iCurrentSendBufPtr.Set(iTransmitBuf.Ptr() + iCurrentSendBufPos, supportedDataLength);
            iCurrentSendBufPos += supportedDataLength;
            }
        else
            {
            // All buffer will be sent in this time
            iCurrentSendBufPtr.Set(iTransmitBuf.Ptr() + iCurrentSendBufPos
                                 , iTransmitBuf.Length() - iCurrentSendBufPos);
            iCurrentSendBufPos = iTransmitBuf.Length();
            }
        // Sending data
        iTempSendBuf.Close();
        iTempSendBuf.Create(iCurrentSendBufPtr);
        iConnection.Transmit(iStatus, iTempSendBuf);
        SetActive();
        }
    // Sent signal for each successful sending
    iSendObserver.WriteComplete(error, bytesWritten);
    END
    }

void CLlcpSenderType1::DoCancel(void)
    {
    BEGIN
    // Cancel any outstanding write request on iSocket at this time.
    iConnection.TransmitCancel();
    END
    }

/*
    Start of implementation of Receiver AO for connection less mode (type1) - CLlcpReceiverType1
*/

/*
    Constructor
*/
CLlcpReceiverType1::CLlcpReceiverType1(MLlcpConnLessTransporter& aConnection, MLlcpReadWriteCb& aCallBack)
                       : CActive(CActive::EPriorityStandard)
                       , iConnection(aConnection)
                       , iReceiveObserver(aCallBack)
    {
    }

/*
    Constructor
*/
CLlcpReceiverType1* CLlcpReceiverType1::NewL(MLlcpConnLessTransporter& aConnection, MLlcpReadWriteCb& aCallBack)
    {
    CLlcpReceiverType1* self = new (ELeave) CLlcpReceiverType1(aConnection, aCallBack);
    CActiveScheduler::Add(self);
    return self;
    }


/*
    Set active for the receiver AO
    Receive complete callback function "cb" registered
*/
TInt CLlcpReceiverType1::Receive()
    {
    BEGIN
    TInt error = KErrNotReady;
    TInt length = 0;
    length = iConnection.SupportedDataLength();
    iReceiveBuf.Zero();
    if (length > 0)
        error = iReceiveBuf.ReAlloc(length);

    if (error == KErrNone)
        {
        iConnection.Receive(iStatus, iReceiveBuf);
        SetActive();
        }

    END
    return error;
    }

void CLlcpReceiverType1::RunL(void)
    {
    BEGIN
    TInt error = iStatus.Int();
     // Call back functions of notifying the llcp receiver completed.
    iReceiveObserver.ReceiveComplete(error);
    END
    }

CLlcpReceiverType1::~CLlcpReceiverType1()
    {
    BEGIN
    // cancel ANY outstanding request at time of destruction
    Cancel();
    iReceiveBuf.Close();
    END
    }

TInt CLlcpReceiverType1::ReceiveDataFromBuf(TDes8& aData)
    {
    BEGIN
    if (iReceiveBuf.Size() == 0)
        return 0;

    TInt requiredLength =  aData.MaxLength() - aData.Length();
    TInt bufLength =  iReceiveBuf.Length();
    TInt readLength = requiredLength < bufLength ? requiredLength : bufLength;

    TPtrC8 ptr(iReceiveBuf.Ptr(),readLength);
    aData.Append(ptr);

    //Empty the buffer as long as receive data request issued.
    iReceiveBuf.Zero();
    END
    return readLength;
    }

bool CLlcpReceiverType1::HasPendingDatagrams() const
    {
    return iReceiveBuf.Size() > 0 ? true : false;
    }

TInt64 CLlcpReceiverType1::PendingDatagramSize() const
    {
    return iReceiveBuf.Size();
    }

void CLlcpReceiverType1::DoCancel(void)
    {
    BEGIN
    // Cancel any outstanding write request on iSocket at this time.
    iConnection.ReceiveCancel();
    END
    }

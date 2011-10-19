/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef LLCPSOCKETTYPE1_SYMBIAN_H_
#define LLCPSOCKETTYPE1_SYMBIAN_H_

#include <e32base.h>
#include <nfcserver.h>                      // RNfcServer
#include <llcpconnlesslistener.h>           // MLlcpConnLessListener

/*
 *   FORWARD DECLARATIONS
 */
class COwnLlcpConnectionWrapper;
class CLlcpSenderType1;
class CLlcpReceiverType1;
class CLlcpTimer;
class CLlcpProvider;
class MLlcpConnLessTransporter;

#include <qconnectivityglobal.h>
#include "../qllcpsocket_symbian_p.h"

class MLlcpReadWriteCb
    {
public:

    /*
     * Empty Destructor.
     */
    virtual ~MLlcpReadWriteCb() {};

    /*
     * Called
     */
    virtual void ReceiveComplete(TInt aError) = 0;
    virtual void WriteComplete(TInt aError, TInt aSize) = 0;
    };

/*
 *  CLASS DECLARATION for CLlcpSocketType1 (ConnectLess Tran).
 */
class CLlcpSocketType1 : public CBase,
                         public MLlcpConnLessListener,
                         public MLlcpReadWriteCb
   {
public:
   /*
    * Creates a new CLlcpSocketType1 object.
    */
   static CLlcpSocketType1* NewL(QLlcpSocketPrivate&);

   /*
    * Creates a new CLlcpSocketType1 object.
    */
   static CLlcpSocketType1* NewLC(QLlcpSocketPrivate&);

   /*
    * Destructor
    */
   ~CLlcpSocketType1();

public:
   TInt StartWriteDatagram(const TDesC8& aData,TUint8 portNum);
   TInt ReadDatagram(TDes8& aData);
   TInt ReadDatagram(TDes8& aData, TUint8& aRemotePortNum);
   TBool Bind(TUint8 portNum);

   /*
       Returns true if at least one datagram is waiting to be read;
       otherwise returns false.
   */
   TBool HasPendingDatagrams() const;
   TInt64 PendingDatagramSize() const;
   TBool WaitForBytesWritten(TInt aMilliSeconds);

private:
   enum TWaitStatus
       {
       ENone,
       EWaitForBytesWritten
       };

private:
   TBool WaitForOperationReady(TWaitStatus aWaitStatus,TInt aMilliSeconds);
   void StopWaitNow(TWaitStatus aWaitStatus);

private:  // from  MLlcpReadWriteCb
     void ReceiveComplete(TInt aError);
     void WriteComplete(TInt aError, TInt aSize);

private: // From MLlcpConnLessListener
    void FrameReceived(MLlcpConnLessTransporter* aConnection);

private:
    // Constructor
    CLlcpSocketType1(QLlcpSocketPrivate&);

    // Second phase constructor
    void ConstructL();
    void Cleanup();

    TInt CreateConnection(TUint8 portNum);
    TInt StartTransportAndReceive(MLlcpConnLessTransporter* aConnection);

private:
   /*
    * Handle to NFC-server.
    * Own.
    */
   RNfcServer iNfcServer;

   /*
    * Pointer to CLlcpProvider object.
    * Own.
    */
   CLlcpProvider* iLlcp;   // Own

   /*
    * Pointer to MLlcpConnLessTransporter object.
    * Own.
    *
    * This is used to send data to local device.
    */
   COwnLlcpConnectionWrapper* iConnectionWrapper;  // Own

   CActiveSchedulerWait * iWait;  //Own
   CLlcpTimer * iTimer;  // Own
   TWaitStatus iWaitStatus;

   bool iPortBinded;
   TUint8 iLocalPort;
   TUint8 iRemotePort;

   QLlcpSocketPrivate& iCallback;
   };

/*
 *  CLASS DECLARATION for COwnLlcpConnectionWrapper.
 *
 */
class COwnLlcpConnectionWrapper : public CBase
    {
public:

   /*
    * Creates a new COwnLlcpConnection object.
    */
   static COwnLlcpConnectionWrapper* NewL(MLlcpConnLessTransporter* aTransporter
                                          , MLlcpReadWriteCb& aCallBack);

   /*
    * Creates a new COwnLlcpConnection object.
    */
   static COwnLlcpConnectionWrapper* NewLC(MLlcpConnLessTransporter* aTransporter
                                           , MLlcpReadWriteCb& aCallBack);

   /*
    * Destructor.
    */
   ~COwnLlcpConnectionWrapper();

public:
   /*
    * Transfer given data to remote device.
    */
   TInt TransferL(const TDesC8& aData);
   bool TransferQueued();
   void TransferCancel();
   TInt Receive();

   /*
    * Cancels COwnLlcpConnection::Receive() request.
    */
   void ReceiveCancel();

   TInt ReceiveDataFromBuf(TDes8& aData);
   bool HasPendingDatagrams() const;
   TInt64 PendingDatagramSize() const;
   bool HasQueuedWrittenDatagram() const;

private:

    // Constructor
    COwnLlcpConnectionWrapper(MLlcpConnLessTransporter* aConnection);
    // Second phase constructor
    void ConstructL(MLlcpReadWriteCb& aCallBack);

private:
    MLlcpConnLessTransporter* iConnection;
    CLlcpSenderType1*      iSenderAO;
    CLlcpReceiverType1*    iReceiverAO;
    RPointerArray<HBufC8> iSendBufArray;
    };


class CLlcpSenderType1 : public CActive
    {
public:
     static CLlcpSenderType1* NewL(MLlcpConnLessTransporter& aConnection, MLlcpReadWriteCb& aCallBack);
    ~CLlcpSenderType1();

public:
    /*
     * Transfer given data to remote device.
     */
    TInt Transfer(const TDesC8& aData);

    /*
     * Cancels COwnLlcpConnection::Transfer() request.
     */
    void TransferCancel();

public: // From CActive
    void RunL();
    void DoCancel();

private:
    CLlcpSenderType1(MLlcpConnLessTransporter& aConnection, MLlcpReadWriteCb& aCallBack);
private:
    /*
         Buffered data for transmitting data.
     */
    MLlcpConnLessTransporter& iConnection;
    MLlcpReadWriteCb& iSendObserver;
    RBuf8 iTransmitBuf;
    // Symbian have limitaion for sending buffer in one send,
    // The variable used to record how many buffer have been sent so far
    TInt   iCurrentSendBufPos;
    TPtrC8 iCurrentSendBufPtr;
    RBuf8  iTempSendBuf; // Temp workround to avoid NFC server's bug, if use ptr, it will crash
    };

class CLlcpReceiverType1 : public CActive
    {
public:
    static CLlcpReceiverType1* NewL(MLlcpConnLessTransporter& aConnection, MLlcpReadWriteCb& aCallBack);
    ~CLlcpReceiverType1();

public:
    /*
     * Starts receive data from ConnLess.
     */
    TInt Receive();

    /*
     * Cancels COwnLlcpConnection::Receive() request.
     */
    void ReceiveCancel();
    TInt ReceiveDataFromBuf(TDes8& aData);

    bool HasPendingDatagrams() const;
    TInt64 PendingDatagramSize() const;

public: // From CActive
    void RunL();
    void DoCancel();

private:
     CLlcpReceiverType1(MLlcpConnLessTransporter& aConnection, MLlcpReadWriteCb& aCallBack);

private:
    RBuf8 iReceiveBuf;
    MLlcpConnLessTransporter& iConnection;
    MLlcpReadWriteCb&         iReceiveObserver;
    };
#endif /* LLCPSOCKETTYPE1_SYMBIAN_H_ */

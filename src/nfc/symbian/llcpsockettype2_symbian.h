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


#ifndef LLCPSOCKETTYPE2_SYMBIAN_H_
#define LLCPSOCKETTYPE2_SYMBIAN_H_

#include <e32base.h>
#include <nfcserver.h>                      // RNfcServer
#include <llcpprovider.h>                   // CLlcpProvider
#include <llcpconnorientedtransporter.h>    // MLlcpConnOrientedTransporter
#include <llcpconnorientedlistener.h>       // MLlcpConnOrientedListener
#include <llcplinklistener.h>               // MLlcpLinkListener
#include <qconnectivityglobal.h>
#include "../qllcpsocket_symbian_p.h"
#include "../qllcpsocket.h"

/*
 *   FORWARD DECLARATIONS
 */
class CLlcpConnecterAO;
class CLlcpSenderAO;
class CLlcpReceiverAO;
class CLlcpTimer;
/*
 *  CLASS DECLARATION for CLlcpSocketType2 (ConnectOriented Transportation).
 */
class CLlcpSocketType2 : public CBase
{
public:
    static CLlcpSocketType2* NewL(QLlcpSocketPrivate* aCallback = NULL);

    static CLlcpSocketType2* NewL(MLlcpConnOrientedTransporter* aTransporter, QLlcpSocketPrivate* aCallback = NULL);
    ~CLlcpSocketType2();

public:
    void ConnectToServiceL( const QString &serviceUri);
    TInt DisconnectFromService();

    TInt StartWriteDatagram(const TDesC8& aData);
    TBool ReceiveData(TDes8& aData);

    TInt64 BytesAvailable();

    //for qt signals
    void Error(QLlcpSocket::SocketError aSocketError);
    void StateChanged(QLlcpSocket::SocketState aSocketState);
    void ReadyRead();
    void BytesWritten(qint64 aBytes);

    TBool WaitForReadyRead(TInt aMilliSeconds);
    TBool WaitForBytesWritten(TInt aMilliSeconds);
    TBool WaitForConnected(TInt aMilliSeconds);

    void AttachCallbackHandler(QLlcpSocketPrivate* aCallback);
private:
    // Constructor
    explicit CLlcpSocketType2(MLlcpConnOrientedTransporter* aTransporter = NULL,QLlcpSocketPrivate* aCallback = NULL);
    // Second phase constructor
    void ConstructL();
    void ConnectToServiceL( const TDesC8& aServiceName);
    enum TWaitStatus
        {
        ENone,
        EWaitForReadyRead,
        EWaitForBytesWritten,
        EWaitForConnected,
        EWaitForDisconnected
        };
    TBool WaitForOperationReady(TWaitStatus aWaitStatus,TInt aSeconds);
    void StopWaitNow(TWaitStatus aWaitStatus);
private:
    friend class CLlcpReceiverAO;
    /*
    * Handle to NFC-server.
    * Own.
    */
    RNfcServer iNfcServer;

    /*
    * Pointer to CLlcpProvider object.
    * Own.
    */
    CLlcpProvider* iLlcp;

    MLlcpConnOrientedTransporter* iTransporter;

    CLlcpConnecterAO* iConnecter;
    CLlcpSenderAO* iSender;
    CLlcpReceiverAO* iReceiver;

    RPointerArray<HBufC8> iReceiveBufArray;
    TInt iBufferOffset;

    CActiveSchedulerWait * iWait;
    TWaitStatus iWaitStatus;
    CLlcpTimer * iTimer;

    QLlcpSocketPrivate*  iCallback; // not own

};

class CLlcpConnecterAO : public CActive
{
public:
    static CLlcpConnecterAO* NewL( MLlcpConnOrientedTransporter& aConnection, CLlcpSocketType2& aSocket );
    ~CLlcpConnecterAO();

public:
    /*
    * Disonnect with remote peer .
    */
    void Disconnect();

    /*
    * Connect to remote peer as given service uri.
    */
    void ConnectL(const TDesC8& aServiceName);

private: // From CActive
    void RunL();
    void DoCancel();

private:
    // Constructor
    CLlcpConnecterAO( MLlcpConnOrientedTransporter& aConnection, CLlcpSocketType2& aSocket );
    // Second phase constructor
    void ConstructL();

private:
    enum TConnectionState
        {
        ENotConnected,
        EConnecting,
        EConnected
        };
    /*
          Pointer to MLlcpConnOrientedTransporter object.
     */
    MLlcpConnOrientedTransporter& iConnection;//Not Own
    CLlcpSocketType2& iSocket;
    /*
          State of LLCP connection object.
     */
    TConnectionState iConnState;
};
class CLlcpSenderAO : public CActive
{
public:
   static CLlcpSenderAO* NewL( MLlcpConnOrientedTransporter& aConnection, CLlcpSocketType2& aSocket );
   ~CLlcpSenderAO();

public:
    /*
    Transfer given data to remote device.
    */
    TInt Send( const TDesC8& aData );

private: // From CActive
    void RunL();
    void DoCancel();

private:
    // Constructor
    CLlcpSenderAO( MLlcpConnOrientedTransporter& aConnection, CLlcpSocketType2& aSocket );
    // Second phase constructor
    void ConstructL();
private:
    void SendRestDataAndSwitchBuffer(RBuf8& aWorkingBuffer, RBuf8& aNextBuffer);

private:
    /*
    Pointer to MLlcpConnOrientedTransporter object.
    */
    MLlcpConnOrientedTransporter& iConnection; //Not Own

    CLlcpSocketType2& iSocket;
    enum TSendBuffer
        {
        EBuffer0,
        EBuffer1
        };
    /*
    * Buffered data for sending data.
    */
    RBuf8 iSendBuf0;
    RBuf8 iSendBuf1;
    TSendBuffer iCurrentBuffer;
    TPtrC8 iCurrentSendPtr;
    TInt iCurrentPos;
    RBuf8 iCurrentSendBuf;
};
class CLlcpReceiverAO : public CActive
{
public:
    static CLlcpReceiverAO* NewL( MLlcpConnOrientedTransporter& aConnection, CLlcpSocketType2& aSocket );
    ~CLlcpReceiverAO();

public:
    /*
    Receive data from remote device.
    */
    TInt StartReceiveDatagram();

private: // From CActive
    void RunL();
    void DoCancel();

private:
    // Constructor
    CLlcpReceiverAO( MLlcpConnOrientedTransporter& aConnection, CLlcpSocketType2& aSocket );
    // Second phase constructor
    void ConstructL();
private:

    /*
    * Pointer to MLlcpConnOrientedTransporter object.
    */
    MLlcpConnOrientedTransporter& iConnection; //Not Own

    CLlcpSocketType2& iSocket;
   /*
    * Buffered data for receiving data.
    */
    RBuf8 iReceiveBuf;
};

#endif /* LLCPSOCKETTYPE2_SYMBIAN_H_ */

/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "llcpserver_symbian.h"
#include "llcpsockettype2_symbian.h"
#include "../qllcpserver_symbian_p.h"
#include "nearfieldutility_symbian.h"

#include "debug.h"

/*
    CLlcpServer::NewL()
*/
CLlcpServer* CLlcpServer::NewL(QLlcpServerPrivate& aCallback)
    {
    BEGIN
    CLlcpServer* self = new (ELeave) CLlcpServer(aCallback);
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    END
    return self;
    }

/*
    CLlcpServer::CLlcpServer()
*/
CLlcpServer::CLlcpServer(QLlcpServerPrivate& aCallback)
     :iLlcp( NULL ),
     iSocketListening(EFalse),
     iCallback(aCallback)
    {
    }

/*
    CLlcpServer::ContructL()
*/
void CLlcpServer::ConstructL()
    {
    BEGIN
    User::LeaveIfError(iNfcServer.Open());
    iLlcp = CLlcpProvider::NewL( iNfcServer );
    END
    }

/*
    Destroys the LLCP socket.
*/
CLlcpServer::~CLlcpServer()
    {
    BEGIN
    StopListening();
    iLlcpSocketArray.ResetAndDestroy();//this should destroy before iLlcp
    iLlcpSocketArray.Close();
    iServiceName.Close();

    delete iLlcp;
    iNfcServer.Close();
    END
    }

/*
    Returns the next pending connection as a connected CLlcpSocketType2
    object.

    The socket is created as a child of the server, which means that
    it is automatically deleted when the CLlcpServer object is
    destroyed. It is still a good idea to delete the object
    explicitly when you are done with it, to avoid wasting memory.
*/
CLlcpSocketType2* CLlcpServer::nextPendingConnection()
    {
    // take first element
    BEGIN
    CLlcpSocketType2 *llcpSocket = NULL;
    if (iLlcpSocketArray.Count() > 0)
        {
        llcpSocket = iLlcpSocketArray[0];
        iLlcpSocketArray.Remove(0);
        }
    END
    return llcpSocket;
    }

TBool CLlcpServer::hasPendingConnections() const
    {
    BEGIN
    END
    return iLlcpSocketArray.Count() > 0 ? ETrue: EFalse;
    }

const TDesC8&  CLlcpServer::serviceUri() const
    {
    BEGIN
    END
    return iServiceName;
    }

/*
    Listen to the LLCP Socket by the URI \a serviceUri .
*/
TBool CLlcpServer::Listen( const TDesC8& aServiceName)
    {
    BEGIN
    TInt error = KErrNone;

    iServiceName.Zero();
    if (iServiceName.Create(aServiceName.Size()) < 0)
        {
        END
        return EFalse;
        }
    iServiceName.Append(aServiceName);

    TRAP(error,iLlcp->StartListeningConnOrientedRequestL( *this, iServiceName ));

    error == KErrNone ? iSocketListening = ETrue : iSocketListening = EFalse;
    END
    return iSocketListening;
    }

void CLlcpServer::StopListening( )
    {
    BEGIN
    if (iSocketListening)
        {
        iLlcp->StopListeningConnOrientedRequest( iServiceName );
        iSocketListening = EFalse;
        iLlcpSocketArray.ResetAndDestroy();
        }
    END
    }


TBool CLlcpServer::isListening() const
    {
    BEGIN
    END
    return iSocketListening;
    }

/*
    Call back from MLlcpConnOrientedListener
*/
void CLlcpServer::RemoteConnectRequest( MLlcpConnOrientedTransporter* aConnection )
    {
    BEGIN
    if (aConnection == NULL)
        {
        END
        return;
        }

    TInt error = KErrNone;

    // create remote connection for the iLlcpsocket
    aConnection->AcceptConnectRequest();

    CLlcpSocketType2 *llcpSocket = NULL;
    TRAP(error,llcpSocket = CLlcpSocketType2::NewL(aConnection));
    // Creating wrapper for connection.
    if (KErrNone == error)
        {
        iLlcpSocketArray.Append(llcpSocket);
        //The newConnection() signal is then emitted each time a client connects to the server.
        TInt error = KErrNone;
        QT_TRYCATCH_ERROR(error, iCallback.invokeNewConnection());
        Q_UNUSED(error);//just skip the error
        }
    END
    }
//EOF

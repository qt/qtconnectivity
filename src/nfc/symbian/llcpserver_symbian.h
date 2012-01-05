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

#ifndef LLCPSERVER_SYMBIAN_H_
#define LLCPSERVER_SYMBIAN_H_

#include <e32base.h>
#include <nfcserver.h>                      // RNfcServer
#include <llcpprovider.h>                   // CLlcpProvider
#include <llcpconnorientedlistener.h>       // MLlcpConnOrientedListener
#include <qconnectivityglobal.h>
#include "../qllcpserver_symbian_p.h"


class CLlcpSocketType2;

class CLlcpServer : public CBase,
                    public MLlcpConnOrientedListener
   {
public:
   /*
    * Creates a new CLlcpServer object.
    */
   static CLlcpServer* NewL(QLlcpServerPrivate&);

   /*
    * Destructor
    */
   ~CLlcpServer();

public:
   TBool Listen( const TDesC8& aServiceName);
   void StopListening();
   TBool isListening() const;
   CLlcpSocketType2 *nextPendingConnection();
   TBool hasPendingConnections() const;
   const TDesC8& serviceUri() const;

private: // From MLlcpConnOrientedListener
    void RemoteConnectRequest( MLlcpConnOrientedTransporter* aConnection );

private:
    // Constructor
    CLlcpServer(QLlcpServerPrivate&);

    // Second phase constructor
    void ConstructL();

private:

    RPointerArray<CLlcpSocketType2>  iLlcpSocketArray;

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

   TBool iSocketListening;

   RBuf8 iServiceName;

   QLlcpServerPrivate& iCallback;
   };

#endif /* LLCPSERVER_SYMBIAN_H_ */

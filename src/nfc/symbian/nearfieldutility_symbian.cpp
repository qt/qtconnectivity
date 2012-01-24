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
#include <ndefmessage.h>
#include <QtCore/QList>
#include <e32base.h>
#include "../qndefmessage.h"
#include "nearfieldutility_symbian.h"
#include "debug.h"

CNdefMessage* QNFCNdefUtility::QNdefMsg2CNdefMsgL( const QNdefMessage& msg )
    {
        QByteArray payload = msg.toByteArray();
        CNdefMessage* cmsg = CNdefMessage::NewL();
        TPtrC8 rawData(reinterpret_cast<const TUint8*>(payload.constData()), payload.size());
        cmsg->ImportRawDataL(rawData, 0);
        return cmsg;
    }


QNdefMessage QNFCNdefUtility::CNdefMsg2QNdefMsgL( const CNdefMessage& msg )
    {
        BEGIN
        QNdefMessage result;
        LOG("CNdefMessage size is "<<msg.SizeL());
        HBufC8* newBuf = HBufC8::NewL(msg.SizeL());
        RBuf8 buf;
        buf.Assign(newBuf);
        buf.CleanupClosePushL();
        LOG("export msg to raw data");
        msg.ExportRawDataL(buf,0);
        LOG("import raw data to msg");
        QByteArray qtArray;
        qtArray.append(reinterpret_cast<const char*>(newBuf->Ptr()),newBuf->Size());
        result = QNdefMessage::fromByteArray(qtArray);
        CleanupStack::PopAndDestroy(&buf);
        END
        return result;
    }

TPtrC8 QNFCNdefUtility::QByteArray2TPtrC8(const QByteArray& qbytearray)
    {
    TPtrC8 ptr(reinterpret_cast<const TUint8*>(qbytearray.constData()), qbytearray.size());
    return ptr;
    }

QByteArray QNFCNdefUtility::TDesC2QByteArray( const TDesC8& des)
    {
        QByteArray result;
        for(int i = 0; i < des.Length(); ++i)
        {
            result.append(des[i]);
        }
        return result;
    }


HBufC8* QNFCNdefUtility::QString2HBufC8L(const QString& qstring)
    {
    TPtrC wide(static_cast<const TUint16*>(qstring.utf16()),qstring.length());
    HBufC8* newBuf = HBufC8::NewL(wide.Length());
    TPtr8 des = newBuf->Des();
    des.Copy(wide);
    return newBuf;
    }

QString QNFCNdefUtility::TDesC82QStringL(const TDesC8& aDescriptor)
    {
    HBufC* newBuf = NULL;
    QT_TRAP_THROWING(newBuf = HBufC::NewL(aDescriptor.Length()));
    TPtr des = newBuf->Des();
    des.Copy(aDescriptor);
    QString ret = QString::fromUtf16(newBuf->Ptr(),newBuf->Length());
    delete newBuf;
    return ret;
    }

// timer implementation
CLlcpTimer* CLlcpTimer::NewL(CActiveSchedulerWait & aWait)
    {
    BEGIN
    CLlcpTimer* self = new (ELeave) CLlcpTimer(aWait);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    END
    return self;
    }

/**
Destructor.
*/
CLlcpTimer::~CLlcpTimer()
    {
    BEGIN
    Cancel();
    END
    }

/**
Starts the shutdown timer.
*/
void CLlcpTimer::Start(TInt aMSecs)
    {
    BEGIN
    const TUint KDelay = (1000 * aMSecs);
    After(KDelay);
    END
    }

void CLlcpTimer::RunL()
    {
    BEGIN
    if (iWait.IsStarted())
        {
        iWait.AsyncStop();
        }
    END
    }

/**
Constructor
*/
CLlcpTimer::CLlcpTimer(CActiveSchedulerWait & aWait) :
    CTimer(EPriorityNormal),
    iWait(aWait)
    {
    }

/**
Second phase constructor.
*/
void CLlcpTimer::ConstructL()
    {
    BEGIN
    CTimer::ConstructL();
    CActiveScheduler::Add(this);
    END
    }



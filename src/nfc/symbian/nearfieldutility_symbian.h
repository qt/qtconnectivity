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

#ifndef QNFCNDEFUTILITY_SYMBIAN_H_
#define QNFCNDEFUTILITY_SYMBIAN_H_
#include <qndefmessage.h>
#include <e32base.h>

class CNdefMessage;

class CLlcpTimer : public CTimer
    {
public:

    static CLlcpTimer* NewL(CActiveSchedulerWait & aWait);
    virtual ~CLlcpTimer();

    void Start(TInt aMSecs);

private: // From CTimer

    void RunL();

private:

    CLlcpTimer(CActiveSchedulerWait & aWait);
    void ConstructL();

private:

    CActiveSchedulerWait& iWait; //not own
    };

class QNdefMessage;
class QNFCNdefUtility
{
public:

    /**
     * Maps between CNdefMessage and QNdefMessage
     *
     */
    static CNdefMessage* QNdefMsg2CNdefMsgL( const QNdefMessage& msg );
    static QNdefMessage CNdefMsg2QNdefMsgL( const CNdefMessage& msg );

    static TPtrC8 QByteArray2TPtrC8(const QByteArray& qbytearray);
    static QByteArray TDesC2QByteArray( const TDesC8& des);

    static HBufC8* QString2HBufC8L(const QString& qstring);
    static QString TDesC82QStringL(const TDesC8&);

};

#endif /* QNFCNDEFUTILITY_SYMBIAN_H_ */

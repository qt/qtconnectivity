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

#ifndef NEARFIELDTAG_SYMBIAN_H
#define NEARFIELDTAG_SYMBIAN_H
#include <nfcserver.h>
#include <e32base.h>
#include <iso14443connection.h>

class MNfcTag;
class MNfcConnection;
class MNearFieldTagOperationCallback;


class CNearFieldTag : public CActive
    {
public:
    // Cancel and destroy
    ~CNearFieldTag();

    // Two-phased constructor.
    static CNearFieldTag* NewLC(MNfcTag * aNfcTag, RNfcServer& aNfcServer);

public:
    CNearFieldTag * CastToTag();
    void SetConnection(MNfcConnection * aTagConnection) { iTagConnection = aTagConnection; }

    TInt OpenConnection();
    void CloseConnection();
    TBool IsConnectionOpened();

    TInt RawModeAccess(const TDesC8& aCommand, TDes8& aResponse, TTimeIntervalMicroSeconds32& aTimeout);

    MNfcConnection * TagConnection() { return iTagConnection;}

    void SetTagOperationCallback(MNearFieldTagOperationCallback * const aCallback);

private:
    // C++ constructor
    CNearFieldTag(MNfcTag * aNfcTag, RNfcServer& aNfcServer);

private:
    void RunL();
    void DoCancel();

private:
    // own
    MNfcConnection * iTagConnection;
    MNfcTag * iNfcTag;
    RNfcServer& iNfcServer;
    // Not own
    MNearFieldTagOperationCallback * iCallback;
    TBool iIsTag4;
    };

#endif // NEARFIELDTAG_SYMBIAN_H

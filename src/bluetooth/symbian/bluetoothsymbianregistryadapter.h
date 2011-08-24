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

#ifndef BLUETOOTHSYMBIANREGISTRYADAPTER_H
#define BLUETOOTHSYMBIANREGISTRYADAPTER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qconnectivityglobal.h>
#include <QtCore/QObject>

#include <e32base.h>
#include <btdevice.h>
#include <bt_sock.h>
#include <btengdevman.h>
#include "qbluetoothlocaldevice.h"

QT_BEGIN_HEADER

class QBluetoothDeviceInfo;
class QBluetoothAddress;

class BluetoothSymbianRegistryAdapter : public QObject, public MBTEngDevManObserver
{
    Q_OBJECT
public:

    explicit BluetoothSymbianRegistryAdapter(const QBluetoothAddress &address, QObject *parent = 0);
    ~BluetoothSymbianRegistryAdapter();

    void removePairing();
    QBluetoothLocalDevice::Pairing pairingStatus();
    int errorCode() const;
    QString pairingErrorString() const;

public: //from MBTEngDevManObserver
    virtual void HandleDevManComplete( TInt aErr );
    virtual void HandleGetDevicesComplete( TInt aErr,CBTDeviceArray* aDeviceArray );


private:
    QBluetoothLocalDevice::Pairing remoteDevicePairingStatus();
    CBTDeviceArray* createDeviceArrayL() const;

Q_SIGNALS: // SIGNALS
    void registryHandlingError(int errorCode);
    void pairingStatusChanged(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing);

private:
    enum Operation {
        NoOp,
        PairingStatus, 
        RemovePairing
    };
private:
    // Symbian registry hander
    CBTEngDevMan *m_bluetoothDeviceManager;
    const QBluetoothAddress &m_address;
    Operation m_operation;

    int m_errorCode;
    QString m_pairingErrorString;
};

QT_END_HEADER

#endif //BLUETOOTHSYMBIANREGISTRYADAPTER_H

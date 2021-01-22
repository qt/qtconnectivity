/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef OSXBTDEVICEPAIR_P_H
#define OSXBTDEVICEPAIR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qbluetoothaddress.h"
#include "osxbtutility_p.h"
#include "osxbluetooth_p.h"

#include <QtCore/qglobal.h>

#include <Foundation/Foundation.h>

@class QT_MANGLE_NAMESPACE(OSXBTPairing);

QT_BEGIN_NAMESPACE

namespace OSXBluetooth {

// C++ delegate.
class PairingDelegate
{
public:
    typedef QT_MANGLE_NAMESPACE(OSXBTPairing) ObjCPairingRequest;

    virtual ~PairingDelegate();

    virtual void connecting(ObjCPairingRequest *pair) = 0;
    virtual void requestPIN(ObjCPairingRequest *pair) = 0;
    virtual void requestUserConfirmation(ObjCPairingRequest *pair,
                                         BluetoothNumericValue) = 0;
    virtual void passkeyNotification(ObjCPairingRequest *pair,
                                     BluetoothPasskey passkey) = 0;
    virtual void error(ObjCPairingRequest *pair, IOReturn errorCode) = 0;
    virtual void pairingFinished(ObjCPairingRequest *pair) = 0;
};

ObjCStrongReference<IOBluetoothDevice> device_with_address(const QBluetoothAddress &address);

} // Namespace OSXBluetooth.

QT_END_NAMESPACE

@interface QT_MANGLE_NAMESPACE(OSXBTPairing) : NSObject<IOBluetoothDevicePairDelegate>

- (id)initWithTarget:(const QBluetoothAddress &)address
      delegate:(QT_PREPEND_NAMESPACE(OSXBluetooth::PairingDelegate) *)object;

- (void)dealloc;

- (IOReturn)start;
- (bool)isActive;
- (void)stop;

- (const QBluetoothAddress &)targetAddress;
- (IOBluetoothDevicePair *)pairingRequest;
- (IOBluetoothDevice *)targetDevice;

// IOBluetoothDevicePairDelegate:

- (void)devicePairingStarted:(id)sender;
- (void)devicePairingConnecting:(id)sender;
- (void)deviceParingPINCodeRequest:(id)sender;

- (void)devicePairingUserConfirmationRequest:(id)sender
        numericValue:(BluetoothNumericValue)numericValue;

- (void)devicePairingUserPasskeyNotification:(id)sender
        passkey:(BluetoothPasskey)passkey;

- (void)devicePairingFinished:(id)sender
        error:(IOReturn)error;

- (void)deviceSimplePairingComplete:(id)sender
        status:(BluetoothHCIEventStatus)status;

@end


#endif

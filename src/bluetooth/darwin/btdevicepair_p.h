/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

#ifndef BTDEVICEPAIR_P_H
#define BTDEVICEPAIR_P_H

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
#include "btdelegates_p.h"
#include "btutility_p.h"

#include <QtCore/qglobal.h>

#include <Foundation/Foundation.h>

#include <IOBluetooth/IOBluetooth.h>

QT_BEGIN_NAMESPACE

namespace DarwinBluetooth {

ObjCStrongReference<IOBluetoothDevice> device_with_address(const QBluetoothAddress &address);

} // Namespace DarwinBluetooth.

QT_END_NAMESPACE

@interface QT_MANGLE_NAMESPACE(DarwinBTClassicPairing) : NSObject<IOBluetoothDevicePairDelegate>

- (id)initWithTarget:(const QBluetoothAddress &)address
      delegate:(QT_PREPEND_NAMESPACE(DarwinBluetooth::PairingDelegate) *)object;

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

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

#ifndef OSXBTSOCKETLISTENER_P_H
#define OSXBTSOCKETLISTENER_P_H

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

#include "osxbluetooth_p.h"

#include <QtCore/qglobal.h>

#include <Foundation/Foundation.h>

// TODO: use the special macros we have to create an
// alias for a mangled name.
@class QT_MANGLE_NAMESPACE(OSXBTSocketListener);

QT_BEGIN_NAMESPACE

namespace DarwinBluetooth {

class SocketListener;

}

QT_END_NAMESPACE

// A single OSXBTSocketListener can be started only once with
// RFCOMM or L2CAP protocol. It must be deleted to stop listening.

@interface QT_MANGLE_NAMESPACE(OSXBTSocketListener) : NSObject

- (id)initWithListener:(QT_PREPEND_NAMESPACE(DarwinBluetooth::SocketListener) *)aDelegate;
- (void)dealloc;

- (bool)listenRFCOMMConnectionsWithChannelID:(BluetoothRFCOMMChannelID)channelID;
- (bool)listenL2CAPConnectionsWithPSM:(BluetoothL2CAPPSM)psm;

- (void)rfcommOpenNotification:(IOBluetoothUserNotification *)notification
        channel:(IOBluetoothRFCOMMChannel *)newChannel;

- (void)l2capOpenNotification:(IOBluetoothUserNotification *)notification
        channel:(IOBluetoothL2CAPChannel *)newChannel;

- (quint16)port;

@end

#endif

// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTCONNECTIONMONITOR_P_H
#define BTCONNECTIONMONITOR_P_H

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

#include <QtCore/qglobal.h>

#include <Foundation/Foundation.h>

#include <IOBluetooth/IOBluetooth.h>

@interface QT_MANGLE_NAMESPACE(DarwinBTConnectionMonitor) : NSObject

- (id)initWithMonitor:(QT_PREPEND_NAMESPACE(DarwinBluetooth::ConnectionMonitor) *)monitor;
- (void)connectionNotification:(id)notification withDevice:(IOBluetoothDevice *)device;
- (void)connectionClosedNotification:(id)notification withDevice:(IOBluetoothDevice *)device;

- (void)stopMonitoring;
@end

#endif

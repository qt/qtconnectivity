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

#ifndef OSXBTCONNECTIONMONITOR_P_H
#define OSXBTCONNECTIONMONITOR_P_H

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
#include "osxbluetooth_p.h"

#include <QtCore/qglobal.h>

#include <Foundation/Foundation.h>

@class QT_MANGLE_NAMESPACE(OSXBTConnectionMonitor);

QT_BEGIN_NAMESPACE

namespace OSXBluetooth {

class ConnectionMonitor {
public:
    typedef QT_MANGLE_NAMESPACE(OSXBTConnectionMonitor) ObjCConnectionMonitor;

    virtual ~ConnectionMonitor();

    virtual void deviceConnected(const QBluetoothAddress &address) = 0;
    virtual void deviceDisconnected(const QBluetoothAddress &address) = 0;
};

}

QT_END_NAMESPACE

@interface QT_MANGLE_NAMESPACE(OSXBTConnectionMonitor) : NSObject

- (id)initWithMonitor:(QT_PREPEND_NAMESPACE(OSXBluetooth::ConnectionMonitor) *)monitor;
- (void)connectionNotification:(id)notification withDevice:(IOBluetoothDevice *)device;
- (void)connectionClosedNotification:(id)notification withDevice:(IOBluetoothDevice *)device;

@end

#endif

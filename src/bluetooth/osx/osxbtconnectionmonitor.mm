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

#include "osxbtconnectionmonitor_p.h"
#include "osxbtutility_p.h"

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

namespace OSXBluetooth {

ConnectionMonitor::~ConnectionMonitor()
{
}

}

QT_END_NAMESPACE

#ifdef QT_NAMESPACE
using namespace QT_NAMESPACE;
#endif

@implementation QT_MANGLE_NAMESPACE(OSXBTConnectionMonitor)
{
    QT_PREPEND_NAMESPACE(OSXBluetooth::ConnectionMonitor) *monitor;
    IOBluetoothUserNotification *discoveryNotification;
    NSMutableArray *foundConnections;
}

- (id)initWithMonitor:(OSXBluetooth::ConnectionMonitor *)aMonitor
{
    Q_ASSERT_X(aMonitor, "-initWithMonitor:", "invalid monitor (null)");

    if (self = [super init]) {
        monitor = aMonitor;
        discoveryNotification = [[IOBluetoothDevice registerForConnectNotifications:self
                                  selector:@selector(connectionNotification:withDevice:)] retain];
        foundConnections = [[NSMutableArray alloc] init];
    }

    return self;
}

- (void)dealloc
{
    Q_ASSERT_X(!monitor, "-dealloc",
               "Connection monitor was not stopped, calling -stopMonitoring is required");
    [super dealloc];
}

- (void)connectionNotification:(IOBluetoothUserNotification *)aNotification
        withDevice:(IOBluetoothDevice *)device
{
    Q_UNUSED(aNotification)

    typedef IOBluetoothUserNotification Notification;

    if (!device)
        return;

    QT_BT_MAC_AUTORELEASEPOOL;

    // All Obj-C objects are autoreleased.

    const QBluetoothAddress deviceAddress(OSXBluetooth::qt_address([device getAddress]));
    if (deviceAddress.isNull())
        return;

    if (foundConnections) {
        Notification *const notification = [device registerForDisconnectNotification:self
                                            selector: @selector(connectionClosedNotification:withDevice:)];
        if (notification)
            [foundConnections addObject:notification];
    }

    Q_ASSERT_X(monitor, "-connectionNotification:withDevice:", "invalid monitor (null)");
    monitor->deviceConnected(deviceAddress);
}

- (void)connectionClosedNotification:(IOBluetoothUserNotification *)notification
        withDevice:(IOBluetoothDevice *)device
{
    QT_BT_MAC_AUTORELEASEPOOL;

    [notification unregister];//?
    [foundConnections removeObject:notification];

    const QBluetoothAddress deviceAddress(OSXBluetooth::qt_address([device getAddress]));
    if (deviceAddress.isNull())
        return;

    Q_ASSERT_X(monitor, "-connectionClosedNotification:withDevice:", "invalid monitor (null)");
    monitor->deviceDisconnected(deviceAddress);
}

- (void)stopMonitoring
{
    monitor = nullptr;
    [discoveryNotification unregister];
    [discoveryNotification release];
    discoveryNotification = nil;

    for (IOBluetoothUserNotification *n in foundConnections)
        [n unregister];

    [foundConnections release];
    foundConnections = nil;
}

@end

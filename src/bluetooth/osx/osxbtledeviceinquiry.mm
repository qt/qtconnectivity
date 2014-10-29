/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "osxbtcentralmanagerdelegate_p.h"
#include "osxbtledeviceinquiry_p.h"
#include "qbluetoothdeviceinfo.h"
#include "qbluetoothuuid.h"
#include "osxbtutility_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qdebug.h>

// Foundation header is already included by this point
// (a workaround for a broken 10.9 SDK).
#include "corebluetoothwrapper_p.h"

QT_BEGIN_NAMESPACE

namespace OSXBluetooth {

LEDeviceInquiryDelegate::~LEDeviceInquiryDelegate()
{
}

// TODO: check versions!
#if QT_MAC_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_9, __IPHONE_6_0)

QBluetoothUuid qt_uuid(NSUUID *nsUuid)
{
    if (!nsUuid)
        return QBluetoothUuid();

    uuid_t uuidData = {};
    [nsUuid getUUIDBytes:uuidData];
    quint128 qtUuidData = {};
    std::copy(uuidData, uuidData + 16, qtUuidData.data);
    return QBluetoothUuid(qtUuidData);
}

#else

QBluetoothUuid qt_uuid(CFUUIDRef uuid)
{
    if (!uuid)
        return QBluetoothUuid();

    const CFUUIDBytes data = CFUUIDGetUUIDBytes(uuid);
    quint128 qtUuidData = {{data.byte0, data.byte1, data.byte2, data.byte3,
                           data.byte4, data.byte5, data.byte6, data.byte7,
                           data.byte8, data.byte9, data.byte10, data.byte11,
                           data.byte12, data.byte13, data.byte14, data.byte15}};

    return QBluetoothUuid(qtUuidData);
}

typedef ObjCStrongReference<NSString> StringStrongReference;

StringStrongReference uuid_as_nsstring(CFUUIDRef uuid)
{
    // We use the UUDI's string representation as a key in a dictionary.
    if (!uuid)
        return StringStrongReference();

    CFStringRef cfStr = CFUUIDCreateString(kCFAllocatorDefault, uuid);
    if (!cfStr)
        return StringStrongReference();

    // Imporant: with ARC this will require a different cast/ownership!
    return StringStrongReference((NSString *)cfStr, false);
}

#endif

}


QT_END_NAMESPACE

#ifdef QT_NAMESPACE

using namespace QT_NAMESPACE;

#endif

@interface QT_MANGLE_NAMESPACE(OSXBTLEDeviceInquiry) (PrivateAPI) <CBCentralManagerDelegate, CBPeripheralDelegate>
+ (NSTimeInterval)inquiryLength;
// "Timeout" callback to stop a scan.
- (void)stopScan;
@end

@implementation QT_MANGLE_NAMESPACE(OSXBTLEDeviceInquiry)

+ (NSTimeInterval)inquiryLength
{
    // 10 seconds at the moment. There is no default 'time-out',
    // CBCentralManager startScan does not stop if not asked.
    return 10;
}

- (id)initWithDelegate:(OSXBluetooth::LEDeviceInquiryDelegate *)aDelegate
{
    Q_ASSERT_X(aDelegate, "-initWithWithDelegate:", "invalid delegate (null)");

    if (self = [super init]) {
        delegate = aDelegate;
        peripherals = [[NSMutableDictionary alloc] init];
        manager = nil;
        pendingStart = false;
        cancelled = false;
        isActive = false;
    }

    return self;
}

- (void)dealloc
{
    typedef QT_MANGLE_NAMESPACE(OSXBTCentralManagerTransientDelegate) TransientDelegate;

    [NSObject cancelPreviousPerformRequestsWithTarget:self];

    if (manager) {
        // -start was called.
        if (pendingStart) {
            // State was not updated yet, too early to release.
            TransientDelegate *const transient = [[TransientDelegate alloc] initWithManager:manager];
            // On ARC the lifetime of a transient delegate will become a problem, since delegate itself
            // is a weak reference in a manager.
            [manager setDelegate:transient];
        } else {
            [manager setDelegate:nil];
            if (isActive)
                [manager stopScan];
            [manager release];
        }
    }

    [peripherals release];
    [super dealloc];
}

- (void)stopScan
{
    // Scan's timeout.
    Q_ASSERT_X(delegate, "-stopScan", "invalid delegate (null)");
    Q_ASSERT_X(manager, "-stopScan", "invalid central (nil)");
    Q_ASSERT_X(!pendingStart, "-stopScan", "invalid state");
    Q_ASSERT_X(!cancelled, "-stopScan", "invalid state");
    Q_ASSERT_X(isActive, "-stopScan", "invalid state");

    [manager setDelegate:nil];
    [manager stopScan];
    isActive = false;

    delegate->LEdeviceInquiryFinished();
}

- (bool)start
{
    Q_ASSERT_X(![self isActive], "-start", "LE device scan is already active");
    Q_ASSERT_X(delegate, "-start", "invalid delegate (null)");

    if (!peripherals) {
        qCCritical(QT_BT_OSX) << "-start, internal error (allocation problem)";
        return false;
    }

    cancelled = false;
    [peripherals removeAllObjects];

    if (manager) {
        // We can never be here, if status was not updated yet.
        [manager setDelegate:nil];
        [manager release];
    }

    pendingStart = true;
    manager = [CBCentralManager alloc];
    manager = [manager initWithDelegate:self queue:nil];
    if (!manager) {
        qCCritical(QT_BT_OSX) << "-start, failed to create a central manager";
        return false;
    }

    return true;
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    Q_ASSERT_X(delegate, "-centralManagerDidUpdateState:", "invalid delegate (null)");

    if (cancelled) {
        Q_ASSERT_X(!isActive, "-centralManagerDidUpdateState:", "isActive is true");
        pendingStart = false;
        delegate->LEdeviceInquiryFinished();
        return;
    }

    const CBCentralManagerState state = central.state;
    if (state == CBCentralManagerStatePoweredOn) {
        if (pendingStart) {
            pendingStart = false;
            isActive = true;
            [self performSelector:@selector(stopScan) withObject:nil
                  afterDelay:[QT_MANGLE_NAMESPACE(OSXBTLEDeviceInquiry) inquiryLength]];
            [manager scanForPeripheralsWithServices:nil options:nil];
        } // Else we ignore.
    } else if (state == CBCentralManagerStateUnsupported || state == CBCentralManagerStateUnauthorized) {
        if (pendingStart) {
            pendingStart = false;
            delegate->LEnotSupported();
        } else if (isActive) {
            // It's not clear if this thing can happen at all.
            // We had LE supported and now .. not anymore?
            // Report as an error.
            [NSObject cancelPreviousPerformRequestsWithTarget:self];
            isActive = false;
            [manager stopScan];
            delegate->LEdeviceInquiryError(QBluetoothDeviceDiscoveryAgent::PoweredOffError);
        }
    } else if (state == CBCentralManagerStatePoweredOff) {
        if (pendingStart) {
            pendingStart = false;
            delegate->LEnotSupported();
        } else if (isActive) {
            // We were able to start (isActive == true), so we had
            // powered ON and now the adapter is OFF.
            [NSObject cancelPreviousPerformRequestsWithTarget:self];
            isActive = false;
            [manager stopScan];
            delegate->LEdeviceInquiryError(QBluetoothDeviceDiscoveryAgent::PoweredOffError);
        } // Else we ignore.
    } else {
        // The following two states we ignore (from Apple's docs):
        //"
        // -CBCentralManagerStateUnknown
        // The current state of the central manager is unknown;
        // an update is imminent.
        //
        // -CBCentralManagerStateResetting
        // The connection with the system service was momentarily
        // lost; an update is imminent. "
        //
        // TODO: check if "is imminent" means UpdateState will
        // be called again with something more reasonable.
    }
}

- (void)stop
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];

    if (pendingStart || cancelled) {
        // We have to wait for a status update.
        cancelled = true;
        return;
    }

    if (isActive) {
        [manager stopScan];
        isActive = false;
        delegate->LEdeviceInquiryFinished();
    }
}

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral
        advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI
{
    Q_UNUSED(central)
    Q_UNUSED(advertisementData)

    using namespace OSXBluetooth;

    Q_ASSERT_X(delegate, "-centralManager:didDiscoverPeripheral:advertisementData:RSSI:",
               "invalid delegate (null)");
    Q_ASSERT_X(isActive, "-centralManager:didDiscoverPeripheral:advertisementData:RSSI:",
               "called while there is no active scan");
    Q_ASSERT_X(!pendingStart, "-centralManager:didDiscoverPeripheral:advertisementData:RSSI:",
               "both pendingStart and isActive are true");


#if QT_MAC_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_9, __IPHONE_6_0)
    if (!peripheral.identifier) {
        qCWarning(QT_BT_OSX) << "-centramManager:didDiscoverPeripheral:advertisementData:RSSI:, "
                                "peripheral without NSUUID";
        return;
    }

    if (![peripherals objectForKey:peripheral.identifier]) {
        [peripherals setObject:peripheral forKey:peripheral.identifier];
        const QBluetoothUuid deviceUuid(OSXBluetooth::qt_uuid(peripheral.identifier));
        delegate->LEdeviceFound(peripheral, deviceUuid, advertisementData, RSSI);
    }
#else
    if (!peripheral.UUID) {
        qCWarning(QT_BT_OSX) << "-centramManager:didDiscoverPeripheral:advertisementData:RSSI:, "
                                "peripheral without UUID";
        return;
    }

    StringStrongReference key(uuid_as_nsstring(peripheral.UUID));
    if (![peripherals objectForKey:key.data()]) {
        [peripherals setObject:peripheral forKey:key.data()];
        const QBluetoothUuid deviceUuid(OSXBluetooth::qt_uuid(peripheral.UUID));
        delegate->LEdeviceFound(peripheral, deviceUuid, advertisementData, RSSI);
    }
#endif

}

- (bool)isActive
{
    return pendingStart || isActive;
}

@end

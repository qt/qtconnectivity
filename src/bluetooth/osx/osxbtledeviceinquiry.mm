/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "osxbtledeviceinquiry_p.h"
#include "qbluetoothdeviceinfo.h"
#include "qbluetoothuuid.h"
#include "osxbtutility_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qsysinfo.h>
#include <QtCore/qdebug.h>

#include "corebluetoothwrapper_p.h"

QT_BEGIN_NAMESPACE

namespace OSXBluetooth {

LEDeviceInquiryDelegate::~LEDeviceInquiryDelegate()
{
}

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

#endif

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

}


QT_END_NAMESPACE

#ifdef QT_NAMESPACE

using namespace QT_NAMESPACE;

#endif

@interface QT_MANGLE_NAMESPACE(OSXBTLEDeviceInquiry) (PrivateAPI) <CBCentralManagerDelegate, CBPeripheralDelegate>
// "Timeout" callback to stop a scan.
- (void)stopScan;
- (void)handlePoweredOffAfterDelay;
@end

@implementation QT_MANGLE_NAMESPACE(OSXBTLEDeviceInquiry)

+ (int)inquiryLength
{
    // There is no default timeout,
    // scan does not stop if not asked.
    // Return in milliseconds
    return 10 * 1000;
}

- (id)initWithDelegate:(OSXBluetooth::LEDeviceInquiryDelegate *)aDelegate
{
    Q_ASSERT_X(aDelegate, Q_FUNC_INFO, "invalid delegate (null)");

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
    [NSObject cancelPreviousPerformRequestsWithTarget:self];

    if (manager) {
        [manager setDelegate:nil];
        if (isActive)
            [manager stopScan];
        [manager release];
    }

    [peripherals release];
    [super dealloc];
}

- (void)stopScan
{
    // Scan's timeout.
    Q_ASSERT_X(delegate, Q_FUNC_INFO, "invalid delegate (null)");
    Q_ASSERT_X(manager, Q_FUNC_INFO, "invalid central (nil)");
    Q_ASSERT_X(!pendingStart, Q_FUNC_INFO, "invalid state");
    Q_ASSERT_X(!cancelled, Q_FUNC_INFO, "invalid state");
    Q_ASSERT_X(isActive, Q_FUNC_INFO, "invalid state");

    [manager setDelegate:nil];
    [manager stopScan];
    isActive = false;

    delegate->LEdeviceInquiryFinished();
}

- (void)handlePoweredOffAfterDelay
{
    // If we are here, this means:
    // we received 'PoweredOff' while pendingStart == true
    // and no 'PoweredOn' after this.

    Q_ASSERT_X(delegate, Q_FUNC_INFO, "invalid delegate (null)");
    Q_ASSERT_X(pendingStart, Q_FUNC_INFO, "invalid state");

    pendingStart = false;
    if (cancelled) {
        // Timeout happened before
        // the second status update, but after 'stop'.
        delegate->LEdeviceInquiryFinished();
    } else {
        // Timeout and not 'stop' between 'start'
        // and 'DidUpdateStatus':
        delegate->LEnotSupported();
    }
}

- (bool)start
{
    Q_ASSERT_X(![self isActive], Q_FUNC_INFO, "LE device scan is already active");
    Q_ASSERT_X(delegate, Q_FUNC_INFO, "invalid delegate (null)");

    if (!peripherals) {
        qCCritical(QT_BT_OSX) << Q_FUNC_INFO << "internal error";
        return false;
    }

    cancelled = false;
    [peripherals removeAllObjects];

    if (manager) {
        // We can never be here, if status was not updated yet.
        [manager setDelegate:nil];
        [manager release];
    }

    startTime = QTime();
    pendingStart = true;
    manager = [CBCentralManager alloc];
    manager = [manager initWithDelegate:self queue:nil];
    if (!manager) {
        qCCritical(QT_BT_OSX) << Q_FUNC_INFO << "failed to create a central manager";
        return false;
    }

    return true;
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    Q_ASSERT_X(delegate, Q_FUNC_INFO, "invalid delegate (null)");

    const CBCentralManagerState state = central.state;

    if (pendingStart && (state == CBCentralManagerStatePoweredOn
                         || state == CBCentralManagerStateUnsupported
                         || state == CBCentralManagerStateUnauthorized
                         || state == CBCentralManagerStatePoweredOff)) {
        // We probably had 'PoweredOff' before,
        // cancel the previous handlePoweredOffAfterDelay.
        [NSObject cancelPreviousPerformRequestsWithTarget:self];
    }

    if (cancelled) {
        Q_ASSERT_X(!isActive, Q_FUNC_INFO, "isActive is true");
        pendingStart = false;
        delegate->LEdeviceInquiryFinished();
        return;
    }

    if (state == CBCentralManagerStatePoweredOn) {
        if (pendingStart) {
            pendingStart = false;
            isActive = true;
#ifndef Q_OS_OSX
            const NSTimeInterval timeout([QT_MANGLE_NAMESPACE(OSXBTLEDeviceInquiry) inquiryLength] / 1000);
            Q_ASSERT_X(timeout > 0., Q_FUNC_INFO, "invalid scan timeout");
            [self performSelector:@selector(stopScan) withObject:nil afterDelay:timeout];
#endif
            startTime = QTime::currentTime();
            [manager scanForPeripheralsWithServices:nil options:nil];
        } // Else we ignore.
    } else if (state == CBCentralManagerStateUnsupported || state == CBCentralManagerStateUnauthorized) {
        if (pendingStart) {
            pendingStart = false;
            delegate->LEnotSupported();
        } else if (isActive) {
            // Cancel stopScan:
            [NSObject cancelPreviousPerformRequestsWithTarget:self];

            isActive = false;
            [manager stopScan];
            delegate->LEdeviceInquiryError(QBluetoothDeviceDiscoveryAgent::PoweredOffError);
        }
    } else if (state == CBCentralManagerStatePoweredOff) {
        if (pendingStart) {
#ifndef Q_OS_OSX
            // On iOS a user can see at this point an alert asking to enable
            // Bluetooth in the "Settings" app. If a user does,
            // we'll receive 'PoweredOn' state update later.
            [self performSelector:@selector(handlePoweredOffAfterDelay) withObject:nil afterDelay:30.];
            return;
#endif
            pendingStart = false;
            delegate->LEnotSupported();
        } else if (isActive) {
            // Cancel stopScan:
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
    }
}

- (void)stop
{
    if (!pendingStart) {
        // pendingStart == true means either no selector at all,
        // or handlePoweredOffAfter delay and we do not want to cancel it yet,
        // waiting for DidUpdateState or handlePoweredOffAfter, whoever
        // fires first ...
        [NSObject cancelPreviousPerformRequestsWithTarget:self];
    }

    if (pendingStart || cancelled) {
        // We have to wait for a status update or handlePoweredOffAfterDelay.
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

    if (!isActive)
        return;

    Q_ASSERT_X(delegate, Q_FUNC_INFO, "invalid delegate (null)");

#if QT_MAC_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_9, __IPHONE_7_0)
    if (QSysInfo::MacintoshVersion >= qt_OS_limit(QSysInfo::MV_10_9, QSysInfo::MV_IOS_7_0)) {
        if (!peripheral.identifier) {
            qCWarning(QT_BT_OSX) << Q_FUNC_INFO << "peripheral without NSUUID";
            return;
        }

        if (![peripherals objectForKey:peripheral.identifier]) {
            [peripherals setObject:peripheral forKey:peripheral.identifier];
            const QBluetoothUuid deviceUuid(OSXBluetooth::qt_uuid(peripheral.identifier));
            delegate->LEdeviceFound(peripheral, deviceUuid, advertisementData, RSSI);
        }
        return;
    }
#endif
    // Either SDK or the target is below 10.9/7.0:
    // The property UUID was finally removed in iOS 9, we have
    // to avoid compilation errors ...
    CFUUIDRef cfUUID = Q_NULLPTR;

    if ([peripheral respondsToSelector:@selector(UUID)]) {
        // This will require a bridged cast if we switch to ARC ...
        cfUUID = reinterpret_cast<CFUUIDRef>([peripheral performSelector:@selector(UUID)]);
    }

    if (!cfUUID) {
        qCWarning(QT_BT_OSX) << Q_FUNC_INFO << "peripheral without CFUUID";
        return;
    }

    StringStrongReference key(uuid_as_nsstring(cfUUID));
    if (![peripherals objectForKey:key.data()]) {
        [peripherals setObject:peripheral forKey:key.data()];
        const QBluetoothUuid deviceUuid(OSXBluetooth::qt_uuid(cfUUID));
        delegate->LEdeviceFound(peripheral, deviceUuid, advertisementData, RSSI);
    }
}

- (bool)isActive
{
    return pendingStart || isActive;
}

- (const QTime&)startTime
{
    return startTime;
}

@end

/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
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

@interface QT_MANGLE_NAMESPACE(OSXBTLEDeviceInquiry) (PrivateAPI) <CBCentralManagerDelegate>
- (void)stopScan;
- (void)handlePoweredOff;
@end

@implementation QT_MANGLE_NAMESPACE(OSXBTLEDeviceInquiry)

- (id)init
{
    if (self = [super init]) {
        uuids.reset([[NSMutableSet alloc] init]);
        internalState = InquiryStarting;
        state.store(int(internalState));
    }

    return self;
}

- (void)dealloc
{
    if (manager) {
        [manager setDelegate:nil];
        if (internalState == InquiryActive)
            [manager stopScan];
    }

    [super dealloc];
}

- (void)stopScan
{
    // Scan's "timeout" - we consider LE device
    // discovery finished.
    using namespace OSXBluetooth;

    if (internalState == InquiryActive) {
        if (scanTimer.elapsed() >= qt_LE_deviceInquiryLength() * 1000) {
            // We indeed stop now:
            [manager stopScan];
            [manager setDelegate:nil];
            internalState = InquiryFinished;
            state.store(int(internalState));
        } else {
            dispatch_queue_t leQueue(qt_LE_queue());
            Q_ASSERT(leQueue);
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW,
                                         int64_t(qt_LE_deviceInquiryLength() / 100. * NSEC_PER_SEC)),
                                         leQueue,
                                         ^{
                                               [self stopScan];
                                          });
        }
    }
}

- (void)handlePoweredOff
{
    // This is interesting on iOS only, where
    // the system shows an alert asking to enable
    // Bluetooth in the 'Settings' app. If not done yet (after 30
    // seconds) - we consider it an error.
    if (internalState == InquiryStarting) {
        if (errorTimer.elapsed() >= 30000) {
            [manager setDelegate:nil];
            internalState = ErrorPoweredOff;
            state.store(int(internalState));
        } else {
            dispatch_queue_t leQueue(OSXBluetooth::qt_LE_queue());
            Q_ASSERT(leQueue);
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW,
                                         (int64_t)(30 / 100. * NSEC_PER_SEC)),
                                         leQueue,
                                         ^{
                                              [self handlePoweredOff];
                                          });

        }
    }
}

- (void)start
{
    dispatch_queue_t leQueue(OSXBluetooth::qt_LE_queue());

    Q_ASSERT(leQueue);
    manager.reset([[CBCentralManager alloc] initWithDelegate:self queue:leQueue]);
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    if (central != manager)
        return;

    if (internalState != InquiryActive && internalState != InquiryStarting)
        return;

    using namespace OSXBluetooth;

    dispatch_queue_t leQueue(qt_LE_queue());
    Q_ASSERT(leQueue);

    const CBCentralManagerState cbState(central.state);
    if (cbState == CBCentralManagerStatePoweredOn) {
        if (internalState == InquiryStarting) {
            internalState = InquiryActive;
            // Scan time is actually 10 seconds. Having a block with such delay can prevent
            // 'self' from being deleted in time, which is not good. So we split this
            // 10 s. timeout into smaller 'chunks'.
            scanTimer.start();
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW,
                                         int64_t(qt_LE_deviceInquiryLength() / 100. * NSEC_PER_SEC)),
                                         leQueue,
                                         ^{
                                               [self stopScan];
                                          });
            [manager scanForPeripheralsWithServices:nil options:nil];
        } // Else we ignore.
    } else if (state == CBCentralManagerStateUnsupported || state == CBCentralManagerStateUnauthorized) {
        if (internalState == InquiryActive) {
            [manager stopScan];
            // Not sure how this is possible at all, probably, can never happen.
            internalState = ErrorPoweredOff;
        } else {
            internalState = ErrorLENotSupported;
        }

        [manager setDelegate:nil];
    } else if (cbState == CBCentralManagerStatePoweredOff) {
        if (internalState == InquiryStarting) {
#ifndef Q_OS_OSX
            // On iOS a user can see at this point an alert asking to enable
            // Bluetooth in the "Settings" app. If a user does,
            // we'll receive 'PoweredOn' state update later.
            // No change in state. Wait for 30 seconds (we split it into 'chunks' not
            // to retain 'self' for too long ) ...
            errorTimer.start();
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW,
                                         (int64_t)(30 / 100. * NSEC_PER_SEC)),
                                         leQueue,
                                         ^{
                                              [self handlePoweredOff];
                                          });
            return;
#endif
            internalState = ErrorPoweredOff;
        } else {
            internalState = ErrorPoweredOff;
            [manager stopScan];
        }

        [manager setDelegate:nil];
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
        // Wait for this imminent update.
    }

    state.store(int(internalState));
}

- (void)stop
{
    if (internalState == InquiryActive)
        [manager stopScan];

    [manager setDelegate:nil];
    internalState = InquiryCancelled;
    state.store(int(internalState));
}

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral
        advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI
{
    Q_UNUSED(advertisementData);

    using namespace OSXBluetooth;

    if (central != manager)
        return;

    if (internalState != InquiryActive)
        return;

    QBluetoothUuid deviceUuid;

#if QT_MAC_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_9, __IPHONE_7_0)
    if (QSysInfo::MacintoshVersion >= qt_OS_limit(QSysInfo::MV_10_9, QSysInfo::MV_IOS_7_0)) {
        if (!peripheral.identifier) {
            qCWarning(QT_BT_OSX) << Q_FUNC_INFO << "peripheral without NSUUID";
            return;
        }

        if ([uuids containsObject:peripheral.identifier]) {
            // We already know this peripheral ...
            return;
        }

        [uuids addObject:peripheral.identifier];
        deviceUuid = OSXBluetooth::qt_uuid(peripheral.identifier);
    }
#endif
    // Either SDK or the target is below 10.9/7.0:
    // The property UUID was finally removed in iOS 9, we have
    // to avoid compilation errors ...
    if (deviceUuid.isNull()) {
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
        if ([uuids containsObject:key.data()])
            return; // We've seen this peripheral before ...
        [uuids addObject:key.data()];
        deviceUuid = OSXBluetooth::qt_uuid(cfUUID);
    }

    if (deviceUuid.isNull()) {
        qCWarning(QT_BT_OSX) << Q_FUNC_INFO << "no way to address peripheral, QBluetoothUuid is null";
        return;
    }

    QString name;
    if (peripheral.name)
        name = QString::fromNSString(peripheral.name);

    // TODO: fix 'classOfDevice' (0 for now).
    QBluetoothDeviceInfo newDeviceInfo(deviceUuid, name, 0);
    if (RSSI)
        newDeviceInfo.setRssi([RSSI shortValue]);
    // CoreBluetooth scans only for LE devices.
    newDeviceInfo.setCoreConfigurations(QBluetoothDeviceInfo::LowEnergyCoreConfiguration);
    devices.append(newDeviceInfo);
}

- (LEInquiryState) inquiryState
{
    return LEInquiryState(state.load());
}

- (const QList<QBluetoothDeviceInfo> &)discoveredDevices
{
    return devices;
}

@end

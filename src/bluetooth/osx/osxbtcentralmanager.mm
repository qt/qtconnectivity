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

#include "qlowenergyserviceprivate_p.h"
#include "qlowenergycharacteristic.h"
#include "osxbtcentralmanager_p.h"


#include <QtCore/qloggingcategory.h>
#include <QtCore/qdebug.h>

#include <algorithm>
#include <limits>

QT_BEGIN_NAMESPACE

namespace OSXBluetooth {

CentralManagerDelegate::~CentralManagerDelegate()
{
}

NSUInteger qt_countGATTEntries(CBService *service)
{
    // Identify, how many characteristics/descriptors we have on a given service,
    // +1 for the service itself.
    // No checks if NSUInteger is big enough :)
    // Let's assume such number of entries is not possible :)

    Q_ASSERT_X(service, "qt_countGATTEntries", "invalid service (nil)");

    QT_BT_MAC_AUTORELEASEPOOL;

    NSArray *const cs = service.characteristics;
    if (!cs || !cs.count)
        return 1;

    NSUInteger n = 1 + cs.count;
    for (CBCharacteristic *c in cs) {
        NSArray *const ds = c.descriptors;
        if (ds)
            n += ds.count;
    }

    return n;
}

}

QT_END_NAMESPACE


#ifdef QT_NAMESPACE
using namespace QT_NAMESPACE;
#endif

@interface QT_MANGLE_NAMESPACE(OSXBTCentralManager) (PrivateAPI)

- (QLowEnergyController::Error)connectToDevice; // "Device" is in Qt's world ...
- (void)connectToPeripheral; // "Peripheral" is in Core Bluetooth.
- (void)discoverIncludedServices;
- (void)readCharacteristics:(CBService *)service;
- (void)serviceDetailsDiscoveryFinished:(CBService *)service;

// Aux. functions.
- (CBService *)serviceForUUID:(const QBluetoothUuid &)qtUuid;
- (CBCharacteristic *)nextCharacteristicForService:(CBService*)service
                      startingFrom:(CBCharacteristic *)from;
- (CBCharacteristic *)nextCharacteristicForService:(CBService*)service
                      startingFrom:(CBCharacteristic *)from
                      withProperties:(CBCharacteristicProperties)properties;
- (CBDescriptor *)nextDescriptorForCharacteristic:(CBCharacteristic *)characteristic
                  startingFrom:(CBDescriptor *)descriptor;

@end

@implementation QT_MANGLE_NAMESPACE(OSXBTCentralManager)

- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(OSXBluetooth)::CentralManagerDelegate *)aDelegate
{
    Q_ASSERT_X(aDelegate, "-initWithDelegate:", "invalid delegate (null)");

    if (self = [super init]) {
        manager = nil;
        managerState = OSXBluetooth::CentralManagerIdle;
        disconnectPending = false;
        peripheral = nil;
        delegate = aDelegate;
        currentService = 0;
        lastValidHandle = 0;
    }

    return self;
}

- (void)dealloc
{
    // In the past I had a 'transient delegate': I've seen some crashes
    // while deleting a manager _before_ its state updated.
    // Strangely enough, I can not reproduce this anymore, so this
    // part is simplified now. To be investigated though.

    visitedServices.reset(nil);
    servicesToVisit.reset(nil);
    servicesToVisitNext.reset(nil);

    [manager setDelegate:nil];
    [manager release];

    [peripheral setDelegate:nil];
    [peripheral release];

    [super dealloc];
}

- (QLowEnergyController::Error)connectToDevice:(const QBluetoothUuid &)aDeviceUuid
{
    Q_ASSERT_X(managerState == OSXBluetooth::CentralManagerIdle, "-connectToDevice",
               "invalid state"); // QLowEnergyController connects only if in UnconnectedState.

    deviceUuid = aDeviceUuid;

    if (!manager) {
        managerState = OSXBluetooth::CentralManagerUpdating; // We'll have to wait for updated state.
        manager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
        if (!manager) {
            managerState = OSXBluetooth::CentralManagerIdle;
            qCWarning(QT_BT_OSX) << "-connectToDevice:, failed to allocate a "
                                    "central manager";
            return QLowEnergyController::ConnectionError;
        }
        return QLowEnergyController::NoError;
    } else {
        return [self connectToDevice];
    }
}

- (QLowEnergyController::Error)connectToDevice
{
    Q_ASSERT_X(manager, "-connectToDevice", "invalid central manager (nil)");
    Q_ASSERT_X(managerState == OSXBluetooth::CentralManagerIdle,
               "-connectToDevice", "invalid state");

    if ([self isConnected]) {
        qCDebug(QT_BT_OSX) << "-connectToDevice, already connected";
        delegate->connectSuccess();
        return QLowEnergyController::NoError;
    } else if (peripheral) {
        // Was retrieved already, but not connected
        // or disconnected.
        [self connectToPeripheral];
        return QLowEnergyController::NoError;
    }

    using namespace OSXBluetooth;

    // Retrieve a peripheral first ...
    ObjCScopedPointer<NSMutableArray> uuids([[NSMutableArray alloc] init]);
    if (!uuids) {
        qCWarning(QT_BT_OSX) << "-connectToDevice, failed to allocate identifiers";
        return QLowEnergyController::ConnectionError;
    }

#if QT_MAC_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_9, __IPHONE_6_0)
    const quint128 qtUuidData(deviceUuid.toUInt128());
    // STATIC_ASSERT on sizes would be handy!
    uuid_t uuidData = {};
    std::copy(qtUuidData.data, qtUuidData.data + 16, uuidData);
    const ObjCScopedPointer<NSUUID> nsUuid([[NSUUID alloc] initWithUUIDBytes:uuidData]);
    if (!nsUuid) {
        qCWarning(QT_BT_OSX) << "-connectToDevice, failed to allocate NSUUID identifier";
        return QLowEnergyController::ConnectionError;
    }

    [uuids addObject:nsUuid];

    // With the latest CoreBluetooth, we can synchronously retrive peripherals:
    QT_BT_MAC_AUTORELEASEPOOL;
    NSArray *const peripherals = [manager retrievePeripheralsWithIdentifiers:uuids];
    if (!peripherals || peripherals.count != 1) {
        qCWarning(QT_BT_OSX) << "-connectToDevice, failed to retrive a peripheral";
        return QLowEnergyController::UnknownRemoteDeviceError;
    }

    peripheral = [static_cast<CBPeripheral *>([peripherals objectAtIndex:0]) retain];
    [self connectToPeripheral];

    return QLowEnergyController::NoError;
#else
    OSXBluetooth::CFStrongReference<CFUUIDRef> cfUuid(OSXBluetooth::cf_uuid(deviceUuid));
    if (!cfUuid) {
        qCWarning(QT_BT_OSX) << "-connectToDevice:, failed to create CFUUID object";
        return QLowEnergyController::ConnectionError;
    }
    // TODO: With ARC this cast will be illegal:
    [uuids addObject:(id)cfUuid.data()];
    // Unfortunately, with old Core Bluetooth this call is asynchronous ...
    managerState = OSXBluetooth::CentralManagerConnecting;
    [manager retrievePeripherals:uuids];

    return QLowEnergyController::NoError;
#endif
}

- (void)connectToPeripheral
{
    Q_ASSERT_X(manager, "-connectToPeripheral", "invalid central manager (nil)");
    Q_ASSERT_X(peripheral, "-connectToPeripheral", "invalid peripheral (nil)");
    Q_ASSERT_X(managerState == OSXBluetooth::CentralManagerIdle,
               "-connectToPeripheral", "invalid state");

    // The state is still the same - connecting.
    if ([self isConnected]) {
        qCDebug(QT_BT_OSX) << "-connectToPeripheral, already connected";
        delegate->connectSuccess();
    } else {
        qCDebug(QT_BT_OSX) << "-connectToPeripheral, trying to connect";
        managerState = OSXBluetooth::CentralManagerConnecting;
        [manager connectPeripheral:peripheral options:nil];
    }
}

- (bool)isConnected
{
    if (!peripheral)
        return false;

#if QT_MAC_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_9, __IPHONE_7_0)
    return peripheral.state == CBPeripheralStateConnected;
#else
    return peripheral.isConnected;
#endif
}

- (void)disconnectFromDevice
{
    servicesToDiscoverDetails.clear();

    if (managerState == OSXBluetooth::CentralManagerUpdating) {
        disconnectPending = true;
    } else {
        disconnectPending = false;

        if ([self isConnected])
            managerState = OSXBluetooth::CentralManagerDisconnecting;
        else
            managerState = OSXBluetooth::CentralManagerIdle;

        // We have to call -cancelPeripheralConnection: even
        // if not connected (to cancel a pending connect attempt).
        // Unfortunately, didDisconnect callback is not always called
        // (despite of Apple's docs saying it _must_).
        [manager cancelPeripheralConnection:peripheral];
    }
}

- (void)discoverServices
{
    Q_ASSERT_X(peripheral, "-discoverServices", "invalid peripheral (nil)");
    Q_ASSERT_X(managerState == OSXBluetooth::CentralManagerIdle,
               "-discoverServices", "invalid state");

    // From Apple's docs:
    //
    //"If the servicesUUIDs parameter is nil, all the available
    //services of the peripheral are returned; setting the
    //parameter to nil is considerably slower and is not recommended."
    //
    // ... but we'd like to have them all:

    lastValidHandle = 0;
    serviceMap.clear();
    charMap.clear();
    descMap.clear();

    [peripheral setDelegate:self];
    managerState = OSXBluetooth::CentralManagerDiscovering;
    [peripheral discoverServices:nil];
}

- (void)discoverIncludedServices
{
    using namespace OSXBluetooth;

    Q_ASSERT_X(managerState == CentralManagerIdle, "-discoverIncludedServices",
               "invalid state");
    Q_ASSERT_X(manager, "-discoverIncludedServices", "invalid manager (nil)");
    Q_ASSERT_X(peripheral, "-discoverIncludedServices", "invalid peripheral (nil)");

    QT_BT_MAC_AUTORELEASEPOOL;

    NSArray *const services = peripheral.services;
    if (!services || !services.count) { // Actually, !services.count works in both cases, but ...
        // A peripheral without any services at all.
        Q_ASSERT_X(delegate, "-discoverIncludedServices",
                   "invalid delegate (null)");
        delegate->serviceDiscoveryFinished(ObjCStrongReference<NSArray>());
    } else {
        // 'reset' also calls retain on a parameter.
        servicesToVisitNext.reset(nil);
        servicesToVisit.reset([NSMutableArray arrayWithArray:services]);
        currentService = 0;
        visitedServices.reset([NSMutableSet setWithCapacity:peripheral.services.count]);

        CBService *const s = [services objectAtIndex:currentService];
        [visitedServices addObject:s];
        managerState = CentralManagerDiscovering;
        [peripheral discoverIncludedServices:nil forService:s];
    }
}

- (bool)discoverServiceDetails:(const QBluetoothUuid &)serviceUuid
{
    // This function does not change 'managerState', since it
    // can be called concurrently (not waiting for the previous
    // discovery to finish).

    using namespace OSXBluetooth;

    Q_ASSERT_X(managerState != CentralManagerUpdating, "-discoverServiceDetails:",
               "invalid state");
    Q_ASSERT_X(!serviceUuid.isNull(), "-discoverServiceDetails:",
               "invalid service UUID");
    Q_ASSERT_X(peripheral, "-discoverServiceDetailsl:",
               "invalid peripheral (nil)");

    if (servicesToDiscoverDetails.contains(serviceUuid)) {
        qCWarning(QT_BT_OSX) << "-discoverServiceDetails: "
                                "already discovering for " << serviceUuid;
        return true;
    }

    QT_BT_MAC_AUTORELEASEPOOL;

    if (CBService *const service = [self serviceForUUID:serviceUuid]) {
        servicesToDiscoverDetails.append(serviceUuid);
        [peripheral discoverCharacteristics:nil forService:service];
        return true;
    }

    qCWarning(QT_BT_OSX) << "-discoverServiceDetails:, invalid service - "
                            "unknown uuid " << serviceUuid;

    return false;
}

- (void)readCharacteristics:(CBService *)service
{
    // This method does not change 'managerState', we can
    // have several 'detail discoveries' active.
    Q_ASSERT_X(service, "-readCharacteristics:", "invalid service (nil)");

    using namespace OSXBluetooth;

    QT_BT_MAC_AUTORELEASEPOOL;

    Q_ASSERT_X(managerState != CentralManagerUpdating, "-readCharacteristics:",
               "invalid state");
    Q_ASSERT_X(manager, "-readCharacteristics:", "invalid manager (nil)");
    Q_ASSERT_X(peripheral, "-readCharacteristics:", "invalid peripheral (nil)");
    Q_ASSERT_X(delegate, "-readCharacteristics:", "invalid delegate (null)");

    if (!service.characteristics || !service.characteristics.count)
        return [self serviceDetailsDiscoveryFinished:service];

    NSArray *const cs = service.characteristics;
    for (CBCharacteristic *c in cs) {
        if (c.properties & CBCharacteristicPropertyRead)
            return [peripheral readValueForCharacteristic:c];
    }

    // No readable properties? Discover descriptors then:
    [self discoverDescriptors:service];
}

- (void)discoverDescriptors:(CBService *)service
{
    // This method does not change 'managerState', we can have
    // several discoveries active.
    Q_ASSERT_X(service, "-discoverDescriptors:", "invalid service (nil)");

    using namespace OSXBluetooth;

    QT_BT_MAC_AUTORELEASEPOOL;

    Q_ASSERT_X(managerState != CentralManagerUpdating, "-discoverDescriptors",
               "invalid state");
    Q_ASSERT_X(manager, "-discoverDescriptors:", "invalid manager (nil)");
    Q_ASSERT_X(peripheral, "-discoverDescriptors:", "invalid peripheral (nil)");

    if (!service.characteristics || !service.characteristics.count) {
        [self serviceDetailsDiscoveryFinished:service];
    } else {
        // Start from 0 and continue in the callback.
        [peripheral discoverDescriptorsForCharacteristic:[service.characteristics objectAtIndex:0]];
    }
}

- (void)readDescriptors:(CBService *)service
{
    Q_ASSERT_X(service, "-readDescriptors:",
               "invalid service (nil)");
    Q_ASSERT_X(managerState != OSXBluetooth::CentralManagerUpdating,
               "-readDescriptors:",
               "invalid state");
    Q_ASSERT_X(peripheral, "-readDescriptors:",
               "invalid peripheral (nil)");

    QT_BT_MAC_AUTORELEASEPOOL;

    NSArray *const cs = service.characteristics;
    // We can never be here if we have no characteristics.
    Q_ASSERT_X(cs && cs.count, "-readDescriptors:",
               "invalid service");
    for (CBCharacteristic *c in cs) {
        if (c.descriptors && c.descriptors.count)
            return [peripheral readValueForDescriptor:[c.descriptors objectAtIndex:0]];
    }

    // No descriptors to read, done.
    [self serviceDetailsDiscoveryFinished:service];
}

- (void)serviceDetailsDiscoveryFinished:(CBService *)service
{
    //
    Q_ASSERT_X(service, "-serviceDetailsDiscoveryFinished:",
               "invalid service (nil)");
    Q_ASSERT_X(delegate, "-serviceDetailsDiscoveryFinished:",
               "invalid delegate (null)");

    using namespace OSXBluetooth;

    QT_BT_MAC_AUTORELEASEPOOL;

    const QBluetoothUuid serviceUuid(qt_uuid(service.UUID));
    servicesToDiscoverDetails.removeAll(serviceUuid);

    const NSUInteger nHandles = qt_countGATTEntries(service);
    Q_ASSERT_X(nHandles, "-serviceDetailsDiscoveryFinished:",
               "unexpected number of GATT entires");

    const QLowEnergyHandle maxHandle = std::numeric_limits<QLowEnergyHandle>::max();
    if (nHandles >= maxHandle || lastValidHandle > maxHandle - nHandles) {
        // Well, that's unlikely :) But we must be sure.
        qCWarning(QT_BT_OSX) << "-serviceDetailsDiscoveryFinished:",
                                "can not allocate more handles";
        // TODO: add more 'error' functions not to use fake handles (0)?
        delegate->error(serviceUuid, 0, QLowEnergyService::OperationError);
        return;
    }

    // A temporary service object to pass the details.
    // Set only uuid, characteristics and descriptors (and probably values),
    // nothing else is needed.
    QSharedPointer<QLowEnergyServicePrivate> qtService(new QLowEnergyServicePrivate);
    qtService->uuid = serviceUuid;
    // We 'register' handles/'CBentities' even if qlowenergycontroller (delegate)
    // later fails to do this with some error. Otherwise, if we try to implement
    // rollback/transaction logic interface is getting too ugly/complicated.
    ++lastValidHandle;
    serviceMap[lastValidHandle] = service;
    qtService->startHandle = lastValidHandle;

    NSArray *const cs = service.characteristics;
    // Now map chars/descriptors and handles.
    if (cs && cs.count) {
        QHash<QLowEnergyHandle, QLowEnergyServicePrivate::CharData> charList;

        for (CBCharacteristic *c in cs) {
            ++lastValidHandle;
            // Register this characteristic:
            charMap[lastValidHandle] = c;
            // Create a Qt's internal characteristic:
            QLowEnergyServicePrivate::CharData newChar = {};
            newChar.uuid = qt_uuid(c.UUID);
            const int cbProps = c.properties & 0xff;
            newChar.properties = static_cast<QLowEnergyCharacteristic::PropertyTypes>(cbProps);
            newChar.value = qt_bytearray(c.value);
            newChar.valueHandle = lastValidHandle;

            NSArray *const ds = c.descriptors;
            if (ds && ds.count) {
                QHash<QLowEnergyHandle, QLowEnergyServicePrivate::DescData> descList;
                for (CBDescriptor *d in ds) {
                    // Register this descriptor:
                    ++lastValidHandle;
                    descMap[lastValidHandle] = d;
                    // Create a Qt's internal descriptor:
                    QLowEnergyServicePrivate::DescData newDesc = {};
                    newDesc.uuid = qt_uuid(d.UUID);
                    newDesc.value = qt_bytearray(d.value);
                    descList[lastValidHandle] = newDesc;
                }

                newChar.descriptorList = descList;
            }

            charList[newChar.valueHandle] = newChar;
        }

        qtService->characteristicList = charList;
    }

    qtService->endHandle = lastValidHandle;

    ObjCStrongReference<CBService> leService(service, true);
    delegate->serviceDetailsDiscoveryFinished(qtService);
}

- (bool)write:(const QT_PREPEND_NAMESPACE(QByteArray) &)value
        charHandle:(QT_PREPEND_NAMESPACE(QLowEnergyHandle))charHandle
        withResponse:(bool)withResponse
{
    using namespace OSXBluetooth;

    Q_ASSERT_X(charHandle, "-write:charHandle:withResponse:", "invalid characteristic handle (0)");

    QT_BT_MAC_AUTORELEASEPOOL;

    if (!charMap.contains(charHandle)) {
        qCWarning(QT_BT_OSX) << "-write:charHandle:withResponse:, "
                                "characteristic: " << charHandle << " not found";
        return false;
    }

    CBCharacteristic *const ch = charMap.value(charHandle);
    Q_ASSERT_X(ch, "-write:charHandle:withResponse:", "invalid characteristic (nil) for a give handle");
    Q_ASSERT_X(peripheral, "-write:charHandle:withResponse:", "invalid peripheral (nil)");

    ObjCStrongReference<NSData> data(data_from_bytearray(value));
    if (!data) {
        // Even if qtData.size() == 0, we still need NSData object.
        qCWarning(QT_BT_OSX) << "-write:charHandle:withResponse:, "
                                "failed to allocate NSData object";
        return false;
    }

    // TODO: check what happens if I'm using NSData with length 0.
    [peripheral writeValue:data.data() forCharacteristic:ch
                type: withResponse? CBCharacteristicWriteWithResponse : CBCharacteristicWriteWithoutResponse];

    return true;
}

// Aux. methods:

- (CBService *)serviceForUUID:(const QBluetoothUuid &)qtUuid
{
    using namespace OSXBluetooth;

    Q_ASSERT_X(!qtUuid.isNull(), "-serviceForUUID:",
               "invalid uuid");
    Q_ASSERT_X(peripheral, "-serviceForUUID:",
               "invalid peripherla (nil)");

    ObjCStrongReference<NSMutableArray> toVisit([NSMutableArray arrayWithArray:peripheral.services], true);
    ObjCStrongReference<NSMutableArray> toVisitNext([[NSMutableArray alloc] init], false);
    ObjCStrongReference<NSMutableSet> visitedNodes([[NSMutableSet alloc] init], false);

    while (true) {
        for (NSUInteger i = 0, e = [toVisit count]; i < e; ++i) {
            CBService *const s = [toVisit objectAtIndex:i];
            if (equal_uuids(s.UUID, qtUuid))
                return s;
            if ([visitedNodes containsObject:s] && s.includedServices && s.includedServices.count) {
                [visitedNodes addObject:s];
                [toVisitNext addObjectsFromArray:s.includedServices];
            }
        }

        if (![toVisitNext count])
            return nil;

        toVisit.resetWithoutRetain(toVisitNext.take());
        toVisitNext.resetWithoutRetain([[NSMutableArray alloc] init]);
    }

    return nil;
}

- (CBCharacteristic *)nextCharacteristicForService:(CBService*)service
                      startingFrom:(CBCharacteristic *)characteristic
{
    Q_ASSERT_X(service, "-nextCharacteristicForService:startingFrom:",
               "invalid service (nil)");
    Q_ASSERT_X(characteristic, "-nextCharacteristicForService:startingFrom:",
               "invalid characteristic (nil)");
    Q_ASSERT_X(service.characteristics, "-nextCharacteristicForService:startingFrom:",
               "invalid service");
    Q_ASSERT_X(service.characteristics.count, "-nextCharacteristicForService:startingFrom:",
               "invalid service");

    QT_BT_MAC_AUTORELEASEPOOL;

    // TODO: test that we NEVER have the same characteristic twice in array!
    // At the moment I just protect against this by iterating in a reverse
    // order (at least avoiding a potential inifite loop with '-indexOfObject:').
    NSArray *const cs = service.characteristics;
    if (cs.count == 1)
        return nil;

    for (NSUInteger index = cs.count - 1; index != 0; --index) {
        if ([cs objectAtIndex:index] == characteristic) {
            if (index + 1 == cs.count)
                return nil;
            else
                return [cs objectAtIndex:index + 1];
        }
    }

    Q_ASSERT_X([cs objectAtIndex:0] == characteristic,
               "-nextCharacteristicForService:startingFrom:",
               "characteristic was not found in service.characteristics");

    return [cs objectAtIndex:1];
}

- (CBCharacteristic *)nextCharacteristicForService:(CBService*)service
                      startingFrom:(CBCharacteristic *)characteristic
                      properties:(CBCharacteristicProperties)properties
{
    Q_ASSERT_X(service, "-nextCharacteristicForService:startingFrom:properties:",
               "invalid service (nil)");
    Q_ASSERT_X(characteristic, "-nextCharacteristicForService:startingFrom:properties:",
               "invalid characteristic (nil)");
    Q_ASSERT_X(service.characteristics, "-nextCharacteristicForService:startingFrom:properties:",
               "invalid service");
    Q_ASSERT_X(service.characteristics.count, "-nextCharacteristicForService:startingFrom:properties:",
               "invalid service");

    QT_BT_MAC_AUTORELEASEPOOL;

    // TODO: test that we NEVER have the same characteristic twice in array!
    // At the moment I just protect against this by iterating in a reverse
    // order (at least avoiding a potential inifite loop with '-indexOfObject:').
    NSArray *const cs = service.characteristics;
    if (cs.count == 1)
        return nil;

    NSUInteger index = cs.count - 1;
    for (; index != 0; --index) {
        if ([cs objectAtIndex:index] == characteristic) {
            if (index + 1 == cs.count) {
                return nil;
            } else {
                index += 1;
                break;
            }
        }
    }

    if (!index) {
        Q_ASSERT_X([cs objectAtIndex:0] == characteristic,
                   "-nextCharacteristicForService:startingFrom:properties:",
                   "characteristic not found in service.characteristics");
        index = 1;
    }

    for (const NSUInteger e = cs.count; index < e; ++index) {
        CBCharacteristic *const c = [cs objectAtIndex:index];
        if (c.properties & properties)
            return c;
    }

    return nil;
}

- (CBDescriptor *)nextDescriptorForCharacteristic:(CBCharacteristic *)characteristic
                  startingFrom:(CBDescriptor *)descriptor
{
    Q_ASSERT_X(characteristic, "-nextDescriptorForCharacteristic:startingFrom:",
               "invalid characteristic (nil)");
    Q_ASSERT_X(descriptor, "-nextDescriptorForCharacteristic:startingFrom:",
               "invalid descriptor (nil)");
    Q_ASSERT_X(characteristic.descriptors,
               "-nextDescriptorForCharacteristic:startingFrom:",
               "invalid characteristic");
    Q_ASSERT_X(characteristic.descriptors.count,
               "-nextDescriptorForCharacteristic:startingFrom:",
               "invalid characteristic");

    QT_BT_MAC_AUTORELEASEPOOL;

    NSArray *const ds = characteristic.descriptors;
    if (ds.count == 1)
        return nil;

    for (NSUInteger index = ds.count - 1; index != 0; --index) {
        if ([ds objectAtIndex:index] == descriptor) {
            if (index + 1 == ds.count)
                return nil;
            else
                return [ds objectAtIndex:index + 1];
        }
    }

    Q_ASSERT_X([ds objectAtIndex:0] == descriptor,
               "-nextDescriptorForCharacteristic:startingFrom:",
               "descriptor was not found in characteristic.descriptors");

    return [ds objectAtIndex:1];
}

// CBCentralManagerDelegate (the real one).

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    Q_ASSERT_X(delegate, "-centralManagerDidUpdateState:", "invalid delegate (null)");

    using namespace OSXBluetooth;

    const CBCentralManagerState state = central.state;

    if (state == CBCentralManagerStateUnknown
        || state == CBCentralManagerStateResetting) {
        // We still have to wait, docs say:
        // "The current state of the central manager is unknown;
        // an update is imminent." or
        // "The connection with the system service was momentarily
        // lost; an update is imminent."
        return;
    }

    if (disconnectPending) {
        disconnectPending = false;
        managerState = OSXBluetooth::CentralManagerIdle;
        return [self disconnectFromDevice];
    }

    // Let's check some states we do not like first:
    if (state == CBCentralManagerStateUnsupported || state == CBCentralManagerStateUnauthorized) {
        if (managerState == CentralManagerUpdating) {
            // We tried to connect just to realize, LE is not supported. Report this.
            managerState = CentralManagerIdle;
            delegate->LEnotSupported();
        } else {
            // TODO: if we are here, LE _was_ supported and we first managed to update
            // and reset managerState from CentralManagerUpdating.
            managerState = CentralManagerIdle;
            delegate->error(QLowEnergyController::InvalidBluetoothAdapterError);
        }
        return;
    }

    if (state == CBCentralManagerStatePoweredOff) {
        if (managerState == CentralManagerUpdating) {
            // I've seen this instead of Unsopported on OS X.
            managerState = CentralManagerIdle;
            delegate->LEnotSupported();
        } else {
            managerState = CentralManagerIdle;
            // TODO: we need a better error +
            // what will happen if later the state changes to PoweredOn???
            delegate->error(QLowEnergyController::InvalidBluetoothAdapterError);
        }
        return;
    }

    if (state == CBCentralManagerStatePoweredOn) {
        if (managerState == CentralManagerUpdating) {
            managerState = CentralManagerIdle;
            const QLowEnergyController::Error status = [self connectToDevice];
            if (status != QLowEnergyController::NoError)// An allocation problem?
                delegate->error(status);
        }
    } else {
        // We actually handled all known states, but .. Core Bluetooth can change?
        Q_ASSERT_X(0, "-centralManagerDidUpdateState:", "invalid centra's state");
    }
}

- (void)centralManager:(CBCentralManager *)central didRetrievePeripherals:(NSArray *)peripherals
{
    Q_UNUSED(central)

    // This method is required for iOS before 7.0 and OS X below 10.9.
    Q_ASSERT_X(manager, "-centralManager:didRetrivePeripherals:", "invalid central manager (nil)");

    if (managerState != OSXBluetooth::CentralManagerConnecting) {
        // Canceled by calling -disconnectFromDevice method.
        return;
    }

    managerState = OSXBluetooth::CentralManagerIdle;

    if (!peripherals || peripherals.count != 1) {
        Q_ASSERT_X(delegate, "-centralManager:didRetrievePeripherals:",
                   "invalid delegate (null)");

        qCDebug(QT_BT_OSX) << "-centralManager:didRetrievePeripherals:, "
                              "unexpected number of peripherals (!= 1)";

        delegate->error(QLowEnergyController::UnknownRemoteDeviceError);
    } else {
        peripheral = [static_cast<CBPeripheral *>([peripherals objectAtIndex:0]) retain];
        [self connectToPeripheral];
    }
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)aPeripheral
{
    Q_UNUSED(central)
    Q_UNUSED(aPeripheral)

    Q_ASSERT_X(delegate, "-centralManager:didConnectPeripheral:",
               "invalid delegate (null)");

    if (managerState != OSXBluetooth::CentralManagerConnecting) {
        // We called cancel but before disconnected, managed to connect?
        return;
    }

    managerState = OSXBluetooth::CentralManagerIdle;
    delegate->connectSuccess();
}

- (void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)aPeripheral
        error:(NSError *)error
{
    Q_UNUSED(central)
    Q_UNUSED(aPeripheral)
    Q_UNUSED(error)

    Q_ASSERT_X(delegate, "-centralManager:didFailToConnectPeripheral:",
               "invalid delegate (null)");

    if (managerState != OSXBluetooth::CentralManagerConnecting) {
        // Canceled already.
        return;
    }

    managerState = OSXBluetooth::CentralManagerIdle;
    // TODO: better error mapping is required.
    delegate->error(QLowEnergyController::UnknownRemoteDeviceError);
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)aPeripheral
        error:(NSError *)error
{
    Q_UNUSED(central)
    Q_UNUSED(aPeripheral)

    Q_ASSERT_X(delegate, "-centralManager:didDisconnectPeripheral:error:",
               "invalid delegate (null)");

    if (error && managerState == OSXBluetooth::CentralManagerDisconnecting) {
        managerState = OSXBluetooth::CentralManagerIdle;
        qCWarning(QT_BT_OSX) << "-centralManager:didDisconnectPeripheral:, "
                                "failed to disconnect";
        // TODO: instead of 'unknown' - .. ?
        delegate->error(QLowEnergyController::UnknownRemoteDeviceError);
    } else {
        managerState = OSXBluetooth::CentralManagerIdle;
        delegate->disconnected();
    }
}

// CBPeripheralDelegate.

- (void)peripheral:(CBPeripheral *)aPeripheral didDiscoverServices:(NSError *)error
{
    Q_UNUSED(aPeripheral)

    if (managerState != OSXBluetooth::CentralManagerDiscovering) {
        // Canceled by -disconnectFromDevice.
        return;
    }

    managerState = OSXBluetooth::CentralManagerIdle;

    if (error) {
        // NSLog, not qCDebug/Warning - to print the error.
        NSLog(@"-peripheral:didDiscoverServices:, failed with error %@", error);
        // TODO: better error mapping required.
        delegate->error(QLowEnergyController::UnknownError);
    } else {
        [self discoverIncludedServices];
    }
}


- (void)peripheral:(CBPeripheral *)aPeripheral didDiscoverIncludedServicesForService:(CBService *)service
        error:(NSError *)error
{
    Q_UNUSED(aPeripheral)

    using namespace OSXBluetooth;

    if (managerState != CentralManagerDiscovering) {
        // Canceled by disconnectFromDevice or -peripheralDidDisconnect...
        return;
    }

    QT_BT_MAC_AUTORELEASEPOOL;

    Q_ASSERT_X(delegate, "-peripheral:didDiscoverIncludedServicesForService:",
               "invalid delegate (null)");
    Q_ASSERT_X(peripheral, "-peripheral:didDiscoverIncludedServicesForService:",
               "invalid peripheral (nil)");
    // TODO: asserts on other "pointers" ...

    managerState = CentralManagerIdle;

    if (error) {
        // NSLog, not qCWarning/Critical - to log the actual NSError and service UUID.
        NSLog(@"-peripheral:didDiscoverIncludedServicesForService:, finished with error %@ for service %@",
              error, service.UUID);
    } else if (service.includedServices && service.includedServices.count) {
        // Now we have even more services to do included services discovery ...
        if (!servicesToVisitNext)
            servicesToVisitNext.reset([NSMutableArray arrayWithArray:service.includedServices]);
        else
            [servicesToVisitNext addObjectsFromArray:service.includedServices];
    }

    // Do we have something else to discover on this 'level'?
    ++currentService;

    for (const NSUInteger e = [servicesToVisit count]; currentService < e; ++currentService) {
        CBService *const s = [servicesToVisit objectAtIndex:currentService];
        if (![visitedServices containsObject:s]) {
            // Continue with discovery ...
            [visitedServices addObject:s];
            managerState = CentralManagerDiscovering;
            return [peripheral discoverIncludedServices:nil forService:s];
        }
    }

    // No services to visit more on this 'level'.

    if (servicesToVisitNext && [servicesToVisitNext count]) {
        servicesToVisit.resetWithoutRetain(servicesToVisitNext.take());

        currentService = 0;
        for (const NSUInteger e = [servicesToVisit count]; currentService < e; ++currentService) {
            CBService *const s = [servicesToVisit objectAtIndex:currentService];
            if (![visitedServices containsObject:s]) {
                [visitedServices addObject:s];
                managerState = CentralManagerDiscovering;
                return [peripheral discoverIncludedServices:nil forService:s];
            }
        }
    }

    // Finally, if we're here, the service discovery is done!

    // Release all these things now, no need to prolong their lifetime.
    visitedServices.reset(nil);
    servicesToVisit.reset(nil);
    servicesToVisitNext.reset(nil);

    const ObjCStrongReference<NSArray> services(peripheral.services, true); // true == retain.
    delegate->serviceDiscoveryFinished(services);
}

- (void)peripheral:(CBPeripheral *)aPeripheral didDiscoverCharacteristicsForService:(CBService *)service
        error:(NSError *)error
{
    // This method does not change 'managerState', we can have several
    // discoveries active.
    Q_UNUSED(aPeripheral)

    // TODO: check that this can never be called after cancelPeripheralConnection was executed.

    using namespace OSXBluetooth;

    Q_ASSERT_X(managerState != CentralManagerUpdating,
               "-peripheral:didDiscoverCharacteristicsForService:",
               "invalid state");
    Q_ASSERT_X(delegate, "-peripheral:didDiscoverCharacteristicsForService:",
               "invalid delegate (null)");

    if (error) {
        // NSLog to show the actual NSError (can contain something interesting).
        NSLog(@"-peripheral:didDiscoverCharacteristicsForService:error, failed with error: %@",
              error);
        // We did not discover any characteristics and can not discover descriptors,
        // inform our delegate (it will set a service state also).
        delegate->error(qt_uuid(service.UUID), QLowEnergyController::UnknownError);
    } else {
        [self readCharacteristics:service];
    }
}

- (void)peripheral:(CBPeripheral *)aPeripheral didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic
        error:(NSError *)error
{
    Q_UNUSED(aPeripheral)

    using namespace OSXBluetooth;

    Q_ASSERT_X(managerState != CentralManagerUpdating,
               "-peripheral:didUpdateValueForCharacteristic:error:",
               "invalid state");
    Q_ASSERT_X(peripheral, "-peripheral:didUpdateValueForCharacteristic:error:",
               "invalid peripheral (nil)");

    QT_BT_MAC_AUTORELEASEPOOL;
    // First, let's check if we're discovering a service details now.
    CBService *const service = characteristic.service;
    const QBluetoothUuid qtUuid(qt_uuid(service.UUID));
    const bool isDetailsDiscovery = servicesToDiscoverDetails.contains(qtUuid);

    if (error) {
        // Use NSLog, not qCDebug/qCWarning to log the actual error.
        NSLog(@"-peripheral:didUpdateValueForCharacteristic:error:, failed with error %@",
              error);

        if (!isDetailsDiscovery) {
            // TODO: this can be something else in a future (if needed at all).
            return;
        }
    }

    if (isDetailsDiscovery) {
        // Test if we have any other characteristic to read yet.
        CBCharacteristic *const next = [self nextCharacteristicForService:characteristic.service
                                             startingFrom:characteristic properties:CBCharacteristicPropertyRead];
        if (next)
            [peripheral readValueForCharacteristic:next];
        else
            [self discoverDescriptors:characteristic.service];
    } else {
        // TODO: this can be something else in a future (if needed at all).
    }
}

- (void)peripheral:(CBPeripheral *)aPeripheral
        didDiscoverDescriptorsForCharacteristic:(CBCharacteristic *)characteristic
        error:(NSError *)error
{
    // This method does not change 'managerState', we can
    // have several discoveries active at the same time.
    Q_UNUSED(aPeripheral)

    QT_BT_MAC_AUTORELEASEPOOL;

    using namespace OSXBluetooth;

    if (error) {
        // Log the error using NSLog:
        NSLog(@"-peripheral:didDiscoverDescriptorsForCharacteristic:error:, failed with error %@",
              error);
        // Probably, we can continue though ...
    }

    // Do we have more characteristics on this service to discover descriptors?
    CBCharacteristic *const next = [self nextCharacteristicForService:characteristic.service
                                         startingFrom:characteristic];
    if (next)
        [peripheral discoverDescriptorsForCharacteristic:next];
    else
        [self readDescriptors:characteristic.service];
}

- (void)peripheral:(CBPeripheral *)aPeripheral
        didUpdateValueForDescriptor:(CBDescriptor *)descriptor
        error:(NSError *)error
{
    Q_UNUSED(aPeripheral)

    Q_ASSERT_X(peripheral, "-peripheral:didUpdateValueForDescriptor:error:",
               "invalid peripheral (nil)");

    QT_BT_MAC_AUTORELEASEPOOL;

    using namespace OSXBluetooth;

    CBService *const service = descriptor.characteristic.service;
    const QBluetoothUuid qtUuid(qt_uuid(service.UUID));
    const bool isDetailsDiscovery = servicesToDiscoverDetails.contains(qtUuid);

    if (error) {
        // NSLog to log the actual error ...
        NSLog(@"-peripheral:didUpdateValueForDescriptor:error:, failed with error %@",
              error);
        if (!isDetailsDiscovery) {
            // TODO: probably will be required in a future.
            return;
        }
    }

    if (isDetailsDiscovery) {
        // Test if we have any other characteristic to read yet.
        CBDescriptor *const next = [self nextDescriptorForCharacteristic:descriptor.characteristic
                                         startingFrom:descriptor];
        if (next) {
            [peripheral readValueForDescriptor:next];
        } else {
            // We either have to read a value for a next descriptor
            // on a given characteristic, or continue with the
            // next characteristic in a given service (if any).
            CBCharacteristic *const ch = descriptor.characteristic;
            CBCharacteristic *nextCh = [self nextCharacteristicForService:ch.service
                                             startingFrom:ch];
            while (nextCh) {
                if (nextCh.descriptors && nextCh.descriptors.count)
                    return [peripheral readValueForDescriptor:[nextCh.descriptors objectAtIndex:0]];

                nextCh = [self nextCharacteristicForService:ch.service
                               startingFrom:nextCh];
            }

            [self serviceDetailsDiscoveryFinished:service];
        }
    } else {
        // TODO: this can be something else in a future (if needed at all).
    }
}

- (void)peripheral:(CBPeripheral *)aPeripheral
        didWriteValueForCharacteristic:(CBCharacteristic *)characteristic
        error:(NSError *)error
{
    // From docs:
    //
    // "This method is invoked only when your app calls the writeValue:forCharacteristic:type:
    //  method with the CBCharacteristicWriteWithResponse constant specified as the write type.
    //  If successful, the error parameter is nil. If unsuccessful,
    //  the error parameter returns the cause of the failure."

    using namespace OSXBluetooth;

    Q_UNUSED(aPeripheral)
    Q_UNUSED(characteristic)
    Q_UNUSED(error)

    Q_ASSERT_X(delegate, "-peripheral:didWriteValueForCharacteristic:error",
               "invalid delegate (null)");

    if (error) {
        // Use NSLog to log the actual error:
        NSLog(@"-peripheral:didWriteValueForCharacteristic:error:, failed with error: %@",
              error);
        // TODO: no char handle at the moment, have to change to char index instead
        // and calculate the right handle in the LE controller.
        delegate->error(qt_uuid(characteristic.service.UUID), 0,
                        QLowEnergyService::CharacteristicWriteError);
    } else {
        ObjCStrongReference<CBCharacteristic> ch(characteristic, true);
        delegate->characteristicWriteNotification(ch);
    }
}

@end

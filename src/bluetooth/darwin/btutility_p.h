// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTUTILITY_P_H
#define BTUTILITY_P_H

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

#include "btraii_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qglobal.h>

#include <QtCore/private/qcore_mac_p.h>

#include <Foundation/Foundation.h>

#include <CoreBluetooth/CoreBluetooth.h>

#ifdef Q_OS_MACOS
#include <IOBluetooth/IOBluetooth.h>
#endif // Q_OS_MACOS

QT_BEGIN_NAMESPACE

class QLowEnergyCharacteristicData;
class QBluetoothAddress;
class QBluetoothUuid;

namespace DarwinBluetooth {

template<class T>
class ObjCScopedPointer
{
public:
    ObjCScopedPointer() = default;
    ObjCScopedPointer(T *ptr, RetainPolicy policy)
        : m_ptr(ptr, policy)
    {
    }
    void swap(ObjCScopedPointer &other)
    {
        m_ptr.swap(other.m_ptr);
    }
    void reset()
    {
        m_ptr.reset();
    }
    void reset(T *ptr, RetainPolicy policy)
    {
        m_ptr.reset(ptr, policy);
    }
    operator T*() const
    {
        return m_ptr.getAs<T>();
    }
    T *get() const
    {
        // operator T * above does not work when accessing
        // properties using '.' syntax.
        return m_ptr.getAs<T>();
    }
private:
    // Copy and move disabled by m_ptr:
    ScopedPointer m_ptr;
};

#define QT_BT_MAC_AUTORELEASEPOOL const QMacAutoReleasePool pool;

template<class T>
class ObjCStrongReference final : public StrongReference {
public:
    using StrongReference::StrongReference;

    operator T *() const
    {
        return this->getAs<T>();
    }

    T *data() const
    {
        return this->getAs<T>();
    }
};

QString qt_address(NSString *address);

#ifndef QT_IOS_BLUETOOTH

QBluetoothAddress qt_address(const BluetoothDeviceAddress *address);
BluetoothDeviceAddress iobluetooth_address(const QBluetoothAddress &address);

ObjCStrongReference<IOBluetoothSDPUUID> iobluetooth_uuid(const QBluetoothUuid &uuid);
QBluetoothUuid qt_uuid(IOBluetoothSDPUUID *uuid);
QString qt_error_string(IOReturn errorCode);
void qt_test_iobluetooth_runloop();

#endif // !QT_IOS_BLUETOOTH

QBluetoothUuid qt_uuid(CBUUID *uuid);
ObjCStrongReference<CBUUID> cb_uuid(const QBluetoothUuid &qtUuid);
bool equal_uuids(const QBluetoothUuid &qtUuid, CBUUID *cbUuid);
bool equal_uuids(CBUUID *cbUuid, const QBluetoothUuid &qtUuid);
QByteArray qt_bytearray(NSData *data);
QByteArray qt_bytearray(NSObject *data);

ObjCStrongReference<NSData> data_from_bytearray(const QByteArray &qtData);
ObjCStrongReference<NSMutableData> mutable_data_from_bytearray(const QByteArray &qtData);

dispatch_queue_t qt_LE_queue();

extern const int defaultLEScanTimeoutMS;
extern const int maxValueLength;
extern const int defaultMtu;

// Add more keys if needed, for now this one is enough:
extern NSString *const bluetoothUsageKey;

bool qt_appNeedsBluetoothUsageDescription();
bool qt_appPlistContainsDescription(NSString *key);

} // namespace DarwinBluetooth

Q_DECLARE_LOGGING_CATEGORY(QT_BT_DARWIN)

QT_END_NAMESPACE

#if QT_MACOS_PLATFORM_SDK_EQUAL_OR_ABOVE(101300) && QT_MACOS_DEPLOYMENT_TARGET_BELOW(101300)

 // In the macOS 10.13 SDK, the identifier property was moved from the CBPeripheral
 // and CBCentral classes to a new CBPeer base class. Because CBPeer is only available
 // on macOS 10.13 and above, the same is true for -[CBPeer identifier]. However,
 // since we know that the derived classes have always had this property,
 // we'll explicitly mark its availability here. This will not adversely affect
 // using the identifier through the CBPeer base class, which will still require macOS 10.13.

@interface CBPeripheral (UnguardedWorkaround)
@property (readonly, nonatomic) NSUUID *identifier NS_AVAILABLE(10_7, 5_0);
@end

@interface CBCentral (UnguardedWorkaround)
@property (readonly, nonatomic) NSUUID *identifier NS_AVAILABLE(10_7, 5_0);
@end

#endif // QT_MACOS_PLATFORM_SDK_EQUAL_OR_ABOVE

#endif // BTUTILITY_P_H

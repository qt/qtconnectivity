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

#ifndef OSXBTLEDEVICEINQUIRY_P_H
#define OSXBTLEDEVICEINQUIRY_P_H

#include "qbluetoothdevicediscoveryagent.h"

#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>

// The Foundation header must be included before
// corebluetoothwrapper_p.h - a workaround for a broken
// 10.9 SDK.
#include <Foundation/Foundation.h>

@class QT_MANGLE_NAMESPACE(OSXBTLEDeviceInquiry);

@class CBCentralManager;
@class CBPeripheral;

QT_BEGIN_NAMESPACE

class QBluetoothDeviceInfo;
class QBluetoothUuid;

namespace OSXBluetooth {

class LEDeviceInquiryDelegate
{
public:
    typedef QT_MANGLE_NAMESPACE(OSXBTLEDeviceInquiry) LEDeviceInquiryObjC;

    virtual ~LEDeviceInquiryDelegate();

    // At the moment the only error we're reporting is PoweredOffError!
    virtual void LEdeviceInquiryError(QBluetoothDeviceDiscoveryAgent::Error error) = 0;

    virtual void LEnotSupported() = 0;
    virtual void LEdeviceFound(CBPeripheral *peripheral, const QBluetoothUuid &uuid,
                               NSDictionary *advertisementData, NSNumber *RSSI) = 0;
    virtual void LEdeviceInquiryFinished() = 0;
};

}

QT_END_NAMESPACE

// Bluetooth Low Energy scan for iOS and OS X.

@interface QT_MANGLE_NAMESPACE(OSXBTLEDeviceInquiry) : NSObject
{// Protocols are adopted in the mm file.
    QT_PREPEND_NAMESPACE(OSXBluetooth::LEDeviceInquiryDelegate) *delegate;

    // TODO: scoped pointers/shared pointers?
    NSMutableDictionary *peripherals; // Found devices.
    CBCentralManager *manager;

    // pending - waiting for a status update first.
    bool pendingStart;
    bool cancelled;
    // scan actually started.
    bool isActive;
}

- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(OSXBluetooth::LEDeviceInquiryDelegate) *)aDelegate;
- (void)dealloc;

// Actual scan can be delayed - we have to wait for a status update first.
- (bool)start;
// Stop can be delayed - if we're waiting for a status update.
- (void)stop;

- (bool)isActive;

@end

#endif

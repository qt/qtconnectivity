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

#ifndef OSXBTDEVICEINQUIRY_P_H
#define OSXBTDEVICEINQUIRY_P_H

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
#include "btdelegates_p.h"

#include <QtCore/qglobal.h>

#include <Foundation/Foundation.h>
#include <IOKit/IOReturn.h>

@interface QT_MANGLE_NAMESPACE(OSXBTDeviceInquiry) : NSObject<IOBluetoothDeviceInquiryDelegate>

- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(DarwinBluetooth::DeviceInquiryDelegate) *)delegate;
- (void)dealloc;

- (bool)isActive;
- (IOReturn)start;
- (IOReturn)stop;

//Obj-C delegate:
- (void)deviceInquiryComplete:(IOBluetoothDeviceInquiry *)sender
        error:(IOReturn)error aborted:(BOOL)aborted;

- (void)deviceInquiryDeviceFound:(IOBluetoothDeviceInquiry *)sender
        device:(IOBluetoothDevice *)device;

- (void)deviceInquiryStarted:(IOBluetoothDeviceInquiry *)sender;

@end

#endif

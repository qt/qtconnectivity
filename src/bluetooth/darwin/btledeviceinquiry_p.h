/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

#ifndef BTLEDEVICEINQUIRY_P_H
#define BTLEDEVICEINQUIRY_P_H

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

#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothdeviceinfo.h"
#include "btgcdtimer_p.h"
#include "btutility_p.h"

#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>

#include <Foundation/Foundation.h>

#include <CoreBluetooth/CoreBluetooth.h>

QT_BEGIN_NAMESPACE

class QBluetoothUuid;

namespace DarwinBluetooth
{

class LECBManagerNotifier;

} // namespace DarwinBluetooth

QT_END_NAMESPACE

using QT_PREPEND_NAMESPACE(DarwinBluetooth)::LECBManagerNotifier;
using QT_PREPEND_NAMESPACE(DarwinBluetooth)::ObjCScopedPointer;

enum LEInquiryState
{
    InquiryStarting,
    InquiryActive,
    InquiryFinished,
    InquiryCancelled,
    ErrorPoweredOff,
    ErrorLENotSupported
};

@interface QT_MANGLE_NAMESPACE(DarwinBTLEDeviceInquiry) : NSObject<CBCentralManagerDelegate, QT_MANGLE_NAMESPACE(GCDTimerDelegate)>
- (id)initWithNotifier:(LECBManagerNotifier *)aNotifier;
- (void)dealloc;

// IMPORTANT: both 'startWithTimeout' and 'stop' MUST be executed on the "Qt's
// LE queue".
- (void)startWithTimeout:(int)timeout;
- (void)stop;

@end

#endif

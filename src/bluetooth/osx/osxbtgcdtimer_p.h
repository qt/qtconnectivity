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

#ifndef OSXBTGCDTIMER_P_H
#define OSXBTGCDTIMER_P_H

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

#include "osxbtutility_p.h"

#include <QtCore/qelapsedtimer.h>
#include <QtCore/qglobal.h>

#include <Foundation/Foundation.h>

QT_BEGIN_NAMESPACE

namespace OSXBluetooth {

enum class OperationTimeout
{
    none,
    serviceDiscovery,
    includedServicesDiscovery,
    characteristicsDiscovery,
    characteristicRead,
    descriptorsDiscovery,
    descriptorRead,
    characteristicWrite
};

} // namespace OSXBluetooth

QT_END_NAMESPACE

@protocol QT_MANGLE_NAMESPACE(GCDTimerDelegate)
@required
- (void)timeout:(id)sender;
@end

@interface QT_MANGLE_NAMESPACE(OSXBTGCDTimer) : NSObject
- (instancetype)initWithDelegate:(id<QT_MANGLE_NAMESPACE(GCDTimerDelegate)>)delegate;
- (void)watchAfter:(id)object withTimeoutType:(QT_PREPEND_NAMESPACE(OSXBluetooth)::OperationTimeout)type;
- (void)startWithTimeout:(qint64)ms step:(qint64)stepMS;
- (void)handleTimeout;
- (void)cancelTimer;
- (id)objectUnderWatch;
- (QT_PREPEND_NAMESPACE(OSXBluetooth)::OperationTimeout)timeoutType;
@end

QT_BEGIN_NAMESPACE

namespace OSXBluetooth {

using GCDTimerObjC = QT_MANGLE_NAMESPACE(OSXBTGCDTimer);
using GCDTimer = ObjCStrongReference<GCDTimerObjC>;

} // namespace OSXBluetooth

QT_END_NAMESPACE

#endif // OSXBTGCDTIMER_P_H


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

#ifndef OSXBTCENTRALMANAGER_P_H
#define OSXBTCENTRALMANAGER_P_H

#include "qlowenergycontroller.h"
#include "qbluetoothuuid.h"
#include "osxbtutility_p.h"

#include <QtCore/qglobal.h>

// Foundation.h must be included before corebluetoothwrapper_p.h -
// a workaround for a broken 10.9 SDK.
#include <Foundation/Foundation.h>

#include "corebluetoothwrapper_p.h"

@class QT_MANGLE_NAMESPACE(OSXBTCentralManager);

QT_BEGIN_NAMESPACE

namespace OSXBluetooth {

class CentralManagerDelegate
{
public:
    typedef QT_MANGLE_NAMESPACE(OSXBTCentralManager) ObjCCentralManager;
    typedef ObjCStrongReference<NSArray> LEServices;
    typedef LEServices LECharacteristics;

    virtual ~CentralManagerDelegate();

    virtual void LEnotSupported() = 0;
    virtual void connectSuccess() = 0;
    virtual void serviceDiscoveryFinished(LEServices services) = 0;
    virtual void disconnected() = 0;

    // General errors.
    virtual void error(QLowEnergyController::Error error) = 0;
    // Service related errors.
    virtual void error(const QBluetoothUuid &serviceUuid,
                       QLowEnergyController::Error error) = 0;
};

enum CentralManagerState
{
    // QLowEnergyController already has some of these states,
    // but it's not enough and we need more special states here.
    CentralManagerIdle,
    // Required by CBCentralManager to avoid problems with dangled pointers.
    CentralManagerUpdating,
    CentralManagerConnecting,
    CentralManagerDiscovering,
    CentralManagerDisconnecting
};

}

QT_END_NAMESPACE

@interface QT_MANGLE_NAMESPACE(OSXBTCentralManager) : NSObject<CBCentralManagerDelegate, CBPeripheralDelegate>
{
    CBCentralManager *manager;
    QT_PREPEND_NAMESPACE(OSXBluetooth)::CentralManagerState managerState;
    bool disconnectPending;

    QT_PREPEND_NAMESPACE(QBluetoothUuid) deviceUuid;
    CBPeripheral *peripheral;

    QT_PREPEND_NAMESPACE(OSXBluetooth)::CentralManagerDelegate *delegate;

    // Quite a verbose service discovery machinery
    // (a "graph traversal").
    QT_PREPEND_NAMESPACE(OSXBluetooth)::ObjCStrongReference<NSMutableArray> servicesToVisit;
    // The service we're discovering now (included services discovery):
    NSUInteger currentService;
    // Included services, we'll iterate through at the end of 'servicesToVisit':
    QT_PREPEND_NAMESPACE(OSXBluetooth)::ObjCStrongReference<NSMutableArray> servicesToVisitNext;
    // We'd like to avoid loops in a services' topology:
    QT_PREPEND_NAMESPACE(OSXBluetooth)::ObjCStrongReference<NSMutableSet> visitedServices;
}

- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(OSXBluetooth)::CentralManagerDelegate *)aDelegate;
- (void)dealloc;

- (QT_PREPEND_NAMESPACE(QLowEnergyController)::Error)
    connectToDevice:(const QT_PREPEND_NAMESPACE(QBluetoothUuid) &)aDeviceUuid;

- (void)disconnectFromDevice;

- (void)discoverServices;

@end

#endif

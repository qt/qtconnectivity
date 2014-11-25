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

#ifndef OSXBTL2CAPCHANNEL_P_H
#define OSXBTL2CAPCHANNEL_P_H

#include <QtCore/qglobal.h>

// Must be imported (Obj-C header, no inclusion guards).
#import <IOBluetooth/objc/IOBluetoothL2CAPChannel.h>

#include <Foundation/Foundation.h>

#include <cstddef>

QT_BEGIN_NAMESPACE

class QBluetoothAddress;

namespace OSXBluetooth {

class ChannelDelegate;

}

QT_END_NAMESPACE

@class IOBluetoothDevice;

@interface QT_MANGLE_NAMESPACE(OSXBTL2CAPChannel) : NSObject<IOBluetoothL2CAPChannelDelegate>
{
    QT_PREPEND_NAMESPACE(OSXBluetooth::ChannelDelegate) *delegate;
    IOBluetoothDevice *device;
    IOBluetoothL2CAPChannel *channel;
    bool connected;
}

- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(OSXBluetooth::ChannelDelegate) *)aDelegate;
- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(OSXBluetooth::ChannelDelegate) *)aDelegate
      channel:(IOBluetoothL2CAPChannel *)aChannel;

- (void)dealloc;

// A single async connection (connect can be called only once).
- (IOReturn)connectAsyncToDevice:(const QBluetoothAddress &)address
            withPSM:(BluetoothL2CAPChannelID)psm;

// IOBluetoothL2CAPChannelDelegate:
- (void)l2capChannelData:(IOBluetoothL2CAPChannel*)l2capChannel
        data:(void *)dataPointer length:(size_t)dataLength;
- (void)l2capChannelOpenComplete:(IOBluetoothL2CAPChannel*)
        l2capChannel status:(IOReturn)error;
- (void)l2capChannelClosed:(IOBluetoothL2CAPChannel*)l2capChannel;
- (void)l2capChannelReconfigured:(IOBluetoothL2CAPChannel*)l2capChannel;
- (void)l2capChannelWriteComplete:(IOBluetoothL2CAPChannel*)l2capChannel
        refcon:(void*)refcon status:(IOReturn)error;
- (void)l2capChannelQueueSpaceAvailable:(IOBluetoothL2CAPChannel*)l2capChannel;

//
- (BluetoothL2CAPPSM)getPSM;
- (BluetoothDeviceAddress)peerAddress;
- (NSString *)peerName;
- (bool)isConnected;

// Writes the given data synchronously over the target L2CAP channel to the remote
// device.
// The length of the data may not exceed the L2CAP channel's ougoing MTU.
// This method will block until the data has been successfully sent to the
// hardware for transmission (or an error occurs).
- (IOReturn) writeSync:(void*)data length:(UInt16)length;

// The length of the data may not exceed the L2CAP channel's ougoing MTU.
// When the data has been successfully passed to the hardware to be transmitted,
// the delegate method -l2capChannelWriteComplete:refcon:status: will be called.
// Returns kIOReturnSuccess if the data was buffered successfully.
- (IOReturn) writeAsync:(void*)data length:(UInt16)length;

@end

#endif

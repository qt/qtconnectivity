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

#include "osxbtchanneldelegate_p.h"
#include "osxbtl2capchannel_p.h"
#include "qbluetoothaddress.h"
#include "osxbtutility_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qdebug.h>

#ifdef QT_NAMESPACE
using namespace QT_NAMESPACE;
#endif

@implementation QT_MANGLE_NAMESPACE(OSXBTL2CAPChannel)

- (id)initWithDelegate:(OSXBluetooth::ChannelDelegate *)aDelegate
{
    Q_ASSERT_X(aDelegate, "-initWithDelegate:", "invalid delegate (null)");

    if (self = [super init]) {
        delegate = aDelegate;
        device = nil;
        channel = nil;
        connected = false;
    }

    return self;
}

- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(OSXBluetooth::ChannelDelegate) *)aDelegate
      channel:(IOBluetoothL2CAPChannel *)aChannel
{
    // This type of channel does not require connect, it's created with
    // already open channel.
    Q_ASSERT_X(aDelegate, "-initWithDelegate:channel:", "invalid delegate (null)");
    Q_ASSERT_X(channel, "-initWithDelegate:channel:", "invalid channel (nil)");

    if (self = [super init]) {
        delegate = aDelegate;
        channel = [aChannel retain];
        [channel setDelegate:self];
        device = [channel.device retain];
        connected = true;
    }

    return self;
}

- (void)dealloc
{
    // TODO: test if this implementation works at all!
    if (channel) {
        [channel setDelegate:nil];
        // From Apple's docs:
        // "This method may only be called by the client that opened the channel
        // in the first place. In the future asynchronous and synchronous versions
        // will be provided that let the client know when the close process has been finished."
        [channel closeChannel];
        [channel release];
    }

    [device release];

    [super dealloc];
}

- (IOReturn)connectAsyncToDevice:(const QBluetoothAddress &)address
            withPSM:(BluetoothL2CAPChannelID)psm
{
    if (address.isNull()) {
        qCCritical(QT_BT_OSX) << "-connectAsyncToDevice:withPSM:, "
                                 "invalid peer address";
        return kIOReturnNoDevice;
    }

    // Can never be called twice.
    if (connected || device || channel) {
        qCCritical(QT_BT_OSX) << "-connectAsyncToDevice:withPSM:, "
                                 "connection is already active";
        return kIOReturnStillOpen;
    }

    QT_BT_MAC_AUTORELEASEPOOL;

    const BluetoothDeviceAddress iobtAddress = OSXBluetooth::iobluetooth_address(address);
    device = [IOBluetoothDevice deviceWithAddress:&iobtAddress];
    if (!device) {
        qCCritical(QT_BT_OSX) << "-connectAsyncToDevice:withPSM:, "
                                 "failed to create a device";
        return kIOReturnNoDevice;
    }

    const IOReturn status = [device openL2CAPChannelAsync:&channel withPSM:psm delegate:self];
    if (status != kIOReturnSuccess) {
        qCCritical(QT_BT_OSX) << "-connectAsyncToDevice:withPSM:, "
                                 "failed to open L2CAP channel";
        // device is still autoreleased.
        device = nil;
        return status;
    }

    [channel retain];// What if we're closed already?
    [device retain];

    return kIOReturnSuccess;
}

// IOBluetoothL2CAPChannelDelegate:

- (void)l2capChannelData:(IOBluetoothL2CAPChannel*)l2capChannel
        data:(void *)dataPointer length:(size_t)dataLength
{
    Q_UNUSED(l2capChannel)

    Q_ASSERT_X(delegate, "-l2capChannelData:data:length",
               "invalid delegate (null)");

    if (dataPointer && dataLength)
        delegate->readChannelData(dataPointer, dataLength);
}

- (void)l2capChannelOpenComplete:(IOBluetoothL2CAPChannel*)
        l2capChannel status:(IOReturn)error
{
    Q_UNUSED(l2capChannel)

    Q_ASSERT_X(delegate, "-l2capChannelOpenComplete:status:",
               "invalid delegate (null)");

    if (error != kIOReturnSuccess) {
        delegate->setChannelError(error);
    } else {
        connected = true;
        delegate->channelOpenComplete();
    }
}

- (void)l2capChannelClosed:(IOBluetoothL2CAPChannel*)l2capChannel
{
    Q_UNUSED(l2capChannel)

    Q_ASSERT_X(delegate, "-l2capChannelClosed:", "invalid delegate (null)");
    delegate->channelClosed();
    connected = false;
}

- (void)l2capChannelReconfigured:(IOBluetoothL2CAPChannel*)l2capChannel
{
    Q_UNUSED(l2capChannel)
}

- (void)l2capChannelWriteComplete:(IOBluetoothL2CAPChannel*)l2capChannel
        refcon:(void*)refcon status:(IOReturn)error
{
    Q_UNUSED(l2capChannel)
    Q_UNUSED(refcon)

    Q_ASSERT_X(delegate, "-l2capChannelWriteComplete:refcon:status",
               "invalid delegate (null)");

    if (error != kIOReturnSuccess)
        delegate->setChannelError(error);
    else
        delegate->writeComplete();
}

- (void)l2capChannelQueueSpaceAvailable:(IOBluetoothL2CAPChannel*)l2capChannel
{
    Q_UNUSED(l2capChannel)
}

// Aux. methods.
- (BluetoothL2CAPPSM)getPSM
{
    if (channel)
        return channel.PSM;

    return 0;
}

- (BluetoothDeviceAddress)peerAddress
{
    const BluetoothDeviceAddress *const addr = device ? [device getAddress]
                                                      : Q_NULLPTR;
    if (addr)
        return *addr;

    return BluetoothDeviceAddress();
}

- (NSString *)peerName
{
    if (device)
        return device.name;

    return nil;
}

- (bool)isConnected
{
    return connected;
}

- (IOReturn) writeSync:(void*)data length:(UInt16)length
{
    Q_ASSERT_X(data, "-writeSync:length:", "invalid data (null)");
    Q_ASSERT_X(length, "-writeSync:length:", "invalid data size");
    Q_ASSERT_X(connected && channel, "-writeSync:",
               "invalid L2CAP channel");

    return [channel writeSync:data length:length];
}

- (IOReturn) writeAsync:(void*)data length:(UInt16)length
{
    Q_ASSERT_X(data, "-writeAsync:length:", "invalid data (null)");
    Q_ASSERT_X(length, "-writeAync:length:", "invalid data size");
    Q_ASSERT_X(connected && channel, "-writeAsync:length:",
               "invalid L2CAP channel");

    return [channel writeAsync:data length:length refcon:Q_NULLPTR];
}


@end

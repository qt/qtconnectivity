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

#include "btdeviceinquiry_p.h"
#include "btutility_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qtimer.h>
#include <QtCore/qdebug.h>

#include <memory>

QT_USE_NAMESPACE

const uint8_t IOBlueoothInquiryLengthS = 15;

@implementation QT_MANGLE_NAMESPACE(DarwinBTClassicDeviceInquiry)
{
    IOBluetoothDeviceInquiry *m_inquiry;
    bool m_active;
    DarwinBluetooth::DeviceInquiryDelegate *m_delegate;//C++ "delegate"

    std::unique_ptr<QTimer> watchDog;
}

- (id)initWithDelegate:(DarwinBluetooth::DeviceInquiryDelegate *)delegate
{
    if (self = [super init]) {
        Q_ASSERT_X(delegate, Q_FUNC_INFO, "invalid device inquiry delegate (null)");

        m_inquiry = [[IOBluetoothDeviceInquiry inquiryWithDelegate:self] retain];

        if (m_inquiry) {
            // Inquiry length is 15 seconds. Starting from macOS 10.15.7
            // (the lowest version I was able to test on, though initially
            // the problem was found on macOS 11, arm64 machine and then
            // confirmed on macOS 12 Beta 4), it seems to be ignored,
            // thus scan never stops. See -start for how we try to prevent
            // this.
            [m_inquiry setInquiryLength:IOBlueoothInquiryLengthS];
            [m_inquiry setUpdateNewDeviceNames:NO];//Useless, disable!
            m_delegate = delegate;
        } else {
            qCCritical(QT_BT_DARWIN) << "failed to create a device inquiry";
        }

        m_active = false;
    }

    return self;
}

- (void)dealloc
{
    // Noop if m_inquiry is nil.
    [m_inquiry setDelegate:nil];
    if (m_active)
        [m_inquiry stop];
    [m_inquiry release];

    [super dealloc];
}

- (bool)isActive
{
    return m_active;
}

- (IOReturn)start
{
    if (!m_inquiry)
        return kIOReturnNoPower;

    if (m_active)
        return kIOReturnBusy;

    m_active = true;
    [m_inquiry clearFoundDevices];
    const IOReturn result = [m_inquiry start];
    if (result != kIOReturnSuccess) {
        // QtBluetooth will probably convert an error into UnknownError,
        // losing the actual information.
        qCWarning(QT_BT_DARWIN) << "failed with IOKit error code:" << result;
        m_active = false;
    } else {
        // Docs say it's 10 s. by default, we set it to 15 s. (see -initWithDelegate:),
        // and it may fail to finish.
        watchDog.reset(new QTimer);
        watchDog->connect(watchDog.get(), &QTimer::timeout, watchDog.get(), [self]{
            qCWarning(QT_BT_DARWIN, "Manually interrupting IOBluetoothDeviceInquiry");
            [self stop];
        });

        watchDog->setSingleShot(true);
        // +2 to give IOBluetooth a chance to stop it first:
        watchDog->setInterval((IOBlueoothInquiryLengthS + 2) * 1000);
        watchDog->start();
    }

    return result;
}

- (IOReturn)stop
{
    if (!m_active)
        return kIOReturnSuccess;

    Q_ASSERT_X(m_inquiry, Q_FUNC_INFO, "active but nil inquiry");

    return [m_inquiry stop];
}

- (void)deviceInquiryComplete:(IOBluetoothDeviceInquiry *)sender
        error:(IOReturn)error aborted:(BOOL)aborted
{
    if (!m_active)
        return;

    if (sender != m_inquiry) // Can never happen in the current version.
        return;

    Q_ASSERT_X(m_delegate, Q_FUNC_INFO, "invalid device inquiry delegate (null)");

    if (error != kIOReturnSuccess && !aborted) {
        // QtBluetooth has not too many error codes, 'UnknownError' is not really
        // useful, log the actual error code here:
        qCWarning(QT_BT_DARWIN) << "IOKit error code: " << error;
        // Let watchDog to stop it, calling -stop at timeout, otherwise,
        // it looks like inquiry continues even after this error and
        // keeps reporting new devices found.
    } else {
        // Either a normal completion or from a timer slot.
        watchDog.reset();
        m_active = false;
        m_delegate->inquiryFinished();
    }
}

- (void)deviceInquiryDeviceFound:(IOBluetoothDeviceInquiry *)sender
        device:(IOBluetoothDevice *)device
{
    if (sender != m_inquiry) // Can never happen in the current version.
        return;

    if (!m_active) {
        // We are not expecting new device(s) to be found after we reported 'finished'.
        qCWarning(QT_BT_DARWIN, "IOBluetooth device found after inquiry complete/interrupted");
        return;
    }

    Q_ASSERT_X(m_delegate, Q_FUNC_INFO, "invalid device inquiry delegate (null)");
    m_delegate->classicDeviceFound(device);
}

- (void)deviceInquiryStarted:(IOBluetoothDeviceInquiry *)sender
{
    Q_UNUSED(sender);
}

@end

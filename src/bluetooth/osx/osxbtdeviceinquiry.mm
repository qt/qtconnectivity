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

#include "osxbtdeviceinquiry_p.h"
#include "osxbtutility_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qglobal.h>
#include <QtCore/qdebug.h>


QT_BEGIN_NAMESPACE

namespace OSXBluetooth {

DeviceInquiryDelegate::~DeviceInquiryDelegate()
{
}

}


QT_END_NAMESPACE


#ifdef QT_NAMESPACE
// We do not want to litter a code with QT_PREPEND_NAMESPACE, right?
using namespace QT_NAMESPACE;
#endif


@implementation QT_MANGLE_NAMESPACE(OSXBTDeviceInquiry)

- (id)initWithDelegate:(OSXBluetooth::DeviceInquiryDelegate *)delegate
{
    if (self = [super init]) {
        Q_ASSERT_X(delegate, Q_FUNC_INFO, "invalid device inquiry delegate (null)");

        m_inquiry = [[IOBluetoothDeviceInquiry inquiryWithDelegate:self] retain];

        if (m_inquiry) {
            [m_inquiry setInquiryLength:15];
            [m_inquiry setUpdateNewDeviceNames:NO];//Useless, disable!
            m_delegate = delegate;
        } else {
            qCCritical(QT_BT_OSX) << Q_FUNC_INFO << "failed to create "
                                     "a device inquiry";
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
        // loosing the actual information.
        qCWarning(QT_BT_OSX) << Q_FUNC_INFO <<"failed with "
                                "IOKit error code: " << result;
        m_active = false;
    }

    return result;
}

- (IOReturn)stop
{
    if (m_active) {
        Q_ASSERT_X(m_inquiry, Q_FUNC_INFO, "active but nil inquiry");

        m_active = false;
        const IOReturn res = [m_inquiry stop];
        if (res != kIOReturnSuccess)
            m_active = true;

        return res;
    }

    return kIOReturnSuccess;
}

- (void)deviceInquiryComplete:(IOBluetoothDeviceInquiry *)sender
        error:(IOReturn)error aborted:(BOOL)aborted
{
    Q_UNUSED(aborted)

    if (sender != m_inquiry) // Can never happen in the current version.
        return;

    m_active = false;

    Q_ASSERT_X(m_delegate, Q_FUNC_INFO, "invalid device inquiry delegate (null)");

    if (error != kIOReturnSuccess) {
        // QtBluetooth has not too many error codes, 'UnknownError' is not really
        // useful, report the actual error code here:
        qCWarning(QT_BT_OSX) << Q_FUNC_INFO << "IOKit error code: " << error;
        m_delegate->error(sender, error);
    } else {
        m_delegate->inquiryFinished(sender);
    }
}

- (void)deviceInquiryDeviceFound:(IOBluetoothDeviceInquiry *)sender
        device:(IOBluetoothDevice *)device
{
    if (sender != m_inquiry) // Can never happen in the current version.
        return;

    Q_ASSERT_X(m_delegate, Q_FUNC_INFO, "invalid device inquiry delegate (null)");
    m_delegate->deviceFound(sender, device);
}

- (void)deviceInquiryStarted:(IOBluetoothDeviceInquiry *)sender
{
    Q_UNUSED(sender)
}

@end

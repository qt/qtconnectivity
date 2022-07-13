/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

#include "qbluetoothdevicewatcher_winrt_p.h"

#include <winrt/Windows.Foundation.Collections.h>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Devices::Enumeration;

QT_BEGIN_NAMESPACE

QBluetoothDeviceWatcherWinRT::QBluetoothDeviceWatcherWinRT(int id, winrt::hstring selector)
    : m_id(id),
      m_watcher(DeviceInformation::CreateWatcher(selector))
{
    qRegisterMetaType<winrt::hstring>("winrt::hstring");
}

QBluetoothDeviceWatcherWinRT::QBluetoothDeviceWatcherWinRT(int id, winrt::hstring selector,
                                winrt::Windows::Devices::Enumeration::DeviceInformationKind kind)
    : m_id(id)
{
    qRegisterMetaType<winrt::hstring>("winrt::hstring");
    const winrt::param::iterable<winrt::hstring> extra {};
    m_watcher = DeviceInformation::CreateWatcher(selector, extra, kind);
}

QBluetoothDeviceWatcherWinRT::~QBluetoothDeviceWatcherWinRT()
{
    if (m_watcher && m_initialized) {
        stop();
        unsubscribeFromEvents();
        m_initialized = false;
    }
}

bool QBluetoothDeviceWatcherWinRT::init()
{
    if (!m_watcher) {
        qWarning("Windows failed to create an instance of DeviceWatcher. "
                 "Detection of Bluetooth devices might not work correctly.");
        return false;
    }
    if (!m_initialized) {
        subscribeToEvents();
        m_initialized = true;
    }
    return true;
}

void QBluetoothDeviceWatcherWinRT::start()
{
    if (m_watcher)
        m_watcher.Start();
}

void QBluetoothDeviceWatcherWinRT::stop()
{
    if (m_watcher && canStop())
        m_watcher.Stop();
}

void QBluetoothDeviceWatcherWinRT::subscribeToEvents()
{
    Q_ASSERT(m_watcher.Status() == DeviceWatcherStatus::Created);
    // The callbacks are triggered from separate threads. So we capture
    // thisPtr to make sure that the object is valid.
    auto thisPtr = shared_from_this();
    m_addedToken = m_watcher.Added([thisPtr](DeviceWatcher, const DeviceInformation &info) {
        emit thisPtr->deviceAdded(info.Id(), thisPtr->m_id);
    });
    m_removedToken =
            m_watcher.Removed([thisPtr](DeviceWatcher, const DeviceInformationUpdate &upd) {
                emit thisPtr->deviceRemoved(upd.Id(), thisPtr->m_id);
            });
    m_updatedToken =
            m_watcher.Updated([thisPtr](DeviceWatcher, const DeviceInformationUpdate &upd) {
                emit thisPtr->deviceUpdated(upd.Id(), thisPtr->m_id);
            });
    // because of ambiguous declaration
    using WinRtInspectable = winrt::Windows::Foundation::IInspectable;
    m_enumerationToken =
            m_watcher.EnumerationCompleted([thisPtr](DeviceWatcher, const WinRtInspectable &) {
                emit thisPtr->enumerationCompleted(thisPtr->m_id);
            });
    m_stoppedToken = m_watcher.Stopped([thisPtr](DeviceWatcher, const WinRtInspectable &) {
        emit thisPtr->watcherStopped(thisPtr->m_id);
    });
}

void QBluetoothDeviceWatcherWinRT::unsubscribeFromEvents()
{
    m_watcher.Added(m_addedToken);
    m_watcher.Removed(m_removedToken);
    m_watcher.Updated(m_updatedToken);
    m_watcher.EnumerationCompleted(m_enumerationToken);
    m_watcher.Stopped(m_stoppedToken);
}

bool QBluetoothDeviceWatcherWinRT::canStop() const
{
    const auto status = m_watcher.Status();
    // Also 'Aborted', but calling Stop() there is a no-op
    return status == DeviceWatcherStatus::Started
            || status == DeviceWatcherStatus::EnumerationCompleted;
}

QT_END_NAMESPACE

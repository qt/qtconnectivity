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

#ifndef QBLUETOOTHDEVICEWATCHER_WINRT_P_H
#define QBLUETOOTHDEVICEWATCHER_WINRT_P_H

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

#include <QtCore/private/qglobal_p.h>
#include <QtCore/QObject>

#include <private/qbluetoothutils_winrt_p.h>

#include <winrt/base.h>
#include <QtCore/private/qfactorycacheregistration_p.h>
#include <winrt/Windows.Devices.Enumeration.h>

QT_BEGIN_NAMESPACE

class QBluetoothDeviceWatcherWinRT : public QObject,
        public std::enable_shared_from_this<QBluetoothDeviceWatcherWinRT>
{
    Q_OBJECT
public:
    QBluetoothDeviceWatcherWinRT(int id, winrt::hstring selector);
    QBluetoothDeviceWatcherWinRT(int id, winrt::hstring selector,
                                 winrt::Windows::Devices::Enumeration::DeviceInformationKind kind);
    ~QBluetoothDeviceWatcherWinRT();

    bool init();
    void start();
    void stop();

signals:
    // The signals will be emitted from a separate thread,
    // so we need to use Qt::QueuedConnection
    void deviceAdded(winrt::hstring deviceId, int id);
    void deviceRemoved(winrt::hstring deviceId, int id);
    void deviceUpdated(winrt::hstring deviceId, int id);
    void enumerationCompleted(int id);
    void watcherStopped(int id);

private:
    void subscribeToEvents();
    void unsubscribeFromEvents();
    bool canStop() const;

    const int m_id; // used to uniquely identify the wrapper
    winrt::Windows::Devices::Enumeration::DeviceWatcher m_watcher = nullptr;

    winrt::event_token m_addedToken;
    winrt::event_token m_removedToken;
    winrt::event_token m_updatedToken;
    winrt::event_token m_enumerationToken;
    winrt::event_token m_stoppedToken;
};

QT_END_NAMESPACE

#endif // QBLUETOOTHDEVICEWATCHER_WINRT_P_H

// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QOHOSBLUETOOTHDEVICEDISCOVERY_P_H
#define QOHOSBLUETOOTHDEVICEDISCOVERY_P_H

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

#include <QtBluetooth/private/qohosbluetoothaccess_p.h>
#include <QtBluetooth/private/qohosbluetoothcommon_p.h>

#include <QtCore/qobject.h>
#include <QtCore/qtconfigmacros.h>
#include <QtCore/qtimer.h>
#include <QtCore/qtypes.h>
#include <QtCore/private/qcore_ohos_p.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

QT_BEGIN_NAMESPACE

namespace QtOhosBluetooth {

struct DiscoveryResult
{
    std::string deviceId;
    std::string deviceName;
    quint32 classOfDevice = 0;
    qint16 rssi = 0;

    static DiscoveryResult makeFromOhosDiscoveryResultObject(
        QOhosJsState &jsState, QNapi::Object discoveryResultObject);
};

class QOhosBluetoothDeviceDiscoveryAgentProxy : public QObject
{
    Q_OBJECT
public:
    static std::shared_ptr<QOhosBluetoothDeviceDiscoveryAgentProxy> instance();

    bool isForeignDiscoveryOngoing();
    std::vector<DiscoveryResult> getPairedDevices();

    void startBluetoothDiscovery();
    void stopBluetoothDiscovery();

Q_SIGNALS:
    void discoveryStopped();
    void bluetoothDevicesFound(const std::vector<DiscoveryResult> &discoveredDevices);

    void missingPermission();
    void discoveryStartFailed();
    void discoveryStopFailed();
    void bluetoothPoweredOff();

protected:
    QOhosBluetoothDeviceDiscoveryAgentProxy();

private:
    bool ensureDiscoveredDevicesConsumerRegistered();
    std::optional<bool> tryCheckBluetoothDiscovering();
    void resetDiscoveryTracking();

    std::shared_ptr<QOhosBluetoothAccessProxy> m_accessProxy;
    std::shared_ptr<void> m_discoveredDevicesConsumerHandle;
    bool m_discoveryStartedByProxy = false;
    QTimer m_discoveryStoppedPollTimer;
    QTimer m_discoveryStoppedTimeoutTimer;
};

}

QT_END_NAMESPACE

#endif // QOHOSBLUETOOTHDEVICEDISCOVERY_P_H

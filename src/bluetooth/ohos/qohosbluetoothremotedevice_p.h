// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QOHOSBLUETOOTHREMOTEDEVICE_P_H
#define QOHOSBLUETOOTHREMOTEDEVICE_P_H

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

#include <QtBluetooth/private/qohosbluetoothcommon_p.h>
#include <QtBluetooth/private/qohosbluetoothenums_p.h>

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qtconfigmacros.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

QT_BEGIN_NAMESPACE

namespace QtOhosBluetooth {

struct RemoteDeviceServices
{
    std::string deviceId;
    std::optional<std::string> optDeviceName;
    std::optional<quint32> optClassOfDevice;
    bool lowEnergyOnly = false;
    std::optional<std::vector<std::string>> optProfileUuids;
};

class QOhosBluetoothRemoteDeviceProxy : public QObject
{
    Q_OBJECT
public:
    using BluetoothTransport =
        QtOhosBluetooth::enums::ohos::bluetooth::connection::BluetoothTransport;
    using BondState = QtOhosBluetooth::enums::ohos::bluetooth::connection::BondState;
    using UnbondCause = QtOhosBluetooth::enums::ohos::bluetooth::connection::UnbondCause;

    static std::shared_ptr<QOhosBluetoothRemoteDeviceProxy> instance();

    std::optional<QString> tryGetRemoteDeviceName(const QString &deviceId);
    std::optional<BondState> tryGetPairState(const QString &deviceId);

    bool pairDevice(const QString &deviceId);
    std::optional<QOhosBluetoothErrorCode> requestRemoteDeviceServices(const QString &deviceId);

Q_SIGNALS:
    void bondStateChanged(QString deviceId, BondState bondState, std::optional<UnbondCause> unbondCause);
    void pairDeviceFailed(QString deviceId);
    void remoteDeviceServicesRead(RemoteDeviceServices remoteDeviceServices);
    void missingPermission();

protected:
    QOhosBluetoothRemoteDeviceProxy();

private:
    bool ensureBondStateChangeConsumerRegistered();

    std::shared_ptr<void> m_bondStateChangeConsumerHandle;
};

std::optional<std::string> readRemoteDeviceName(QOhosJsState &jsState, const std::string &deviceId);
std::optional<quint32> readRemoteDeviceClass(QOhosJsState &jsState, const std::string &deviceId);
std::optional<QOhosBluetoothRemoteDeviceProxy::BluetoothTransport> readRemoteDeviceTransport(
    QOhosJsState &jsState, const std::string &deviceId);
bool isRemoteDeviceLowEnergyOnly(QOhosJsState &jsState, const std::string &deviceId);

}

QT_END_NAMESPACE

#endif // QOHOSBLUETOOTHREMOTEDEVICE_P_H

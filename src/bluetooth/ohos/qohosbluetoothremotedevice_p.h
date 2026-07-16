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

class QOhosBluetoothRemoteDeviceProxy : public QObject
{
    Q_OBJECT
public:
    using BondState = QtOhosBluetooth::enums::ohos::bluetooth::connection::BondState;
    using UnbondCause = QtOhosBluetooth::enums::ohos::bluetooth::connection::UnbondCause;

    static std::shared_ptr<QOhosBluetoothRemoteDeviceProxy> instance();

    std::optional<BondState> tryGetPairState(const QString &deviceId);

    bool pairDevice(const QString &deviceId);

Q_SIGNALS:
    void bondStateChanged(QString deviceId, BondState bondState, std::optional<UnbondCause> unbondCause);
    void pairDeviceFailed(QString deviceId);
    void missingPermission();

protected:
    QOhosBluetoothRemoteDeviceProxy();

private:
    bool ensureBondStateChangeConsumerRegistered();

    std::shared_ptr<void> m_bondStateChangeConsumerHandle;
};


}

QT_END_NAMESPACE

#endif // QOHOSBLUETOOTHREMOTEDEVICE_P_H

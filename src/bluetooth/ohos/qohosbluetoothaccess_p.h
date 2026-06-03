// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QOHOSBLUETOOTHACCESS_P_H
#define QOHOSBLUETOOTHACCESS_P_H

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
#include <QtCore/qtconfigmacros.h>
#include <QtCore/private/qohoscommon_p.h>

#include <optional>

QT_BEGIN_NAMESPACE

namespace QtOhosBluetooth {

class QOhosBluetoothAccessProxy : public QObject
{
    Q_OBJECT

public:
    using BluetoothState = QtOhosBluetooth::enums::ohos::bluetooth::access::BluetoothState;

    static std::shared_ptr<QOhosBluetoothAccessProxy> instance();

    std::optional<BluetoothState> tryGetBluetoothState() const;
    std::optional<QOhosBluetoothErrorCode> trySetBluetoothEnabled(bool enabled);

Q_SIGNALS:
    void stateChanged(BluetoothState state);
    void missingPermission();
    void setBluetoothEnabledFailed(QOhosBluetoothErrorCode errorCode);

protected:
    QOhosBluetoothAccessProxy();

private:
    QOhosSupplier<std::optional<BluetoothState>> m_ohosBluetoothStateSupplier;
};

}

QT_END_NAMESPACE

#endif // QOHOSBLUETOOTHACCESS_P_H

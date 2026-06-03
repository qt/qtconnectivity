// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QOHOSBLUETOOTHLOCALDEVICE_P_H
#define QOHOSBLUETOOTHLOCALDEVICE_P_H

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
#include <QtCore/qtconfigmacros.h>
#include <QtCore/private/qohoscommon_p.h>

#include <memory>
#include <optional>

QT_BEGIN_NAMESPACE

namespace QtOhosBluetooth {

class QOhosBluetoothLocalDeviceProxy : public QObject
{
    Q_OBJECT

public:
    using ScanMode = QtOhosBluetooth::enums::ohos::bluetooth::connection::ScanMode;

    static std::shared_ptr<QOhosBluetoothLocalDeviceProxy> instance();

    std::optional<QString> tryGetLocalDeviceName() const;
    std::optional<ScanMode> tryGetScanMode();
    std::optional<QOhosBluetoothErrorCode> trySetBluetoothScanMode(ScanMode scanMode);

Q_SIGNALS:
    void scanModeChanged(ScanMode scanMode);
    void missingPermission();

protected:
    QOhosBluetoothLocalDeviceProxy();

private:
    QOhosSupplier<std::optional<ScanMode>> m_scanModeSupplier;
};

}

QT_END_NAMESPACE

#endif // QOHOSBLUETOOTHLOCALDEVICE_P_H

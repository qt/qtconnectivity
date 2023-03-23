// Copyright (C) 2013 BlackBerry Limited. All rights reserved.
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QtBluetooth/qbluetoothdeviceinfo.h>

#include <QtCore/qlist.h>
#include <QtCore/qobject.h>

#include <QtQmlIntegration/qqmlintegration.h>

class DeviceInfo: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString deviceName READ getName NOTIFY deviceChanged)
    Q_PROPERTY(QString deviceAddress READ getAddress NOTIFY deviceChanged)

    QML_ANONYMOUS

public:
    DeviceInfo() = default;
    DeviceInfo(const QBluetoothDeviceInfo &d);
    QString getAddress() const;
    QString getName() const;
    QBluetoothDeviceInfo getDevice();
    void setDevice(const QBluetoothDeviceInfo &dev);

Q_SIGNALS:
    void deviceChanged();

private:
    QBluetoothDeviceInfo device;
};

#endif // DEVICEINFO_H

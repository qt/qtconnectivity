/***************************************************************************
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

#ifndef QLOWENERGYSERVICEDATA_H
#define QLOWENERGYSERVICEDATA_H

#include <QtBluetooth/qtbluetoothglobal.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QBluetoothUuid;
class QLowEnergyCharacteristicData;
class QLowEnergyService;
struct QLowEnergyServiceDataPrivate;

class Q_BLUETOOTH_EXPORT QLowEnergyServiceData
{
public:
    QLowEnergyServiceData();
    QLowEnergyServiceData(const QLowEnergyServiceData &other);
    ~QLowEnergyServiceData();

    QLowEnergyServiceData &operator=(const QLowEnergyServiceData &other);
    friend bool operator==(const QLowEnergyServiceData &a, const QLowEnergyServiceData &b)
    {
        return equals(a, b);
    }
    friend bool operator!=(const QLowEnergyServiceData &a, const QLowEnergyServiceData &b)
    {
        return !equals(a, b);
    }

    enum ServiceType { ServiceTypePrimary = 0x2800, ServiceTypeSecondary = 0x2801 };
    ServiceType type() const;
    void setType(ServiceType type);

    QBluetoothUuid uuid() const;
    void setUuid(const QBluetoothUuid &uuid);

    QList<QLowEnergyService *> includedServices() const;
    void setIncludedServices(const QList<QLowEnergyService *> &services);
    void addIncludedService(QLowEnergyService *service);

    QList<QLowEnergyCharacteristicData> characteristics() const;
    void setCharacteristics(const QList<QLowEnergyCharacteristicData> &characteristics);
    void addCharacteristic(const QLowEnergyCharacteristicData &characteristic);

    bool isValid() const;

    void swap(QLowEnergyServiceData &other) noexcept { d.swap(other.d); }

private:
    static bool equals(const QLowEnergyServiceData &a, const QLowEnergyServiceData &b);
    QSharedDataPointer<QLowEnergyServiceDataPrivate> d;
};

Q_DECLARE_SHARED(QLowEnergyServiceData)

QT_END_NAMESPACE

#endif // Include guard.

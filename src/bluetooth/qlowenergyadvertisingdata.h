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

#ifndef QLOWENERGYADVERTISINGDATA_H
#define QLOWENERGYADVERTISINGDATA_H

#include <QtBluetooth/qtbluetoothglobal.h>
#include <QtBluetooth/qbluetoothuuid.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QLowEnergyAdvertisingDataPrivate;

class Q_BLUETOOTH_EXPORT QLowEnergyAdvertisingData
{
public:
    QLowEnergyAdvertisingData();
    QLowEnergyAdvertisingData(const QLowEnergyAdvertisingData &other);
    ~QLowEnergyAdvertisingData();

    QLowEnergyAdvertisingData &operator=(const QLowEnergyAdvertisingData &other);
    friend bool operator==(const QLowEnergyAdvertisingData &a, const QLowEnergyAdvertisingData &b)
    {
        return equals(a, b);
    }
    friend bool operator!=(const QLowEnergyAdvertisingData &a, const QLowEnergyAdvertisingData &b)
    {
        return !equals(a, b);
    }

    void setLocalName(const QString &name);
    QString localName() const;

    static quint16 invalidManufacturerId() { return 0xffff; }
    void setManufacturerData(quint16 id, const QByteArray &data);
    quint16 manufacturerId() const;
    QByteArray manufacturerData() const;

    void setIncludePowerLevel(bool doInclude);
    bool includePowerLevel() const;

    enum Discoverability {
        DiscoverabilityNone, DiscoverabilityLimited, DiscoverabilityGeneral
    };
    void setDiscoverability(Discoverability mode);
    Discoverability discoverability() const;

    void setServices(const QList<QBluetoothUuid> &services);
    QList<QBluetoothUuid> services() const;

    // TODO: BR/EDR capability flag?

    void setRawData(const QByteArray &data);
    QByteArray rawData() const;

    void swap(QLowEnergyAdvertisingData &other) noexcept { d.swap(other.d); }

private:
    static bool equals(const QLowEnergyAdvertisingData &a, const QLowEnergyAdvertisingData &b);
    QSharedDataPointer<QLowEnergyAdvertisingDataPrivate> d;
};

Q_DECLARE_SHARED(QLowEnergyAdvertisingData)

QT_END_NAMESPACE

#endif // Include guard

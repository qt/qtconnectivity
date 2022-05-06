/***************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
****************************************************************************/
#ifndef QLOWENERGYCHARACTERISTICDATA_H
#define QLOWENERGYCHARACTERISTICDATA_H

#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QLowEnergyDescriptorData;
struct QLowEnergyCharacteristicDataPrivate;
class Q_BLUETOOTH_EXPORT QLowEnergyCharacteristicData
{
    friend Q_BLUETOOTH_EXPORT bool operator==(const QLowEnergyCharacteristicData &cd1,
                                              const QLowEnergyCharacteristicData &cd2);
public:
    QLowEnergyCharacteristicData();
    QLowEnergyCharacteristicData(const QLowEnergyCharacteristicData &other);
    ~QLowEnergyCharacteristicData();

    QLowEnergyCharacteristicData &operator=(const QLowEnergyCharacteristicData &other);

    QBluetoothUuid uuid() const;
    void setUuid(const QBluetoothUuid &uuid);

    QByteArray value() const;
    void setValue(const QByteArray &value);

    QLowEnergyCharacteristic::PropertyTypes properties() const;
    void setProperties(QLowEnergyCharacteristic::PropertyTypes properties);

    QList<QLowEnergyDescriptorData> descriptors() const;
    void setDescriptors(const QList<QLowEnergyDescriptorData> &descriptors);
    void addDescriptor(const QLowEnergyDescriptorData &descriptor);

    void setReadConstraints(QBluetooth::AttAccessConstraints constraints);
    QBluetooth::AttAccessConstraints readConstraints() const;

    void setWriteConstraints(QBluetooth::AttAccessConstraints constraints);
    QBluetooth::AttAccessConstraints writeConstraints() const;

    void setValueLength(int minimum, int maximum);
    int minimumValueLength() const;
    int maximumValueLength() const;

    bool isValid() const;

    void swap(QLowEnergyCharacteristicData &other) noexcept { d.swap(other.d); }

private:
    QSharedDataPointer<QLowEnergyCharacteristicDataPrivate> d;
};

Q_BLUETOOTH_EXPORT bool operator==(const QLowEnergyCharacteristicData &cd1,
                                   const QLowEnergyCharacteristicData &cd2);
inline bool operator!=(const QLowEnergyCharacteristicData &cd1,
                       const QLowEnergyCharacteristicData &cd2)
{
    return !(cd1 == cd2);
}

Q_DECLARE_SHARED(QLowEnergyCharacteristicData)

QT_END_NAMESPACE

#endif // Include guard.

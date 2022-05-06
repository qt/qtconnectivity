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

#ifndef QLOWENERGYDESCRIPTORDATA_H
#define QLOWENERGYDESCRIPTORDATA_H

#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/qbluetoothuuid.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QByteArray;
struct QLowEnergyDescriptorDataPrivate;

class Q_BLUETOOTH_EXPORT QLowEnergyDescriptorData
{
    friend Q_BLUETOOTH_EXPORT bool operator==(const QLowEnergyDescriptorData &d1,
                                              const QLowEnergyDescriptorData &d12);
public:
    QLowEnergyDescriptorData();
    QLowEnergyDescriptorData(const QBluetoothUuid &uuid,
                             const QByteArray &value);
    QLowEnergyDescriptorData(const QLowEnergyDescriptorData &other);
    ~QLowEnergyDescriptorData();

    QLowEnergyDescriptorData &operator=(const QLowEnergyDescriptorData &other);

    QByteArray value() const;
    void setValue(const QByteArray &value);

    QBluetoothUuid uuid() const;
    void setUuid(const QBluetoothUuid &uuid);

    bool isValid() const;

    void setReadPermissions(bool readable,
            QBluetooth::AttAccessConstraints constraints = QBluetooth::AttAccessConstraints());
    bool isReadable() const;
    QBluetooth::AttAccessConstraints readConstraints() const;

    void setWritePermissions(bool writable,
            QBluetooth::AttAccessConstraints constraints = QBluetooth::AttAccessConstraints());
    bool isWritable() const;
    QBluetooth::AttAccessConstraints writeConstraints() const;

    void swap(QLowEnergyDescriptorData &other) noexcept { d.swap(other.d); }

private:
    QSharedDataPointer<QLowEnergyDescriptorDataPrivate> d;
};

Q_BLUETOOTH_EXPORT bool operator==(const QLowEnergyDescriptorData &d1,
                                   const QLowEnergyDescriptorData &d2);

inline bool operator!=(const QLowEnergyDescriptorData &d1, const QLowEnergyDescriptorData &d2)
{
    return !(d1 == d2);
}

Q_DECLARE_SHARED(QLowEnergyDescriptorData)

QT_END_NAMESPACE

#endif // Include guard.

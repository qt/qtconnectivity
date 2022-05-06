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

#ifndef QLOWENERGYADVERTISINGPARAMETERS_H
#define QLOWENERGYADVERTISINGPARAMETERS_H

#include <QtBluetooth/qtbluetoothglobal.h>
#include <QtBluetooth/qbluetoothaddress.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtCore/qlist.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QLowEnergyAdvertisingParametersPrivate;

class Q_BLUETOOTH_EXPORT QLowEnergyAdvertisingParameters
{
    friend Q_BLUETOOTH_EXPORT bool operator==(const QLowEnergyAdvertisingParameters &p1,
                                              const QLowEnergyAdvertisingParameters &p2);
public:
    QLowEnergyAdvertisingParameters();
    QLowEnergyAdvertisingParameters(const QLowEnergyAdvertisingParameters &other);
    ~QLowEnergyAdvertisingParameters();

    QLowEnergyAdvertisingParameters &operator=(const QLowEnergyAdvertisingParameters &other);

    enum Mode { AdvInd = 0x0, AdvScanInd = 0x2, AdvNonConnInd = 0x3 };
    void setMode(Mode mode);
    Mode mode() const;

    struct AddressInfo {
        AddressInfo(const QBluetoothAddress &addr, QLowEnergyController::RemoteAddressType t)
            : address(addr), type(t) {}
        AddressInfo() : type(QLowEnergyController::PublicAddress) {}

        QBluetoothAddress address;
        QLowEnergyController::RemoteAddressType type;
    };
    enum FilterPolicy {
        IgnoreWhiteList = 0x00,
        UseWhiteListForScanning = 0x01,
        UseWhiteListForConnecting = 0x02,
        UseWhiteListForScanningAndConnecting = 0x03,
    };
    void setWhiteList(const QList<AddressInfo> &whiteList, FilterPolicy policy);
    QList<AddressInfo> whiteList() const;
    FilterPolicy filterPolicy() const;

    void setInterval(quint16 minimum, quint16 maximum);
    int minimumInterval() const;
    int maximumInterval() const;

    // TODO: own address type
    // TODO: For ADV_DIRECT_IND: peer address + peer address type

    void swap(QLowEnergyAdvertisingParameters &other) noexcept { d.swap(other.d); }

private:
    QSharedDataPointer<QLowEnergyAdvertisingParametersPrivate> d;
};

inline bool operator==(const QLowEnergyAdvertisingParameters::AddressInfo &ai1,
                       const QLowEnergyAdvertisingParameters::AddressInfo &ai2)
{
    return ai1.address == ai2.address && ai1.type == ai2.type;
}

Q_BLUETOOTH_EXPORT bool operator==(const QLowEnergyAdvertisingParameters &p1,
                                   const QLowEnergyAdvertisingParameters &p2);
inline bool operator!=(const QLowEnergyAdvertisingParameters &p1,
                       const QLowEnergyAdvertisingParameters &p2)
{
    return !(p1 == p2);
}

Q_DECLARE_SHARED(QLowEnergyAdvertisingParameters)

QT_END_NAMESPACE

#endif // Include guard

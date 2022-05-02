/****************************************************************************
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

#ifndef QLOWENERGYSERVICEPRIVATE_P_H
#define QLOWENERGYSERVICEPRIVATE_P_H

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

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/QLowEnergyService>
#include <QtBluetooth/QLowEnergyCharacteristic>

#if defined(QT_ANDROID_BLUETOOTH)
#include <QtCore/QJniObject>
#endif

QT_BEGIN_NAMESPACE

class QLowEnergyControllerPrivate;

class QLowEnergyServicePrivate : public QObject
{
    Q_OBJECT
public:
    explicit QLowEnergyServicePrivate(QObject *parent = nullptr);
    ~QLowEnergyServicePrivate();

    struct DescData {
        QByteArray value;
        QBluetoothUuid uuid;
    };

    struct CharData {
        QLowEnergyHandle valueHandle;
        QBluetoothUuid uuid;
        QLowEnergyCharacteristic::PropertyTypes properties;
        QByteArray value;
        QHash<QLowEnergyHandle, DescData> descriptorList;
    };

    enum GattAttributeTypes {
        PrimaryService = 0x2800,
        SecondaryService = 0x2801,
        IncludeAttribute = 0x2802,
        Characteristic = 0x2803
    };

    void setController(QLowEnergyControllerPrivate* control);
    void setError(QLowEnergyService::ServiceError newError);
    void setState(QLowEnergyService::ServiceState newState);

signals:
    void stateChanged(QLowEnergyService::ServiceState newState);
    void errorOccurred(QLowEnergyService::ServiceError error);
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                               const QByteArray &newValue);
    void characteristicRead(const QLowEnergyCharacteristic &info,
                            const QByteArray &value);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                               const QByteArray &newValue);
    void descriptorRead(const QLowEnergyDescriptor &info,
                        const QByteArray &value);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor,
                           const QByteArray &newValue);

public:
    QLowEnergyHandle startHandle = 0;
    QLowEnergyHandle endHandle = 0;

    QBluetoothUuid uuid;
    QList<QBluetoothUuid> includedServices;
    QLowEnergyService::ServiceTypes type = QLowEnergyService::PrimaryService;
    QLowEnergyService::ServiceState state = QLowEnergyService::InvalidService;
    QLowEnergyService::ServiceError lastError = QLowEnergyService::NoError;
    QLowEnergyService::DiscoveryMode mode = QLowEnergyService::FullDiscovery;

    QHash<QLowEnergyHandle, CharData> characteristicList;

    QPointer<QLowEnergyControllerPrivate> controller;

#if defined(QT_ANDROID_BLUETOOTH)
    // reference to the BluetoothGattService object
    QJniObject androidService;
#endif

};

typedef QHash<QLowEnergyHandle, QLowEnergyServicePrivate::CharData> CharacteristicDataMap;
typedef QHash<QLowEnergyHandle, QLowEnergyServicePrivate::DescData> DescriptorDataMap;

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QSharedPointer<QLowEnergyServicePrivate>)

#endif // QLOWENERGYSERVICEPRIVATE_P_H

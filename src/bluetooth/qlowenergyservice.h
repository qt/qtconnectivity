/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Copyright (C) 2016 Javier S. Pedro <maemo@javispedro.com>
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
#ifndef QLOWENERGYSERVICE_H
#define QLOWENERGYSERVICE_H

#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothUuid>
#include <QtBluetooth/QLowEnergyCharacteristic>

QT_BEGIN_NAMESPACE

class QLowEnergyServicePrivate;
class Q_BLUETOOTH_EXPORT QLowEnergyService : public QObject
{
    Q_OBJECT
public:
    enum ServiceType {
        PrimaryService = 0x0001,
        IncludedService = 0x0002
    };
    Q_ENUM(ServiceType)
    Q_DECLARE_FLAGS(ServiceTypes, ServiceType)

    enum ServiceError {
        NoError = 0,
        OperationError,
        CharacteristicWriteError,
        DescriptorWriteError,
        UnknownError,
        CharacteristicReadError,
        DescriptorReadError
    };
    Q_ENUM(ServiceError)

    enum ServiceState {
        InvalidService = 0,
        DiscoveryRequired,  // we know start/end handle but nothing more
        //TODO Rename DiscoveringServices -> DiscoveringDetails or DiscoveringService
        DiscoveringServices,// discoverDetails() called and running
        ServiceDiscovered,  // all details have been synchronized
        LocalService,
    };
    Q_ENUM(ServiceState)

    enum WriteMode {
        WriteWithResponse = 0,
        WriteWithoutResponse,
        WriteSigned
    };
    Q_ENUM(WriteMode)

    ~QLowEnergyService();

    QList<QBluetoothUuid> includedServices() const;

    QLowEnergyService::ServiceTypes type() const;
    QLowEnergyService::ServiceState state() const;

    QLowEnergyCharacteristic characteristic(const QBluetoothUuid &uuid) const;
    QList<QLowEnergyCharacteristic> characteristics() const;
    QBluetoothUuid serviceUuid() const;
    QString serviceName() const;

    void discoverDetails();

    ServiceError error() const;

    bool contains(const QLowEnergyCharacteristic &characteristic) const;
    void readCharacteristic(const QLowEnergyCharacteristic &characteristic);
    void writeCharacteristic(const QLowEnergyCharacteristic &characteristic,
                             const QByteArray &newValue,
                             WriteMode mode = WriteWithResponse);

    bool contains(const QLowEnergyDescriptor &descriptor) const;
    void readDescriptor(const QLowEnergyDescriptor &descriptor);
    void writeDescriptor(const QLowEnergyDescriptor &descriptor,
                         const QByteArray &newValue);

Q_SIGNALS:
    void stateChanged(QLowEnergyService::ServiceState newState);
    void characteristicChanged(const QLowEnergyCharacteristic &info,
                               const QByteArray &value);
    void characteristicRead(const QLowEnergyCharacteristic &info,
                            const QByteArray &value);
    void characteristicWritten(const QLowEnergyCharacteristic &info,
                               const QByteArray &value);
    void descriptorRead(const QLowEnergyDescriptor &info,
                        const QByteArray &value);
    void descriptorWritten(const QLowEnergyDescriptor &info,
                           const QByteArray &value);
    void error(QLowEnergyService::ServiceError error);

private:
    Q_DECLARE_PRIVATE(QLowEnergyService)
    QSharedPointer<QLowEnergyServicePrivate> d_ptr;

    // QLowEnergyController is the factory for this class
    friend class QLowEnergyController;
    friend class QLowEnergyControllerPrivate;
    friend class QLowEnergyControllerPrivateBluez;
    friend class QLowEnergyControllerPrivateAndroid;
    friend class QLowEnergyControllerPrivateDarwin;
    QLowEnergyService(QSharedPointer<QLowEnergyServicePrivate> p,
                      QObject *parent = nullptr);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QLowEnergyService::ServiceTypes)

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QLowEnergyService::ServiceError)
Q_DECLARE_METATYPE(QLowEnergyService::ServiceState)
Q_DECLARE_METATYPE(QLowEnergyService::ServiceType)
Q_DECLARE_METATYPE(QLowEnergyService::WriteMode)

#endif // QLOWENERGYSERVICE_H

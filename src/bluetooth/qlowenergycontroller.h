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

#ifndef QLOWENERGYCONTROLLER_H
#define QLOWENERGYCONTROLLER_H

#include <QtCore/QObject>
#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothDeviceInfo>
#include <QtBluetooth/QBluetoothUuid>
#include <QtBluetooth/QLowEnergyAdvertisingData>
#include <QtBluetooth/QLowEnergyConnectionParameters>
#include <QtBluetooth/QLowEnergyService>

QT_BEGIN_NAMESPACE

class QLowEnergyAdvertisingParameters;
class QLowEnergyControllerPrivate;
class QLowEnergyServiceData;

class Q_BLUETOOTH_EXPORT QLowEnergyController : public QObject
{
    Q_OBJECT
public:
    enum Error {
        NoError,
        UnknownError,
        UnknownRemoteDeviceError,
        NetworkError,
        InvalidBluetoothAdapterError,
        ConnectionError,
        AdvertisingError,
        RemoteHostClosedError,
        AuthorizationError
    };
    Q_ENUM(Error)

    enum ControllerState {
        UnconnectedState = 0,
        ConnectingState,
        ConnectedState,
        DiscoveringState,
        DiscoveredState,
        ClosingState,
        AdvertisingState,
    };
    Q_ENUM(ControllerState)

    enum RemoteAddressType {
        PublicAddress = 0,
        RandomAddress
    };
    Q_ENUM(RemoteAddressType)

    enum Role { CentralRole, PeripheralRole };
    Q_ENUM(Role)

    static QLowEnergyController *createCentral(const QBluetoothDeviceInfo &remoteDevice,
                                               QObject *parent = nullptr);
    static QLowEnergyController *createCentral(const QBluetoothDeviceInfo &remoteDevice,
                                               const QBluetoothAddress &localDevice,
                                               QObject *parent = nullptr);
    static QLowEnergyController *createPeripheral(const QBluetoothAddress &localDevice,
                                                  QObject *parent = nullptr);
    static QLowEnergyController *createPeripheral(QObject *parent = nullptr);

    // TODO: Allow to set connection timeout (disconnect when no data has been exchanged for n seconds).

    ~QLowEnergyController();

    QBluetoothAddress localAddress() const;
    QBluetoothAddress remoteAddress() const;
    QBluetoothUuid remoteDeviceUuid() const;

    QString remoteName() const;

    ControllerState state() const;

    // TODO Qt6 remove this property. It is not longer needed when using Bluez DBus backend
    RemoteAddressType remoteAddressType() const;
    void setRemoteAddressType(RemoteAddressType type);

    void connectToDevice();
    void disconnectFromDevice();

    void discoverServices();
    QList<QBluetoothUuid> services() const;
    QLowEnergyService *createServiceObject(const QBluetoothUuid &service, QObject *parent = nullptr);

    void startAdvertising(const QLowEnergyAdvertisingParameters &parameters,
                          const QLowEnergyAdvertisingData &advertisingData,
                          const QLowEnergyAdvertisingData &scanResponseData = QLowEnergyAdvertisingData());
    void stopAdvertising();

    QLowEnergyService *addService(const QLowEnergyServiceData &service, QObject *parent = nullptr);

    void requestConnectionUpdate(const QLowEnergyConnectionParameters &parameters);

    Error error() const;
    QString errorString() const;

    Role role() const;

    int mtu() const;

Q_SIGNALS:
    void connected();
    void disconnected();
    void stateChanged(QLowEnergyController::ControllerState state);
    void errorOccurred(QLowEnergyController::Error newError);
    void mtuChanged(int mtu);

    void serviceDiscovered(const QBluetoothUuid &newService);
    void discoveryFinished();
    void connectionUpdated(const QLowEnergyConnectionParameters &parameters);

private:
    // peripheral role ctor
    explicit QLowEnergyController(const QBluetoothAddress &localDevice, QObject *parent = nullptr);

    // central role ctors
    explicit QLowEnergyController(const QBluetoothDeviceInfo &remoteDevice,
                                  const QBluetoothAddress &localDevice,
                                  QObject *parent = nullptr);


    Q_DECLARE_PRIVATE(QLowEnergyController)
    QLowEnergyControllerPrivate *d_ptr;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QLowEnergyController::Error)
Q_DECLARE_METATYPE(QLowEnergyController::ControllerState)
Q_DECLARE_METATYPE(QLowEnergyController::RemoteAddressType)
Q_DECLARE_METATYPE(QLowEnergyController::Role)

#endif // QLOWENERGYCONTROLLER_H

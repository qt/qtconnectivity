/****************************************************************************
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

#ifndef QLOWENERGYCONTROLLERPRIVATEWINRT_P_H
#define QLOWENERGYCONTROLLERPRIVATEWINRT_P_H

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

#include <qglobal.h>
#include <QtCore/QQueue>
#include <QtCore/QVector>
#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergyservicedata.h>
#include "qlowenergycontroller.h"
#include "qlowenergycontrollerbase_p.h"

#include <wrl.h>
#include <windows.devices.bluetooth.h>

#include <functional>

QT_BEGIN_NAMESPACE

class QLowEnergyServiceData;
class QTimer;
class QWinRTLowEnergyServiceHandler;

extern void registerQLowEnergyControllerMetaType();

class QLowEnergyControllerPrivateWinRT final : public QLowEnergyControllerPrivate
{
    Q_OBJECT
public:
    QLowEnergyControllerPrivateWinRT();
    ~QLowEnergyControllerPrivateWinRT() override;

    void init() override;

    void connectToDevice() override;
    void disconnectFromDevice() override;

    void discoverServices() override;
    void discoverServiceDetails(const QBluetoothUuid &service) override;

    void startAdvertising(const QLowEnergyAdvertisingParameters &params,
                          const QLowEnergyAdvertisingData &advertisingData,
                          const QLowEnergyAdvertisingData &scanResponseData) override;
    void stopAdvertising() override;

    void requestConnectionUpdate(const QLowEnergyConnectionParameters &params) override;

    // read data
    void readCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> service,
                            const QLowEnergyHandle charHandle) override;
    void readDescriptor(const QSharedPointer<QLowEnergyServicePrivate> service,
                        const QLowEnergyHandle charHandle,
                        const QLowEnergyHandle descriptorHandle) override;

    // write data
    void writeCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> service,
                             const QLowEnergyHandle charHandle,
                             const QByteArray &newValue, QLowEnergyService::WriteMode mode) override;
    void writeDescriptor(const QSharedPointer<QLowEnergyServicePrivate> service,
                         const QLowEnergyHandle charHandle,
                         const QLowEnergyHandle descriptorHandle,
                         const QByteArray &newValue) override;

    void addToGenericAttributeList(const QLowEnergyServiceData &service,
                                   QLowEnergyHandle startHandle) override;

signals:
    void characteristicChanged(quint16 charHandle, const QByteArray &data);

private slots:
    void handleCharacteristicChanged(quint16 charHandle, const QByteArray &data);
    void doConnectToDevice();

private:
    Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::IBluetoothLEDevice> mDevice;
    EventRegistrationToken mStatusChangedToken;
    struct ValueChangedEntry {
        ValueChangedEntry() {}
        ValueChangedEntry(Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::IGattCharacteristic> c,
                          EventRegistrationToken t)
            : characteristic(c)
            , token(t)
        {
        }

        Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::IGattCharacteristic> characteristic;
        EventRegistrationToken token;
    };
    QVector<ValueChangedEntry> mValueChangedTokens;

    Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::IGattDeviceService> getNativeService(const QBluetoothUuid &serviceUuid);
    Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::IGattCharacteristic> getNativeCharacteristic(const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid);

    void registerForValueChanges(const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid);
    void unregisterFromValueChanges();

    void obtainIncludedServices(QSharedPointer<QLowEnergyServicePrivate> servicePointer,
        Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::IGattDeviceService> nativeService);

};

QT_END_NAMESPACE

#endif // QLOWENERGYCONTROLLERPRIVATEWINRT_P_H

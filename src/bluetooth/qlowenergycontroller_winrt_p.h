// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
#include <QtCore/QList>
#include <QtCore/QQueue>
#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergyservicedata.h>
#include "qlowenergycontroller.h"
#include "qlowenergycontrollerbase_p.h"

namespace ABI {
    namespace Windows {
        namespace Devices {
            namespace Bluetooth {
                struct IBluetoothLEDevice;
            }
        }
        namespace Foundation {
            template <typename T> struct IAsyncOperation;
            enum class AsyncStatus;
        }
    }
}

#include <wrl.h>
#include <windows.devices.bluetooth.genericattributeprofile.h>

#include <functional>

QT_BEGIN_NAMESPACE

class QLowEnergyServiceData;
class QTimer;

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
    void discoverServiceDetails(const QBluetoothUuid &service,
                                QLowEnergyService::DiscoveryMode mode) override;

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

    int mtu() const override;

signals:
    void characteristicChanged(quint16 charHandle, const QByteArray &data);
    void abortConnection();

private slots:
    void handleCharacteristicChanged(quint16 charHandle, const QByteArray &data);
    void handleServiceHandlerError(const QString &error);

private:
    void handleConnectionError(const char *logMessage);

    Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::IBluetoothLEDevice> mDevice;
    Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::IGattSession>
            mGattSession;
    EventRegistrationToken mStatusChangedToken;
    EventRegistrationToken mMtuChangedToken;
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
    QList<ValueChangedEntry> mValueChangedTokens;

    using GattDeviceServiceComPtr = Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::IGattDeviceService>;
    QMap<QBluetoothUuid, GattDeviceServiceComPtr> m_openedServices;
    QSet<QBluetoothUuid> m_requestDetailsServiceUuids;

    using NativeServiceCallback = std::function<void(GattDeviceServiceComPtr)>;
    HRESULT getNativeService(const QBluetoothUuid &serviceUuid, NativeServiceCallback callback);

    using GattCharacteristicComPtr = Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::IGattCharacteristic>;
    using NativeCharacteristicCallback = std::function<void(GattCharacteristicComPtr)>;
    HRESULT getNativeCharacteristic(const QBluetoothUuid &serviceUuid,
                                    const QBluetoothUuid &charUuid,
                                    NativeCharacteristicCallback callback);

    void registerForValueChanges(const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid);
    void unregisterFromValueChanges();
    HRESULT onValueChange(ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::IGattCharacteristic *characteristic,
                          ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::IGattValueChangedEventArgs *args);
    HRESULT onMtuChange(ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::IGattSession *session,
                        IInspectable *args);
    bool registerForMtuChanges();
    void unregisterFromMtuChanges();

    bool registerForStatusChanges();
    void unregisterFromStatusChanges();
    HRESULT onStatusChange(ABI::Windows::Devices::Bluetooth::IBluetoothLEDevice *dev, IInspectable *);

    void obtainIncludedServices(QSharedPointer<QLowEnergyServicePrivate> servicePointer,
        Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::IGattDeviceService> nativeService);
    HRESULT onServiceDiscoveryFinished(ABI::Windows::Foundation::IAsyncOperation<ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceServicesResult *> *op,
                                       ABI::Windows::Foundation::AsyncStatus status);

    void readCharacteristicHelper(const QSharedPointer<QLowEnergyServicePrivate> service,
                                  const QLowEnergyHandle charHandle,
                                  GattCharacteristicComPtr characteristic);
    void readDescriptorHelper(const QSharedPointer<QLowEnergyServicePrivate> service,
                              const QLowEnergyHandle charHandle,
                              const QLowEnergyHandle descriptorHandle,
                              GattCharacteristicComPtr characteristic);
    void writeCharacteristicHelper(const QSharedPointer<QLowEnergyServicePrivate> service,
                                   const QLowEnergyHandle charHandle, const QByteArray &newValue,
                                   bool writeWithResponse,
                                   GattCharacteristicComPtr characteristic);
    void writeDescriptorHelper(const QSharedPointer<QLowEnergyServicePrivate> service,
                               const QLowEnergyHandle charHandle,
                               const QLowEnergyHandle descriptorHandle,
                               const QByteArray &newValue,
                               GattCharacteristicComPtr characteristic);
    void discoverServiceDetailsHelper(const QBluetoothUuid &service,
                                      QLowEnergyService::DiscoveryMode mode,
                                      GattDeviceServiceComPtr deviceService);

    void clearAllServices();
    void closeAndRemoveService(const QBluetoothUuid &uuid);
};

QT_END_NAMESPACE

#endif // QLOWENERGYCONTROLLERPRIVATEWINRT_P_H

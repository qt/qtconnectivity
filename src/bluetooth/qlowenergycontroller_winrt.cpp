/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qlowenergycontroller_p.h"

#include <QtCore/qfunctions_winrt.h>
#include <QtCore/QLoggingCategory>
#include <private/qeventdispatcher_winrt_p.h>

#include <functional>
#include <robuffer.h>
#include <windows.devices.enumeration.h>
#include <windows.devices.bluetooth.h>
#include <windows.foundation.collections.h>
#include <windows.storage.streams.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Devices;
using namespace ABI::Windows::Devices::Bluetooth;
using namespace ABI::Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace ABI::Windows::Devices::Enumeration;
using namespace ABI::Windows::Storage::Streams;

QT_BEGIN_NAMESPACE

typedef ITypedEventHandler<BluetoothLEDevice *, IInspectable *> StatusHandler;

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINRT)

QLowEnergyControllerPrivate::QLowEnergyControllerPrivate()
    : QObject(),
      state(QLowEnergyController::UnconnectedState),
      error(QLowEnergyController::NoError)
{
    qCDebug(QT_BT_WINRT) << __FUNCTION__;
}

QLowEnergyControllerPrivate::~QLowEnergyControllerPrivate()
{
    if (mDevice && mStatusChangedToken.value)
        mDevice->remove_ConnectionStatusChanged(mStatusChangedToken);
}

void QLowEnergyControllerPrivate::init()
{
}

void QLowEnergyControllerPrivate::connectToDevice()
{
    qCDebug(QT_BT_WINRT) << __FUNCTION__;
    Q_Q(QLowEnergyController);
    if (remoteDevice.isNull()) {
        qWarning() << "Invalid/null remote device address";
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        return;
    }

    setState(QLowEnergyController::ConnectingState);

    ComPtr<IBluetoothLEDeviceStatics> deviceStatics;
    HRESULT hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_BluetoothLEDevice).Get(), &deviceStatics);
    Q_ASSERT_SUCCEEDED(hr);
    ComPtr<IAsyncOperation<BluetoothLEDevice *>> deviceFromIdOperation;
    hr = deviceStatics->FromBluetoothAddressAsync(remoteDevice.toUInt64(), &deviceFromIdOperation);
    Q_ASSERT_SUCCEEDED(hr);
    hr = QWinRTFunctions::await(deviceFromIdOperation, mDevice.GetAddressOf());
    Q_ASSERT_SUCCEEDED(hr);

    if (!mDevice) {
        qCDebug(QT_BT_WINRT) << "Could not find LE device";
        setError(QLowEnergyController::InvalidBluetoothAdapterError);
        setState(QLowEnergyController::UnconnectedState);
    }
    BluetoothConnectionStatus status;
    hr = mDevice->get_ConnectionStatus(&status);
    Q_ASSERT_SUCCEEDED(hr);
    hr = mDevice->add_ConnectionStatusChanged(Callback<StatusHandler>([this, q](IBluetoothLEDevice *dev, IInspectable *){
        BluetoothConnectionStatus status;
        dev->get_ConnectionStatus(&status);
        if (status == BluetoothConnectionStatus::BluetoothConnectionStatus_Connected) {
            setState(QLowEnergyController::ConnectedState);
            emit q->connected();
        } else if (status == BluetoothConnectionStatus::BluetoothConnectionStatus_Disconnected) {
            setState(QLowEnergyController::UnconnectedState);
            emit q->disconnected();
        }
        return S_OK;
    }).Get(), &mStatusChangedToken);
    Q_ASSERT_SUCCEEDED(hr);

    if (status == BluetoothConnectionStatus::BluetoothConnectionStatus_Connected) {
        setState(QLowEnergyController::ConnectedState);
        emit q->connected();
        return;
    }

    IVectorView <GattDeviceService *> *deviceServices;
    hr = mDevice->get_GattServices(&deviceServices);
    Q_ASSERT_SUCCEEDED(hr);
    uint serviceCount;
    hr = deviceServices->get_Size(&serviceCount);
    Q_ASSERT_SUCCEEDED(hr);
    // Windows Phone automatically connects to the device as soon as a service value is read/written.
    // Thus we read one value in order to establish the connection.
    for (uint i = 0; i < serviceCount; ++i) {
        ComPtr<IGattDeviceService> service;
        hr = deviceServices->GetAt(i, &service);
        Q_ASSERT_SUCCEEDED(hr);
        ComPtr<IGattDeviceService2> service2;
        hr = service.As(&service2);
        Q_ASSERT_SUCCEEDED(hr);
        ComPtr<IVectorView<GattCharacteristic *>> characteristics;
        hr = service2->GetAllCharacteristics(&characteristics);
        if (hr == E_ACCESSDENIED) {
            setError(QLowEnergyController::ConnectionError);
            return;
        } else {
            Q_ASSERT_SUCCEEDED(hr);
        }
        uint characteristicsCount;
        hr = characteristics->get_Size(&characteristicsCount);
        Q_ASSERT_SUCCEEDED(hr);
        for (uint j = 0; j < characteristicsCount; ++j) {
            ComPtr<IGattCharacteristic> characteristic;
            hr = characteristics->GetAt(j, &characteristic);
            Q_ASSERT_SUCCEEDED(hr);
            ComPtr<IAsyncOperation<GattReadResult *>> op;
            GattCharacteristicProperties props;
            hr = characteristic->get_CharacteristicProperties(&props);
            Q_ASSERT_SUCCEEDED(hr);
            if (!(props & GattCharacteristicProperties_Read))
                continue;
            hr = characteristic->ReadValueWithCacheModeAsync(BluetoothCacheMode::BluetoothCacheMode_Uncached, &op);
            Q_ASSERT_SUCCEEDED(hr);
            ComPtr<IGattReadResult> result;
            hr = QWinRTFunctions::await(op, result.GetAddressOf());
            Q_ASSERT_SUCCEEDED(hr);
            ComPtr<ABI::Windows::Storage::Streams::IBuffer> buffer;
            hr = result->get_Value(&buffer);
            Q_ASSERT_SUCCEEDED(hr);
            if (!buffer) {
                qCDebug(QT_BT_WINRT) << "Problem reading value";
                setError(QLowEnergyController::ConnectionError);
                setState(QLowEnergyController::UnconnectedState);
            }
            return;
        }
    }
}

void QLowEnergyControllerPrivate::disconnectFromDevice()
{
    qCDebug(QT_BT_WINRT) << __FUNCTION__;
    Q_Q(QLowEnergyController);
    setState(QLowEnergyController::UnconnectedState);
    emit q->disconnected();
}

void QLowEnergyControllerPrivate::obtainIncludedServices(QSharedPointer<QLowEnergyServicePrivate> servicePointer,
    ComPtr<IGattDeviceService> service)
{
    Q_Q(QLowEnergyController);
    ComPtr<IGattDeviceService2> service2;
    HRESULT hr = service.As(&service2);
    Q_ASSERT_SUCCEEDED(hr);
    ComPtr<IVectorView<GattDeviceService *>> includedServices;
    hr = service2->GetAllIncludedServices(&includedServices);
    Q_ASSERT_SUCCEEDED(hr);

    uint count;
    includedServices->get_Size(&count);
    for (uint i = 0; i < count; ++i) {
        ComPtr<IGattDeviceService> includedService;
        hr = includedServices->GetAt(i, &includedService);
        Q_ASSERT_SUCCEEDED(hr);
        GUID guuid;
        hr = includedService->get_Uuid(&guuid);
        Q_ASSERT_SUCCEEDED(hr);
        const QBluetoothUuid includedUuid(guuid);
        QSharedPointer<QLowEnergyServicePrivate> includedPointer;
        if (serviceList.contains(includedUuid)) {
            includedPointer = serviceList.value(includedUuid);
        } else {
            QLowEnergyServicePrivate *priv = new QLowEnergyServicePrivate();
            priv->uuid = includedUuid;
            priv->setController(this);

            includedPointer = QSharedPointer<QLowEnergyServicePrivate>(priv);
            serviceList.insert(includedUuid, includedPointer);
        }
        includedPointer->type |= QLowEnergyService::IncludedService;
        servicePointer->includedServices.append(includedUuid);

        obtainIncludedServices(includedPointer, includedService);

        emit q->serviceDiscovered(includedUuid);
    }
}

void QLowEnergyControllerPrivate::discoverServices()
{
    Q_Q(QLowEnergyController);

    qCDebug(QT_BT_WINRT) << "Service discovery initiated";
    ComPtr<IVectorView<GattDeviceService *>> deviceServices;
    HRESULT hr = mDevice->get_GattServices(&deviceServices);
    Q_ASSERT_SUCCEEDED(hr);
    uint serviceCount;
    hr = deviceServices->get_Size(&serviceCount);
    Q_ASSERT_SUCCEEDED(hr);
    for (uint i = 0; i < serviceCount; ++i) {
        ComPtr<IGattDeviceService> deviceService;
        hr = deviceServices->GetAt(i, &deviceService);
        Q_ASSERT_SUCCEEDED(hr);
        GUID guuid;
        hr = deviceService->get_Uuid(&guuid);
        Q_ASSERT_SUCCEEDED(hr);
        const QBluetoothUuid service(guuid);

        QSharedPointer<QLowEnergyServicePrivate> pointer;
        if (serviceList.contains(service)) {
            pointer = serviceList.value(service);
        } else {
            QLowEnergyServicePrivate *priv = new QLowEnergyServicePrivate();
            priv->uuid = service;
            priv->setController(this);

            pointer = QSharedPointer<QLowEnergyServicePrivate>(priv);
            serviceList.insert(service, pointer);
        }
        pointer->type |= QLowEnergyService::PrimaryService;

        obtainIncludedServices(pointer, deviceService);

        emit q->serviceDiscovered(service);
    }

    setState(QLowEnergyController::DiscoveredState);
    emit q->discoveryFinished();
}

void QLowEnergyControllerPrivate::discoverServiceDetails(const QBluetoothUuid &)
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivate::startAdvertising(const QLowEnergyAdvertisingParameters &, const QLowEnergyAdvertisingData &, const QLowEnergyAdvertisingData &)
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivate::stopAdvertising()
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivate::requestConnectionUpdate(const QLowEnergyConnectionParameters &)
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivate::readCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> ,
                        const QLowEnergyHandle)
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivate::readDescriptor(const QSharedPointer<QLowEnergyServicePrivate>,
                    const QLowEnergyHandle,
                    const QLowEnergyHandle)
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivate::writeCharacteristic(const QSharedPointer<QLowEnergyServicePrivate>,
        const QLowEnergyHandle,
        const QByteArray &,
        QLowEnergyService::WriteMode)
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivate::writeDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate>,
        const QLowEnergyHandle,
        const QLowEnergyHandle,
        const QByteArray &)
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivate::addToGenericAttributeList(const QLowEnergyServiceData &, QLowEnergyHandle)
{
    Q_UNIMPLEMENTED();
}

QT_END_NAMESPACE

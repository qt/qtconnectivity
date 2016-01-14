/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothdevicediscoveryagent_p.h"
#include "qbluetoothaddress.h"
#include "qbluetoothuuid.h"
#include "qfunctions_winrt.h"

#include <QtCore/QLoggingCategory>
#include <QtCore/private/qeventdispatcher_winrt_p.h>

#include <wrl.h>
#include <windows.devices.enumeration.h>
#include <windows.devices.bluetooth.h>
#include <windows.foundation.collections.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Devices;
using namespace ABI::Windows::Devices::Bluetooth;
using namespace ABI::Windows::Devices::Enumeration;

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINRT)

#define WARN_AND_RETURN_IF_FAILED(msg, ret) \
    if (FAILED(hr)) { \
        qCWarning(QT_BT_WINRT) << msg; \
        ret; \
    }

QBluetoothDeviceInfo bluetoothInfoFromDeviceId(HSTRING deviceId)
{
    ComPtr<IBluetoothDeviceStatics> deviceStatics;
    ComPtr<IBluetoothDevice> device;
    HRESULT hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_BluetoothDevice).Get(), &deviceStatics);
    WARN_AND_RETURN_IF_FAILED("Could not obtain bluetooth device statics", return QBluetoothDeviceInfo());
    ComPtr<IAsyncOperation<BluetoothDevice *>> deviceFromIdOperation;
    hr = deviceStatics->FromIdAsync(deviceId, &deviceFromIdOperation);
    WARN_AND_RETURN_IF_FAILED("Could not obtain bluetooth device from id", return QBluetoothDeviceInfo());
    QWinRTFunctions::await(deviceFromIdOperation, device.GetAddressOf());
    WARN_AND_RETURN_IF_FAILED("Could not wait for bluetooth device operation to finish", return QBluetoothDeviceInfo());
    if (!device)
        return QBluetoothDeviceInfo();
    UINT64 address;
    HString name;
    ComPtr<IBluetoothClassOfDevice> classOfDevice;
    UINT32 classOfDeviceInt;
    device->get_BluetoothAddress(&address);
    device->get_Name(name.GetAddressOf());
    const QString btName = QString::fromWCharArray(WindowsGetStringRawBuffer(name.Get(), nullptr));
    device->get_ClassOfDevice(&classOfDevice);
    classOfDevice->get_RawValue(&classOfDeviceInt);
    IVectorView <Rfcomm::RfcommDeviceService *> *deviceServices;
    hr = device->get_RfcommServices(&deviceServices);
    WARN_AND_RETURN_IF_FAILED("Could not obtain bluetooth device services", return QBluetoothDeviceInfo());
    uint serviceCount;
    deviceServices->get_Size(&serviceCount);
    QList<QBluetoothUuid> uuids;
    for (uint i = 0; i < serviceCount; ++i) {
        ComPtr<Rfcomm::IRfcommDeviceService> service;
        deviceServices->GetAt(i, &service);
        ComPtr<Rfcomm::IRfcommServiceId> id;
        service->get_ServiceId(&id);
        GUID uuid;
        id->get_Uuid(&uuid);
        uuids.append(QBluetoothUuid(uuid));
    }

    qCDebug(QT_BT_WINRT) << "Discovered BT device: " << QString::number(address) << btName
        << "Num UUIDs" << uuids.count();

    QBluetoothDeviceInfo info(QBluetoothAddress(address), btName, classOfDeviceInt);
    info.setCoreConfigurations(QBluetoothDeviceInfo::BaseRateCoreConfiguration);
    info.setServiceUuids(uuids, QBluetoothDeviceInfo::DataIncomplete);
    info.setCached(true);

    return info;
}

QBluetoothDeviceInfo bluetoothInfoFromLeDeviceId(HSTRING deviceId)
{
    ComPtr<IBluetoothLEDeviceStatics> deviceStatics;
    ComPtr<IBluetoothLEDevice> device;
    HRESULT hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_BluetoothLEDevice).Get(), &deviceStatics);
    WARN_AND_RETURN_IF_FAILED("Could not obtain bluetooth LE device statics", return QBluetoothDeviceInfo());
    ComPtr<IAsyncOperation<BluetoothLEDevice *>> deviceFromIdOperation;
    hr = deviceStatics->FromIdAsync(deviceId, &deviceFromIdOperation);
    WARN_AND_RETURN_IF_FAILED("Could not obtain bluetooth LE device from id", return QBluetoothDeviceInfo());
    QWinRTFunctions::await(deviceFromIdOperation, device.GetAddressOf());
    WARN_AND_RETURN_IF_FAILED("Could not wait for bluetooth LE device operation to finish", return QBluetoothDeviceInfo());
    if (!device)
        return QBluetoothDeviceInfo();
    UINT64 address;
    HString name;
    device->get_BluetoothAddress(&address);
    device->get_Name(name.GetAddressOf());
    const QString btName = QString::fromWCharArray(WindowsGetStringRawBuffer(name.Get(), nullptr));
    IVectorView <GenericAttributeProfile::GattDeviceService *> *deviceServices;
    hr = device->get_GattServices(&deviceServices);
    WARN_AND_RETURN_IF_FAILED("Could not obtain bluetooth LE device services", return QBluetoothDeviceInfo());
    uint serviceCount;
    deviceServices->get_Size(&serviceCount);
    QList<QBluetoothUuid> uuids;
    for (uint i = 0; i < serviceCount; ++i) {
        ComPtr<GenericAttributeProfile::IGattDeviceService> service;
        deviceServices->GetAt(i, &service);
        ComPtr<Rfcomm::IRfcommServiceId> id;
        GUID uuid;
        service->get_Uuid(&uuid);
        uuids.append(QBluetoothUuid(uuid));
    }

    qCDebug(QT_BT_WINRT) << "Discovered BTLE device: " << QString::number(address) << btName
        << "Num UUIDs" << uuids.count();

    QBluetoothDeviceInfo info(QBluetoothAddress(address), btName, 0);
    info.setCoreConfigurations(QBluetoothDeviceInfo::LowEnergyCoreConfiguration);
    info.setServiceUuids(uuids, QBluetoothDeviceInfo::DataIncomplete);
    info.setCached(true);

    return info;
}

class QWinRTBluetoothDeviceDiscoveryWorker : public QObject
{
    Q_OBJECT
public:
    QWinRTBluetoothDeviceDiscoveryWorker() : initializedModes(0)
    {
    }

    void start()
    {
        QEventDispatcherWinRT::runOnXamlThread([this]() {
            startDeviceDiscovery(BT);

            startDeviceDiscovery(BTLE);
            return S_OK;
        });

        qCDebug(QT_BT_WINRT) << "Worker started";
    }

    ~QWinRTBluetoothDeviceDiscoveryWorker()
    {
    }

private:
    enum BTMode {
        BT = 0x1,
        BTLE = 0x2,
        BTAll = BT|BTLE
    };

    void startDeviceDiscovery(BTMode mode)
    {
        HString deviceSelector;
        ComPtr<IDeviceInformationStatics> deviceInformationStatics;
        HRESULT hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Enumeration_DeviceInformation).Get(), &deviceInformationStatics);
        WARN_AND_RETURN_IF_FAILED("Could not obtain device information statics", return);
        if (mode == BTLE) {
            ComPtr<IBluetoothLEDeviceStatics> bluetoothLeDeviceStatics;
            hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_BluetoothLEDevice).Get(), &bluetoothLeDeviceStatics);
            WARN_AND_RETURN_IF_FAILED("Could not obtain bluetooth LE device statics", return);
            bluetoothLeDeviceStatics->GetDeviceSelector(deviceSelector.GetAddressOf());
        } else {
            ComPtr<IBluetoothDeviceStatics> bluetoothDeviceStatics;
            hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_BluetoothDevice).Get(), &bluetoothDeviceStatics);
            WARN_AND_RETURN_IF_FAILED("Could not obtain bluetooth device statics", return);
            bluetoothDeviceStatics->GetDeviceSelector(deviceSelector.GetAddressOf());
        }
        ComPtr<IAsyncOperation<DeviceInformationCollection *>> op;
        hr = deviceInformationStatics->FindAllAsyncAqsFilter(deviceSelector.Get(), &op);
        WARN_AND_RETURN_IF_FAILED("Could not start bluetooth device discovery operation", return);
        op->put_Completed(
            Callback<IAsyncOperationCompletedHandler<DeviceInformationCollection *>>([this, mode](IAsyncOperation<DeviceInformationCollection *> *op, AsyncStatus) {
                onDeviceDiscoveryFinished(op, mode);
                return S_OK;
            }).Get());
        WARN_AND_RETURN_IF_FAILED("Could not add callback to bluetooth device discovery operation", return);
    }

    void onDeviceDiscoveryFinished(IAsyncOperation<DeviceInformationCollection *> *op, BTMode mode)
    {
        qCDebug(QT_BT_WINRT) << (mode == BT ? "BT" : "BTLE") << "scan completed";
        ComPtr<IVectorView<DeviceInformation *>> devices;
        op->GetResults(&devices);
        onDevicesFound(devices.Get(), mode);
        initializedModes |= mode;
        if (initializedModes == BTAll) {
            qCDebug(QT_BT_WINRT) << "All scans completed";
            emit initializationCompleted();
            deleteLater();
        }
    }

    void onDeviceAdded(IDeviceInformation *deviceInfo, BTMode mode)
    {
        HString deviceId;
        deviceInfo->get_Id(deviceId.GetAddressOf());
        const QBluetoothDeviceInfo info = mode == BTLE
            ? bluetoothInfoFromLeDeviceId(deviceId.Get())
            : bluetoothInfoFromDeviceId(deviceId.Get());
        if (!info.isValid())
            return;

        for (QVector<QBluetoothDeviceInfo>::iterator iter = deviceList.begin();
            iter != deviceList.end(); ++iter) {
            // one of them has to be the traditional device, the other one the BTLE version (found in both scans)
            if (iter->address() == info.address()) {
                qCDebug(QT_BT_WINRT) << "Updating device" << iter->name() << iter->address();
                // merge service uuids
                QList<QBluetoothUuid> uuids = iter->serviceUuids();
                uuids.append(info.serviceUuids());
                const QSet<QBluetoothUuid> uuidSet = uuids.toSet();
                if (iter->serviceUuids().count() != uuidSet.count())
                    iter->setServiceUuids(uuidSet.toList(), QBluetoothDeviceInfo::DataIncomplete);

                iter->setCoreConfigurations(QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration);
                return;
            }
        }
        qCDebug(QT_BT_WINRT) << "Adding device" << info.name() << info.address();
        deviceList.append(info);
    }

    void onDevicesFound(IVectorView<DeviceInformation *> *devices, BTMode mode)
    {
        quint32 deviceCount;
        HRESULT hr = devices->get_Size(&deviceCount);
        deviceList.reserve(deviceList.length() + deviceCount);
        for (quint32 i = 0; i < deviceCount; ++i) {
            ComPtr<IDeviceInformation> device;
            hr = devices->GetAt(i, &device);
            onDeviceAdded(device.Get(), mode);
        }
    }

Q_SIGNALS:
    void initializationCompleted();

public:
    QVector<QBluetoothDeviceInfo> deviceList;

private:
    quint8 initializedModes;
};

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(
                const QBluetoothAddress &deviceAdapter,
                QBluetoothDeviceDiscoveryAgent *parent)

    :   inquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry),
        lastError(QBluetoothDeviceDiscoveryAgent::NoError),
        q_ptr(parent)
{
    Q_UNUSED(deviceAdapter);
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
    disconnectAndClearWorker();
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    return worker;
}

void QBluetoothDeviceDiscoveryAgentPrivate::start()
{
    if (worker)
        return;

    // The worker handles its lifetime on its own (basically deletes itself as soon
    // as it's done with its work) to prevent windows callbacks that access objects
    // that have already been destroyed. Thus we create a new worker for every start
    // and just forget about it as soon as the operation is canceled or finished.
    worker = new QWinRTBluetoothDeviceDiscoveryWorker();
    discoveredDevices.clear();
    connect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::initializationCompleted,
        this, &QBluetoothDeviceDiscoveryAgentPrivate::onListInitializationCompleted);
    worker->start();
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    if (worker) {
        disconnectAndClearWorker();
        emit q->canceled();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::onListInitializationCompleted()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    discoveredDevices = worker->deviceList.toList();
    foreach (const QBluetoothDeviceInfo &info, worker->deviceList)
        emit q->deviceDiscovered(info);

    disconnectAndClearWorker();
    emit q->finished();
}

void QBluetoothDeviceDiscoveryAgentPrivate::disconnectAndClearWorker()
{
    if (!worker)
        return;

    disconnect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::initializationCompleted,
        this, &QBluetoothDeviceDiscoveryAgentPrivate::onListInitializationCompleted);
    // worker deletion is done by the worker itself (see comment in start())
    worker.clear();
}

QT_END_NAMESPACE

#include <qbluetoothdevicediscoveryagent_winrt.moc>

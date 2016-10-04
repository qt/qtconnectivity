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
    hr = QWinRTFunctions::await(deviceFromIdOperation, device.GetAddressOf());
    WARN_AND_RETURN_IF_FAILED("Could not wait for bluetooth device operation to finish", return QBluetoothDeviceInfo());
    if (!device)
        return QBluetoothDeviceInfo();
    UINT64 address;
    HString name;
    ComPtr<IBluetoothClassOfDevice> classOfDevice;
    UINT32 classOfDeviceInt;
    hr = device->get_BluetoothAddress(&address);
    WARN_AND_RETURN_IF_FAILED("Could not obtain device's bluetooth address", return QBluetoothDeviceInfo());
    hr = device->get_Name(name.GetAddressOf());
    WARN_AND_RETURN_IF_FAILED("Could not obtain device's name", return QBluetoothDeviceInfo());
    const QString btName = QString::fromWCharArray(WindowsGetStringRawBuffer(name.Get(), nullptr));
    hr = device->get_ClassOfDevice(&classOfDevice);
    WARN_AND_RETURN_IF_FAILED("Could not obtain device's ckass", return QBluetoothDeviceInfo());
    hr = classOfDevice->get_RawValue(&classOfDeviceInt);
    WARN_AND_RETURN_IF_FAILED("Could not obtain raw device value", return QBluetoothDeviceInfo());
    IVectorView <Rfcomm::RfcommDeviceService *> *deviceServices;
    hr = device->get_RfcommServices(&deviceServices);
    WARN_AND_RETURN_IF_FAILED("Could not obtain bluetooth device services", return QBluetoothDeviceInfo());
    uint serviceCount;
    hr = deviceServices->get_Size(&serviceCount);
    Q_ASSERT_SUCCEEDED(hr);
    QList<QBluetoothUuid> uuids;
    for (uint i = 0; i < serviceCount; ++i) {
        ComPtr<Rfcomm::IRfcommDeviceService> service;
        hr = deviceServices->GetAt(i, &service);
        Q_ASSERT_SUCCEEDED(hr);
        ComPtr<Rfcomm::IRfcommServiceId> id;
        hr = service->get_ServiceId(&id);
        Q_ASSERT_SUCCEEDED(hr);
        GUID uuid;
        hr = id->get_Uuid(&uuid);
        Q_ASSERT_SUCCEEDED(hr);
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
    hr = QWinRTFunctions::await(deviceFromIdOperation, device.GetAddressOf());
    WARN_AND_RETURN_IF_FAILED("Could not wait for bluetooth LE device operation to finish", return QBluetoothDeviceInfo());
    if (!device)
        return QBluetoothDeviceInfo();
    UINT64 address;
    HString name;
    hr = device->get_BluetoothAddress(&address);
    WARN_AND_RETURN_IF_FAILED("Could not obtain device's bluetooth address", return QBluetoothDeviceInfo());
    hr = device->get_Name(name.GetAddressOf());
    WARN_AND_RETURN_IF_FAILED("Could not obtain device's name", return QBluetoothDeviceInfo());
    const QString btName = QString::fromWCharArray(WindowsGetStringRawBuffer(name.Get(), nullptr));
    IVectorView <GenericAttributeProfile::GattDeviceService *> *deviceServices;
    hr = device->get_GattServices(&deviceServices);
    WARN_AND_RETURN_IF_FAILED("Could not obtain bluetooth LE device services", return QBluetoothDeviceInfo());
    uint serviceCount;
    hr = deviceServices->get_Size(&serviceCount);
    Q_ASSERT_SUCCEEDED(hr);
    QList<QBluetoothUuid> uuids;
    for (uint i = 0; i < serviceCount; ++i) {
        ComPtr<GenericAttributeProfile::IGattDeviceService> service;
        hr = deviceServices->GetAt(i, &service);
        Q_ASSERT_SUCCEEDED(hr);
        ComPtr<Rfcomm::IRfcommServiceId> id;
        GUID uuid;
        hr = service->get_Uuid(&uuid);
        Q_ASSERT_SUCCEEDED(hr);
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
    QWinRTBluetoothDeviceDiscoveryWorker() : requestedModes(0), initializedModes(0)
    {
    }

    void start()
    {
        QEventDispatcherWinRT::runOnXamlThread([this]() {
            if (requestedModes & QBluetoothDeviceDiscoveryAgent::ClassicMethod)
                startDeviceDiscovery(QBluetoothDeviceDiscoveryAgent::ClassicMethod);

            if (requestedModes & QBluetoothDeviceDiscoveryAgent::LowEnergyMethod)
                startDeviceDiscovery(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
            return S_OK;
        });

        qCDebug(QT_BT_WINRT) << "Worker started";
    }

    ~QWinRTBluetoothDeviceDiscoveryWorker()
    {
        if (leDeviceWatcher && leDeviceAddedToken.value) {
            HRESULT hr;
            hr = leDeviceWatcher->remove_Added(leDeviceAddedToken);
            Q_ASSERT_SUCCEEDED(hr);
        }
    }

private:
    void startDeviceDiscovery(QBluetoothDeviceDiscoveryAgent::DiscoveryMethod mode)
    {
        HString deviceSelector;
        ComPtr<IDeviceInformationStatics> deviceInformationStatics;
        HRESULT hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Enumeration_DeviceInformation).Get(), &deviceInformationStatics);
        WARN_AND_RETURN_IF_FAILED("Could not obtain device information statics", return);
        if (mode == QBluetoothDeviceDiscoveryAgent::LowEnergyMethod) {
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
        hr = op->put_Completed(
            Callback<IAsyncOperationCompletedHandler<DeviceInformationCollection *>>([this, mode](IAsyncOperation<DeviceInformationCollection *> *op, AsyncStatus) {
                onDeviceDiscoveryFinished(op, mode);
                return S_OK;
            }).Get());
        WARN_AND_RETURN_IF_FAILED("Could not add callback to bluetooth device discovery operation", return);
    }

    void onDeviceDiscoveryFinished(IAsyncOperation<DeviceInformationCollection *> *op, QBluetoothDeviceDiscoveryAgent::DiscoveryMethod mode)
    {
        qCDebug(QT_BT_WINRT) << (mode == QBluetoothDeviceDiscoveryAgent::ClassicMethod ? "BT" : "BTLE")
                             << " scan completed";
        ComPtr<IVectorView<DeviceInformation *>> devices;
        HRESULT hr;
        hr = op->GetResults(&devices);
        Q_ASSERT_SUCCEEDED(hr);
        onDevicesFound(devices.Get(), mode);
        initializedModes |= mode;
        if (initializedModes == requestedModes) {
            qCDebug(QT_BT_WINRT) << "All scans completed";
            emit initializationCompleted();
            setupLEDeviceWatcher();
        }
    }

    void onDeviceAdded(IDeviceInformation *deviceInfo, QBluetoothDeviceDiscoveryAgent::DiscoveryMethod mode)
    {
        HString deviceId;
        HRESULT hr;
        hr = deviceInfo->get_Id(deviceId.GetAddressOf());
        Q_ASSERT_SUCCEEDED(hr);
        const QBluetoothDeviceInfo info = mode == QBluetoothDeviceDiscoveryAgent::LowEnergyMethod
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

    void onDevicesFound(IVectorView<DeviceInformation *> *devices, QBluetoothDeviceDiscoveryAgent::DiscoveryMethod mode)
    {
        quint32 deviceCount;
        HRESULT hr = devices->get_Size(&deviceCount);
        Q_ASSERT_SUCCEEDED(hr);
        deviceList.reserve(deviceList.length() + deviceCount);
        for (quint32 i = 0; i < deviceCount; ++i) {
            ComPtr<IDeviceInformation> device;
            hr = devices->GetAt(i, &device);
            Q_ASSERT_SUCCEEDED(hr);
            onDeviceAdded(device.Get(), mode);
        }
    }

    void setupLEDeviceWatcher()
    {
        HString deviceSelector;
        ComPtr<IDeviceInformationStatics> deviceInformationStatics;
        HRESULT hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Enumeration_DeviceInformation).Get(), &deviceInformationStatics);
        WARN_AND_RETURN_IF_FAILED("Could not obtain device information statics", return);
        ComPtr<IBluetoothLEDeviceStatics> bluetoothLeDeviceStatics;
        hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_BluetoothLEDevice).Get(), &bluetoothLeDeviceStatics);
        WARN_AND_RETURN_IF_FAILED("Could not obtain bluetooth LE device statics", return);
        hr = bluetoothLeDeviceStatics->GetDeviceSelector(deviceSelector.GetAddressOf());
        WARN_AND_RETURN_IF_FAILED("Could not obtain device selector string", return);
        hr = deviceInformationStatics->CreateWatcherAqsFilter(deviceSelector.Get(), &leDeviceWatcher);
        WARN_AND_RETURN_IF_FAILED("Could not create le device watcher", return);
        auto deviceAddedCallback =
            Callback<ITypedEventHandler<DeviceWatcher*, DeviceInformation*>>([this](IDeviceWatcher *, IDeviceInformation *deviceInfo)
        {
            HString deviceId;
            HRESULT hr;
            hr = deviceInfo->get_Id(deviceId.GetAddressOf());
            Q_ASSERT_SUCCEEDED(hr);
            const QBluetoothDeviceInfo info = bluetoothInfoFromLeDeviceId(deviceId.Get());
            if (!info.isValid())
                return S_OK;

            qCDebug(QT_BT_WINRT) << "Found device" << info.name() << info.address();
            emit leDeviceFound(info);
            return S_OK;
        });
        hr = leDeviceWatcher->add_Added(deviceAddedCallback.Get(), &leDeviceAddedToken);
        WARN_AND_RETURN_IF_FAILED("Could not add \"device added\" callback", return);
        hr = leDeviceWatcher->Start();
        WARN_AND_RETURN_IF_FAILED("Could not start device watcher", return);
    }

public slots:
    void onLeTimeout()
    {
        if (initializedModes == requestedModes)
            emit scanFinished();
        else
            emit scanCanceled();
        deleteLater();
    }

Q_SIGNALS:
    void initializationCompleted();
    void leDeviceFound(const QBluetoothDeviceInfo &info);
    void scanFinished();
    void scanCanceled();

public:
    QVector<QBluetoothDeviceInfo> deviceList;
    quint8 requestedModes;

private:
    quint8 initializedModes;
    ComPtr<IDeviceWatcher> leDeviceWatcher;
    EventRegistrationToken leDeviceAddedToken;
};

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(
                const QBluetoothAddress &deviceAdapter,
                QBluetoothDeviceDiscoveryAgent *parent)

    :   inquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry),
        lastError(QBluetoothDeviceDiscoveryAgent::NoError),
        lowEnergySearchTimeout(25000),
        q_ptr(parent),
        leScanTimer(0)
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

QBluetoothDeviceDiscoveryAgent::DiscoveryMethods QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods()
{
    return (ClassicMethod | LowEnergyMethod);
}

void QBluetoothDeviceDiscoveryAgentPrivate::start(QBluetoothDeviceDiscoveryAgent::DiscoveryMethods methods)
{
    if (worker)
        return;

    worker = new QWinRTBluetoothDeviceDiscoveryWorker();
    worker->requestedModes = methods;
    discoveredDevices.clear();
    connect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::initializationCompleted,
        this, &QBluetoothDeviceDiscoveryAgentPrivate::onListInitializationCompleted);
    connect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::leDeviceFound,
            this, &QBluetoothDeviceDiscoveryAgentPrivate::onLeDeviceFound);
    connect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::scanFinished,
            this, &QBluetoothDeviceDiscoveryAgentPrivate::onScanFinished);
    connect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::scanCanceled,
            this, &QBluetoothDeviceDiscoveryAgentPrivate::onScanCanceled);
    worker->start();

    if (lowEnergySearchTimeout > 0 && methods & QBluetoothDeviceDiscoveryAgent::LowEnergyMethod) { // otherwise no timeout and stop() required
        if (!leScanTimer) {
            leScanTimer = new QTimer(this);
            leScanTimer->setSingleShot(true);
        }
        connect(leScanTimer, &QTimer::timeout,
            worker, &QWinRTBluetoothDeviceDiscoveryWorker::onLeTimeout);
        leScanTimer->setInterval(lowEnergySearchTimeout);
        leScanTimer->start();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    if (worker) {
        disconnectAndClearWorker();
        emit q->canceled();
    }
    if (leScanTimer)
        leScanTimer->stop();
}

void QBluetoothDeviceDiscoveryAgentPrivate::onListInitializationCompleted()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    discoveredDevices = worker->deviceList.toList();
    foreach (const QBluetoothDeviceInfo &info, worker->deviceList)
        emit q->deviceDiscovered(info);
}

void QBluetoothDeviceDiscoveryAgentPrivate::onLeDeviceFound(const QBluetoothDeviceInfo &info)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    for (auto discoveredInfo : discoveredDevices)
        if (discoveredInfo.address() == info.address())
            return;

    discoveredDevices << info;
    emit q->deviceDiscovered(info);
}

void QBluetoothDeviceDiscoveryAgentPrivate::onScanFinished()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    disconnectAndClearWorker();
    emit q->finished();
}

void QBluetoothDeviceDiscoveryAgentPrivate::onScanCanceled()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    disconnectAndClearWorker();
    emit q->canceled();
}

void QBluetoothDeviceDiscoveryAgentPrivate::disconnectAndClearWorker()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    if (!worker)
        return;

    disconnect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::initializationCompleted,
        this, &QBluetoothDeviceDiscoveryAgentPrivate::onListInitializationCompleted);
    disconnect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::scanCanceled,
        this, &QBluetoothDeviceDiscoveryAgentPrivate::onScanCanceled);
    disconnect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::scanFinished,
        this, &QBluetoothDeviceDiscoveryAgentPrivate::onScanFinished);
    disconnect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::leDeviceFound,
        q, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered);
    if (leScanTimer) {
        disconnect(leScanTimer, &QTimer::timeout,
            worker, &QWinRTBluetoothDeviceDiscoveryWorker::onLeTimeout);
    }
    worker->deleteLater();
    worker.clear();
}

QT_END_NAMESPACE

#include <qbluetoothdevicediscoveryagent_winrt.moc>

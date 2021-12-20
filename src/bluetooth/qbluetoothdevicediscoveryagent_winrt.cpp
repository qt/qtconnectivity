/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothdevicediscoveryagent_p.h"
#include "qbluetoothaddress.h"
#include "qbluetoothuuid.h"

#include <QtBluetooth/private/qtbluetoothglobal_p.h>
#include <QtBluetooth/private/qbluetoothutils_winrt_p.h>
#include <QtCore/QLoggingCategory>
#include <QtCore/qmutex.h>
#include <QtCore/private/qfunctions_winrt_p.h>

#include <robuffer.h>
#include <wrl.h>
#include <windows.devices.enumeration.h>
#include <windows.devices.bluetooth.h>
#include <windows.foundation.collections.h>
#include <windows.storage.streams.h>

#include <windows.devices.bluetooth.advertisement.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Devices;
using namespace ABI::Windows::Devices::Bluetooth;
using namespace ABI::Windows::Devices::Bluetooth::Advertisement;
using namespace ABI::Windows::Devices::Enumeration;
using namespace ABI::Windows::Storage::Streams;

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

#define EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED(msg, error, ret) \
    if (FAILED(hr)) { \
        emit errorOccured(error); \
        qCWarning(QT_BT_WINDOWS) << msg; \
        ret; \
    }

#define WARN_AND_RETURN_IF_FAILED(msg, ret) \
    if (FAILED(hr)) { \
        qCWarning(QT_BT_WINDOWS) << msg; \
        ret; \
    }

#define WARN_AND_CONTINUE_IF_FAILED(msg) \
    if (FAILED(hr)) { \
        qCWarning(QT_BT_WINDOWS) << msg; \
        continue; \
    }

static ManufacturerData extractManufacturerData(ComPtr<IBluetoothLEAdvertisement> ad)
{
    ManufacturerData ret;
    ComPtr<IVector<BluetoothLEManufacturerData*>> data;
    HRESULT hr = ad->get_ManufacturerData(&data);
    WARN_AND_RETURN_IF_FAILED("Could not obtain list of manufacturer data.", return ret);
    quint32 size;
    hr = data->get_Size(&size);
    WARN_AND_RETURN_IF_FAILED("Could not obtain manufacturer data's list size.", return ret);
    for (quint32 i = 0; i < size; ++i) {
        ComPtr<IBluetoothLEManufacturerData> d;
        hr = data->GetAt(i, &d);
        WARN_AND_CONTINUE_IF_FAILED("Could not obtain manufacturer data.");
        quint16 id;
        hr = d->get_CompanyId(&id);
        WARN_AND_CONTINUE_IF_FAILED("Could not obtain manufacturer data company id.");
        ComPtr<IBuffer> buffer;
        hr = d->get_Data(&buffer);
        WARN_AND_CONTINUE_IF_FAILED("Could not obtain manufacturer data set.");
        const QByteArray bufferData = byteArrayFromBuffer(buffer);
        if (ret.contains(id))
            qCWarning(QT_BT_WINDOWS) << "Company ID already present in manufacturer data.";
        ret.insert(id, bufferData);
    }
    return ret;
}

class QWinRTBluetoothDeviceDiscoveryWorker : public QObject
{
    Q_OBJECT
public:
    explicit QWinRTBluetoothDeviceDiscoveryWorker(QBluetoothDeviceDiscoveryAgent::DiscoveryMethods methods);
    ~QWinRTBluetoothDeviceDiscoveryWorker();
    void start();
    void stopLEWatcher();

private:
    void startDeviceDiscovery(QBluetoothDeviceDiscoveryAgent::DiscoveryMethod mode);
    void onDeviceDiscoveryFinished(IAsyncOperation<DeviceInformationCollection *> *op,
                                   QBluetoothDeviceDiscoveryAgent::DiscoveryMethod mode);
    void gatherDeviceInformation(IDeviceInformation *deviceInfo,
                                 QBluetoothDeviceDiscoveryAgent::DiscoveryMethod mode);
    void gatherMultipleDeviceInformation(quint32 deviceCount, IVectorView<DeviceInformation *> *devices,
                                         QBluetoothDeviceDiscoveryAgent::DiscoveryMethod mode);
    void setupLEDeviceWatcher();
    void classicBluetoothInfoFromDeviceIdAsync(HSTRING deviceId);
    void leBluetoothInfoFromDeviceIdAsync(HSTRING deviceId);
    void leBluetoothInfoFromAddressAsync(quint64 address);
    HRESULT onPairedClassicBluetoothDeviceFoundAsync(IAsyncOperation<BluetoothDevice *> *op, AsyncStatus status );
    HRESULT onPairedBluetoothLEDeviceFoundAsync(IAsyncOperation<BluetoothLEDevice *> *op, AsyncStatus status);
    HRESULT onBluetoothLEDeviceFoundAsync(IAsyncOperation<BluetoothLEDevice *> *op, AsyncStatus status);
    enum PairingCheck {
        CheckForPairing,
        OmitPairingCheck
    };
    HRESULT onBluetoothLEDeviceFound(ComPtr<IBluetoothLEDevice> device, PairingCheck pairingCheck);
    HRESULT onBluetoothLEDeviceFound(ComPtr<IBluetoothLEDevice> device);
    HRESULT onBluetoothLEAdvertisementReceived(IBluetoothLEAdvertisementReceivedEventArgs *args);
    HRESULT onRfcommServicesReceived(IAsyncOperation<Rfcomm::RfcommDeviceServicesResult *> *op,
                                     AsyncStatus status, UINT64 address, UINT32 classOfDeviceInt,
                                     const QString &btName);
    HRESULT onLeServicesReceived(IAsyncOperation<GenericAttributeProfile::GattDeviceServicesResult *> *op,
                                 AsyncStatus status, QBluetoothDeviceInfo &info);

    void decrementPairedDevicesAndCheckFinished();

public slots:
    void finishDiscovery();

Q_SIGNALS:
    void deviceFound(const QBluetoothDeviceInfo &info);
    void deviceDataChanged(const QBluetoothAddress &address, QBluetoothDeviceInfo::Fields,
                           qint16 rssi, ManufacturerData manufacturerData);
    void errorOccured(QBluetoothDeviceDiscoveryAgent::Error error);
    void scanFinished();

public:
    quint8 requestedModes;

private:
    ComPtr<IBluetoothLEAdvertisementWatcher> m_leWatcher;
    EventRegistrationToken m_leDeviceAddedToken;
    QMutex m_foundDevicesMutex;
    struct LEAdvertisingInfo {
        QList<QBluetoothUuid> services;
        ManufacturerData manufacturerData;
        qint16 rssi = 0;
    };

    QMap<quint64, LEAdvertisingInfo> m_foundLEDevicesMap;
    int m_pendingPairedDevices;

    ComPtr<IBluetoothDeviceStatics> m_deviceStatics;
    ComPtr<IBluetoothLEDeviceStatics> m_leDeviceStatics;
};

QWinRTBluetoothDeviceDiscoveryWorker::QWinRTBluetoothDeviceDiscoveryWorker(QBluetoothDeviceDiscoveryAgent::DiscoveryMethods methods)
    : requestedModes(methods)
    , m_pendingPairedDevices(0)
{
    qRegisterMetaType<QBluetoothDeviceInfo>();
    qRegisterMetaType<QBluetoothDeviceInfo::Fields>();
    qRegisterMetaType<ManufacturerData>();

    CoInitialize(NULL);
    HRESULT hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_BluetoothDevice).Get(), &m_deviceStatics);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain bluetooth device factory",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return)
    hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_BluetoothLEDevice).Get(), &m_leDeviceStatics);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain bluetooth le device factory",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return)
}

QWinRTBluetoothDeviceDiscoveryWorker::~QWinRTBluetoothDeviceDiscoveryWorker()
{
    stopLEWatcher();
    CoUninitialize();
}

void QWinRTBluetoothDeviceDiscoveryWorker::start()
{
    if (requestedModes & QBluetoothDeviceDiscoveryAgent::ClassicMethod)
        startDeviceDiscovery(QBluetoothDeviceDiscoveryAgent::ClassicMethod);

    if (requestedModes & QBluetoothDeviceDiscoveryAgent::LowEnergyMethod) {
        startDeviceDiscovery(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
        setupLEDeviceWatcher();
    }

    qCDebug(QT_BT_WINDOWS) << "Worker started";
}

void QWinRTBluetoothDeviceDiscoveryWorker::stopLEWatcher()
{
    if (m_leWatcher) {
        HRESULT hr = m_leWatcher->Stop();
        EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not stop le watcher",
                                               QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                               return)
        if (m_leDeviceAddedToken.value) {
            hr = m_leWatcher->remove_Received(m_leDeviceAddedToken);
            EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could remove le watcher token",
                                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                                   return)
        }
    }
}

void QWinRTBluetoothDeviceDiscoveryWorker::startDeviceDiscovery(QBluetoothDeviceDiscoveryAgent::DiscoveryMethod mode)
{
    HString deviceSelector;
    ComPtr<IDeviceInformationStatics> deviceInformationStatics;
    HRESULT hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Enumeration_DeviceInformation).Get(), &deviceInformationStatics);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain device information statics",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return);
    if (mode == QBluetoothDeviceDiscoveryAgent::LowEnergyMethod)
        m_leDeviceStatics->GetDeviceSelector(deviceSelector.GetAddressOf());
    else
        m_deviceStatics->GetDeviceSelector(deviceSelector.GetAddressOf());
    ComPtr<IAsyncOperation<DeviceInformationCollection *>> op;
    hr = deviceInformationStatics->FindAllAsyncAqsFilter(deviceSelector.Get(), &op);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not start bluetooth device discovery operation",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return);
    QPointer<QWinRTBluetoothDeviceDiscoveryWorker> thisPointer(this);
    hr = op->put_Completed(
        Callback<IAsyncOperationCompletedHandler<DeviceInformationCollection *>>([thisPointer, mode](IAsyncOperation<DeviceInformationCollection *> *op, AsyncStatus status) {
        if (status == Completed && thisPointer)
            thisPointer->onDeviceDiscoveryFinished(op, mode);
        return S_OK;
    }).Get());
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not add device discovery callback",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return);
}

void QWinRTBluetoothDeviceDiscoveryWorker::onDeviceDiscoveryFinished(IAsyncOperation<DeviceInformationCollection *> *op, QBluetoothDeviceDiscoveryAgent::DiscoveryMethod mode)
{
    qCDebug(QT_BT_WINDOWS) << (mode == QBluetoothDeviceDiscoveryAgent::ClassicMethod ? "BT" : "BTLE")
        << "scan completed";
    ComPtr<IVectorView<DeviceInformation *>> devices;
    HRESULT hr;
    hr = op->GetResults(&devices);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain discovery result",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return);
    quint32 deviceCount;
    hr = devices->get_Size(&deviceCount);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain discovery result size",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return);

    // For classic discovery only paired devices will be found. If we only do classic disovery and
    // no device is found, the scan is finished.
    if (requestedModes == QBluetoothDeviceDiscoveryAgent::ClassicMethod &&
        deviceCount == 0) {
        finishDiscovery();
        return;
    }

    m_pendingPairedDevices += deviceCount;
    gatherMultipleDeviceInformation(deviceCount, devices.Get(), mode);
}

void QWinRTBluetoothDeviceDiscoveryWorker::gatherDeviceInformation(IDeviceInformation *deviceInfo, QBluetoothDeviceDiscoveryAgent::DiscoveryMethod mode)
{
    HString deviceId;
    HRESULT hr;
    hr = deviceInfo->get_Id(deviceId.GetAddressOf());
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain device ID",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return);
    if (mode == QBluetoothDeviceDiscoveryAgent::LowEnergyMethod)
        leBluetoothInfoFromDeviceIdAsync(deviceId.Get());
    else
        classicBluetoothInfoFromDeviceIdAsync(deviceId.Get());
}

void QWinRTBluetoothDeviceDiscoveryWorker::gatherMultipleDeviceInformation(quint32 deviceCount, IVectorView<DeviceInformation *> *devices, QBluetoothDeviceDiscoveryAgent::DiscoveryMethod mode)
{
    for (quint32 i = 0; i < deviceCount; ++i) {
        ComPtr<IDeviceInformation> device;
        HRESULT hr;
        hr = devices->GetAt(i, &device);
        EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain device",
                                               QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                               return);
        gatherDeviceInformation(device.Get(), mode);
    }
}

HRESULT QWinRTBluetoothDeviceDiscoveryWorker::onBluetoothLEAdvertisementReceived(IBluetoothLEAdvertisementReceivedEventArgs *args)
{
    quint64 address;
    HRESULT hr;
    hr = args->get_BluetoothAddress(&address);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain bluetooth address",
                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                   return S_OK);
    qint16 rssi;
    hr = args->get_RawSignalStrengthInDBm(&rssi);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain signal strength",
                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                   return S_OK);
    ComPtr<IBluetoothLEAdvertisement> ad;
    hr = args->get_Advertisement(&ad);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could get advertisement",
                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                   return S_OK);
    const ManufacturerData manufacturerData = extractManufacturerData(ad);
    QBluetoothDeviceInfo::Fields changedFields = QBluetoothDeviceInfo::Field::None;
    ComPtr<IVector<GUID>> guids;
    hr = ad->get_ServiceUuids(&guids);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain service uuid list",
                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                   return S_OK);
    quint32 size;
    hr = guids->get_Size(&size);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain service uuid list size",
                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                   return S_OK);
    QList<QBluetoothUuid> serviceUuids;
    for (quint32 i = 0; i < size; ++i) {
        GUID guid;
        hr = guids->GetAt(i, &guid);
        EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain uuid",
                                       QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                       return S_OK);
        QBluetoothUuid uuid(guid);
        serviceUuids.append(uuid);
    }

    { // scope for QMutexLocker
        QMutexLocker locker(&m_foundDevicesMutex);
        // Merge newly found services with list of currently found ones
        if (m_foundLEDevicesMap.contains(address)) {
            const LEAdvertisingInfo adInfo = m_foundLEDevicesMap.value(address);
            QList<QBluetoothUuid> foundServices = adInfo.services;
            if (adInfo.rssi != rssi) {
                m_foundLEDevicesMap[address].rssi = rssi;
                changedFields.setFlag(QBluetoothDeviceInfo::Field::RSSI);
            }
            if (adInfo.manufacturerData != manufacturerData) {
                m_foundLEDevicesMap[address].manufacturerData.insert(manufacturerData);
                if (adInfo.manufacturerData != m_foundLEDevicesMap[address].manufacturerData)
                    changedFields.setFlag(QBluetoothDeviceInfo::Field::ManufacturerData);
            }
            bool newServiceAdded = false;
            for (const QBluetoothUuid &uuid : qAsConst(serviceUuids)) {
                if (!foundServices.contains(uuid)) {
                    foundServices.append(uuid);
                    newServiceAdded = true;
                }
            }
            if (!newServiceAdded) {
                if (!changedFields.testFlag(QBluetoothDeviceInfo::Field::None)) {
                    QMetaObject::invokeMethod(this, "deviceDataChanged", Qt::AutoConnection,
                                              Q_ARG(QBluetoothAddress, QBluetoothAddress(address)),
                                              Q_ARG(QBluetoothDeviceInfo::Fields, changedFields),
                                              Q_ARG(qint16, rssi),
                                              Q_ARG(ManufacturerData, manufacturerData));
                }
                return S_OK;
            }
            m_foundLEDevicesMap[address].services = foundServices;
        } else {
            LEAdvertisingInfo info;
            info.services = std::move(serviceUuids);
            info.manufacturerData = std::move(manufacturerData);
            info.rssi = rssi;
            m_foundLEDevicesMap.insert(address, info);
        }
    }
    leBluetoothInfoFromAddressAsync(address);
    return S_OK;
}

void QWinRTBluetoothDeviceDiscoveryWorker::setupLEDeviceWatcher()
{
    HRESULT hr = RoActivateInstance(HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_Advertisement_BluetoothLEAdvertisementWatcher).Get(), &m_leWatcher);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not create advertisment watcher",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return);
    hr = m_leWatcher->put_ScanningMode(BluetoothLEScanningMode_Active);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not set scanning mode",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return);
    QPointer<QWinRTBluetoothDeviceDiscoveryWorker> thisPointer(this);
    hr = m_leWatcher->add_Received(
                Callback<ITypedEventHandler<BluetoothLEAdvertisementWatcher *, BluetoothLEAdvertisementReceivedEventArgs *>>(
                    [thisPointer](IBluetoothLEAdvertisementWatcher *, IBluetoothLEAdvertisementReceivedEventArgs *args) {
        if (thisPointer)
            return thisPointer->onBluetoothLEAdvertisementReceived(args);

        return S_OK;
    }).Get(), &m_leDeviceAddedToken);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not add device callback",
                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                   return);
    hr = m_leWatcher->Start();
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not start device watcher",
                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                   return);
}

void QWinRTBluetoothDeviceDiscoveryWorker::finishDiscovery()
{
    emit scanFinished();
    stopLEWatcher();
    deleteLater();
}

// "deviceFound" will be emitted at the end of the deviceFromIdOperation callback
void QWinRTBluetoothDeviceDiscoveryWorker::classicBluetoothInfoFromDeviceIdAsync(HSTRING deviceId)
{
    ComPtr<IAsyncOperation<BluetoothDevice *>> deviceFromIdOperation;
    // on Windows 10 FromIdAsync might ask for device permission. We cannot wait here but have to handle that asynchronously
    HRESULT hr = m_deviceStatics->FromIdAsync(deviceId, &deviceFromIdOperation);
    if (FAILED(hr)) {
        emit errorOccured(QBluetoothDeviceDiscoveryAgent::UnknownError);
        decrementPairedDevicesAndCheckFinished();
        qCWarning(QT_BT_WINDOWS) << "Could not obtain bluetooth device from id";
        return;
    }
    QPointer<QWinRTBluetoothDeviceDiscoveryWorker> thisPointer(this);
    hr = deviceFromIdOperation->put_Completed(Callback<IAsyncOperationCompletedHandler<BluetoothDevice *>>
                                              ([thisPointer](IAsyncOperation<BluetoothDevice *> *op, AsyncStatus status)
    {
        if (thisPointer) {
            if (status == Completed)
                thisPointer->onPairedClassicBluetoothDeviceFoundAsync(op, status);
            else
                thisPointer->decrementPairedDevicesAndCheckFinished();
        }
        return S_OK;
    }).Get());
    if (FAILED(hr)) {
        emit errorOccured(QBluetoothDeviceDiscoveryAgent::UnknownError);
        decrementPairedDevicesAndCheckFinished();
        qCWarning(QT_BT_WINDOWS) << "Could not register device found callback";
        return;
    }
}

// "deviceFound" will be emitted at the end of the deviceFromIdOperation callback
void QWinRTBluetoothDeviceDiscoveryWorker::leBluetoothInfoFromDeviceIdAsync(HSTRING deviceId)
{
    // Note: in this method we do not need to call
    // decrementPairedDevicesAndCheckFinished() because we *do* run LE
    // scanning, so the condition in the check will always be false.
    // It's enough to just decrement m_pendingPairedDevices.
    ComPtr<IAsyncOperation<BluetoothLEDevice *>> deviceFromIdOperation;
    // on Windows 10 FromIdAsync might ask for device permission. We cannot wait here but have to handle that asynchronously
    HRESULT hr = m_leDeviceStatics->FromIdAsync(deviceId, &deviceFromIdOperation);
    if (FAILED(hr)) {
        emit errorOccured(QBluetoothDeviceDiscoveryAgent::UnknownError);
        --m_pendingPairedDevices;
        qCWarning(QT_BT_WINDOWS) << "Could not obtain bluetooth device from id";
        return;
    }
    QPointer<QWinRTBluetoothDeviceDiscoveryWorker> thisPointer(this);
    hr = deviceFromIdOperation->put_Completed(Callback<IAsyncOperationCompletedHandler<BluetoothLEDevice *>>
                                              ([thisPointer] (IAsyncOperation<BluetoothLEDevice *> *op, AsyncStatus status)
    {
        if (thisPointer) {
            if (status == Completed)
                thisPointer->onPairedBluetoothLEDeviceFoundAsync(op, status);
            else
                --thisPointer->m_pendingPairedDevices;
        }
        return S_OK;
    }).Get());
    if (FAILED(hr)) {
        emit errorOccured(QBluetoothDeviceDiscoveryAgent::UnknownError);
        --m_pendingPairedDevices;
        qCWarning(QT_BT_WINDOWS) << "Could not register device found callback";
        return;
    }
}

// "deviceFound" will be emitted at the end of the deviceFromAdressOperation callback
void QWinRTBluetoothDeviceDiscoveryWorker::leBluetoothInfoFromAddressAsync(quint64 address)
{
    ComPtr<IAsyncOperation<BluetoothLEDevice *>> deviceFromAddressOperation;
    // on Windows 10 FromBluetoothAddressAsync might ask for device permission. We cannot wait
    // here but have to handle that asynchronously
    HRESULT hr = m_leDeviceStatics->FromBluetoothAddressAsync(address, &deviceFromAddressOperation);
    if (FAILED(hr)) {
        emit errorOccured(QBluetoothDeviceDiscoveryAgent::UnknownError);
        qCWarning(QT_BT_WINDOWS) << "Could not obtain bluetooth device from address";
        return;
    }
    QPointer<QWinRTBluetoothDeviceDiscoveryWorker> thisPointer(this);
    hr = deviceFromAddressOperation->put_Completed(Callback<IAsyncOperationCompletedHandler<BluetoothLEDevice *>>
                                                   ([thisPointer](IAsyncOperation<BluetoothLEDevice *> *op, AsyncStatus status)
    {
        if (status == Completed && thisPointer)
            thisPointer->onBluetoothLEDeviceFoundAsync(op, status);
        return S_OK;
    }).Get());
    if (FAILED(hr)) {
        emit errorOccured(QBluetoothDeviceDiscoveryAgent::UnknownError);
        qCWarning(QT_BT_WINDOWS) << "Could not register device found callback";
        return;
    }
}

HRESULT QWinRTBluetoothDeviceDiscoveryWorker::onPairedClassicBluetoothDeviceFoundAsync(IAsyncOperation<BluetoothDevice *> *op, AsyncStatus status)
{
    HRESULT hr;
    // Need to decrement m_pendingPairedDevices and perform the check if some
    // operation fails. Otherwise it will be done in the callback.
    auto guard = qScopeGuard([this, &hr]() {
        if (FAILED(hr)) {
            qCWarning(QT_BT_WINDOWS) << "Failed to request Rfcomm services";
            decrementPairedDevicesAndCheckFinished();
        }
    });
    Q_UNUSED(guard); // to suppress warning

    if (status != AsyncStatus::Completed)
        return S_OK;

    ComPtr<IBluetoothDevice> device;
    hr = op->GetResults(&device);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain bluetooth device",
                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                   return S_OK);

    if (!device)
        return S_OK;

    UINT64 address;
    HString name;
    ComPtr<IBluetoothClassOfDevice> classOfDevice;
    UINT32 classOfDeviceInt;
    hr = device->get_BluetoothAddress(&address);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain bluetooth address",
                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                   return S_OK);
    hr = device->get_Name(name.GetAddressOf());
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain device name",
                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                   return S_OK);
    const QString btName = QString::fromWCharArray(WindowsGetStringRawBuffer(name.Get(), nullptr));
    hr = device->get_ClassOfDevice(&classOfDevice);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain device class",
                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                   return S_OK);
    hr = classOfDevice->get_RawValue(&classOfDeviceInt);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain raw value of device class",
                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                   return S_OK);

    ComPtr<IBluetoothDevice3> device3;
    hr = device.As(&device3);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain bluetooth device3 interface",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);

    ComPtr<IAsyncOperation<Rfcomm::RfcommDeviceServicesResult *>> deviceServicesOperation;
    hr = device3->GetRfcommServicesAsync(&deviceServicesOperation);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Async Rfcomm services request failed",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);

    QPointer<QWinRTBluetoothDeviceDiscoveryWorker> thisPointer(this);
    hr = deviceServicesOperation->put_Completed(
            Callback<IAsyncOperationCompletedHandler<Rfcomm::RfcommDeviceServicesResult *>>(
                    [thisPointer, address, btName, classOfDeviceInt](
                            IAsyncOperation<Rfcomm::RfcommDeviceServicesResult *> *op,
                            AsyncStatus status) {
                        if (thisPointer) {
                            thisPointer->onRfcommServicesReceived(op, status, address,
                                                                  classOfDeviceInt, btName);
                        }
                        return S_OK;
                    }).Get());
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not add Rfcomm services discovery callback",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);

    return S_OK;
}

HRESULT QWinRTBluetoothDeviceDiscoveryWorker::onRfcommServicesReceived(
        IAsyncOperation<Rfcomm::RfcommDeviceServicesResult *> *op, AsyncStatus status,
        UINT64 address, UINT32 classOfDeviceInt, const QString &btName)
{
    // need to perform the check even if some of the operations fails
    auto guard = qScopeGuard([this]() {
        decrementPairedDevicesAndCheckFinished();
    });
    Q_UNUSED(guard); // to suppress warning

    if (status != Completed)
        return S_OK;

    ComPtr<Rfcomm::IRfcommDeviceServicesResult> servicesResult;
    HRESULT hr = op->GetResults(&servicesResult);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain device services",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);

    BluetoothError error;
    hr = servicesResult->get_Error(&error);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain error code",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);
    if (error != BluetoothError_Success) {
        qCWarning(QT_BT_WINDOWS) << "Obtain device services completed with BluetoothErrot"
                                 << static_cast<int>(error);
    } else {
        IVectorView<Rfcomm::RfcommDeviceService *> *deviceServices;
        hr = servicesResult->get_Services(&deviceServices);
        EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain services list",
                                               QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                               return S_OK);
        uint serviceCount;
        hr = deviceServices->get_Size(&serviceCount);
        EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain service list size",
                                       QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                       return S_OK);
        QList<QBluetoothUuid> uuids;
        for (uint i = 0; i < serviceCount; ++i) {
            ComPtr<Rfcomm::IRfcommDeviceService> service;
            hr = deviceServices->GetAt(i, &service);
            EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain device service",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);
            ComPtr<Rfcomm::IRfcommServiceId> id;
            hr = service->get_ServiceId(&id);
            EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain service id",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);
            GUID uuid;
            hr = id->get_Uuid(&uuid);
            EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain uuid",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);
            uuids.append(QBluetoothUuid(uuid));
        }

        qCDebug(QT_BT_WINDOWS) << "Discovered BT device: " << QString::number(address) << btName
                               << "Num UUIDs" << uuids.count();

        QBluetoothDeviceInfo info(QBluetoothAddress(address), btName, classOfDeviceInt);
        info.setCoreConfigurations(QBluetoothDeviceInfo::BaseRateCoreConfiguration);
        info.setServiceUuids(uuids);
        info.setCached(true);

        QMetaObject::invokeMethod(this, "deviceFound", Qt::AutoConnection,
                                  Q_ARG(QBluetoothDeviceInfo, info));
    }

    return S_OK;
}

void QWinRTBluetoothDeviceDiscoveryWorker::decrementPairedDevicesAndCheckFinished()
{
    if ((--m_pendingPairedDevices == 0)
        && !(requestedModes & QBluetoothDeviceDiscoveryAgent::LowEnergyMethod)) {
        finishDiscovery();
    }
}

HRESULT QWinRTBluetoothDeviceDiscoveryWorker::onPairedBluetoothLEDeviceFoundAsync(IAsyncOperation<BluetoothLEDevice *> *op, AsyncStatus status)
{
    --m_pendingPairedDevices;
    if (status != AsyncStatus::Completed)
        return S_OK;

    ComPtr<IBluetoothLEDevice> device;
    HRESULT hr;
    hr = op->GetResults(&device);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain bluetooth le device",
                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                   return S_OK);
    return onBluetoothLEDeviceFound(device);
}

HRESULT QWinRTBluetoothDeviceDiscoveryWorker::onBluetoothLEDeviceFoundAsync(IAsyncOperation<BluetoothLEDevice *> *op, AsyncStatus status)
{
    if (status != AsyncStatus::Completed)
        return S_OK;

    ComPtr<IBluetoothLEDevice> device;
    HRESULT hr;
    hr = op->GetResults(&device);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain bluetooth le device",
                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                   return S_OK);
    return onBluetoothLEDeviceFound(device);
}

static void invokeDeviceFoundWithDebug(QWinRTBluetoothDeviceDiscoveryWorker *worker,
                                       const QBluetoothDeviceInfo &info)
{
    qCDebug(QT_BT_WINDOWS) << "Discovered BTLE device: " << info.address() << info.name()
                           << "Num UUIDs" << info.serviceUuids().count() << "RSSI:" << info.rssi()
                           << "Num manufacturer data" << info.manufacturerData().count();

    QMetaObject::invokeMethod(worker, "deviceFound", Qt::AutoConnection,
                              Q_ARG(QBluetoothDeviceInfo, info));
}

HRESULT QWinRTBluetoothDeviceDiscoveryWorker::onBluetoothLEDeviceFound(ComPtr<IBluetoothLEDevice> device)
{
    if (!device) {
        qCDebug(QT_BT_WINDOWS) << "onBluetoothLEDeviceFound: No device given";
        return S_OK;
    }

    UINT64 address;
    HString name;
    HRESULT hr = device->get_BluetoothAddress(&address);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain bluetooth address",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);
    hr = device->get_Name(name.GetAddressOf());
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain device name",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);
    const QString btName = QString::fromWCharArray(WindowsGetStringRawBuffer(name.Get(), nullptr));

    ComPtr<IBluetoothLEDevice2> device2;
    hr = device.As(&device2);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not cast device",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);
    ComPtr<IDeviceInformation> deviceInfo;
    hr = device2->get_DeviceInformation(&deviceInfo);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain device info",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);
    if (!deviceInfo) {
        qCDebug(QT_BT_WINDOWS) << "onBluetoothLEDeviceFound: Could not obtain device information";
        return S_OK;
    }
    ComPtr<IDeviceInformation2> deviceInfo2;
    hr = deviceInfo.As(&deviceInfo2);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain cast device info",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);
    ComPtr<IDeviceInformationPairing> pairing;
    hr = deviceInfo2->get_Pairing(&pairing);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain pairing information",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);
    boolean isPaired;
    hr = pairing->get_IsPaired(&isPaired);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain pairing status",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);

    const LEAdvertisingInfo adInfo = m_foundLEDevicesMap.value(address);
    const ManufacturerData manufacturerData = adInfo.manufacturerData;
    const qint16 rssi = adInfo.rssi;

    QBluetoothDeviceInfo info(QBluetoothAddress(address), btName, 0);
    info.setCoreConfigurations(QBluetoothDeviceInfo::LowEnergyCoreConfiguration);
    info.setRssi(rssi);
    for (quint16 key : manufacturerData.keys())
        info.setManufacturerData(key, manufacturerData.value(key));
    info.setCached(true);

    // Use the services obtained from the advertisement data if the device is not paired
    if (!isPaired) {
        info.setServiceUuids(adInfo.services);
        invokeDeviceFoundWithDebug(this, info);
    } else {
        ComPtr<IBluetoothLEDevice3> device3;
        hr = device.As(&device3);
        EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Failed to obtain IBluetoothLEDevice3 instance",
                                               QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                               return S_OK);

        ComPtr<IAsyncOperation<GenericAttributeProfile::GattDeviceServicesResult *>> servicesOp;
        hr = device3->GetGattServicesAsync(&servicesOp);
        EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Failed to execute async services request",
                                               QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                               return S_OK);

        QPointer<QWinRTBluetoothDeviceDiscoveryWorker> thisPtr(this);
        hr = servicesOp->put_Completed(
                Callback<IAsyncOperationCompletedHandler<
                    GenericAttributeProfile::GattDeviceServicesResult *>>([thisPtr, info](
                        IAsyncOperation<GenericAttributeProfile::GattDeviceServicesResult *> *op,
                        AsyncStatus status) mutable {
                            if (thisPtr)
                                thisPtr->onLeServicesReceived(op, status, info);
                            return S_OK;
                        }).Get());
        EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not add LE services discovery callback",
                                               QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                               return S_OK);
    }

    return S_OK;
}

HRESULT QWinRTBluetoothDeviceDiscoveryWorker::onLeServicesReceived(
        IAsyncOperation<GenericAttributeProfile::GattDeviceServicesResult *> *op,
        AsyncStatus status, QBluetoothDeviceInfo &info)
{
    if (status != AsyncStatus::Completed) {
        qCWarning(QT_BT_WINDOWS) << "LE service request finished with status"
                                 << static_cast<int>(status);
        return S_OK;
    }

    ComPtr<GenericAttributeProfile::IGattDeviceServicesResult> servicesResult;
    HRESULT hr = op->GetResults(&servicesResult);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not get async operation result for LE services",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);

    GenericAttributeProfile::GattCommunicationStatus commStatus;
    hr = servicesResult->get_Status(&commStatus);
    EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain services status",
                                           QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                           return S_OK);

    if (commStatus == GenericAttributeProfile::GattCommunicationStatus_Success) {
        IVectorView<GenericAttributeProfile::GattDeviceService *> *deviceServices;
        hr = servicesResult->get_Services(&deviceServices);
        EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain gatt service list",
                                               QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                               return S_OK);
        uint serviceCount;
        hr = deviceServices->get_Size(&serviceCount);
        EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain gatt service list size",
                                               QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                               return S_OK);
        QList<QBluetoothUuid> uuids;
        for (uint i = 0; i < serviceCount; ++i) {
            ComPtr<GenericAttributeProfile::IGattDeviceService> service;
            hr = deviceServices->GetAt(i, &service);
            EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain gatt service",
                                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                                   return S_OK);
            GUID uuid;
            hr = service->get_Uuid(&uuid);
            EMIT_WORKER_ERROR_AND_RETURN_IF_FAILED("Could not obtain uuid",
                                                   QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                                                   return S_OK);
            uuids.append(QBluetoothUuid(uuid));
        }
        info.setServiceUuids(uuids);
    } else {
        qCWarning(QT_BT_WINDOWS) << "Obtaining LE services finished with status"
                                 << static_cast<int>(commStatus);
    }
    invokeDeviceFoundWithDebug(this, info);

    return S_OK;
}

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(
                const QBluetoothAddress &deviceAdapter,
                QBluetoothDeviceDiscoveryAgent *parent)
    :   q_ptr(parent)
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

    worker = new QWinRTBluetoothDeviceDiscoveryWorker(methods);
    discoveredDevices.clear();
    connect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::deviceFound,
            this, &QBluetoothDeviceDiscoveryAgentPrivate::registerDevice);
    connect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::deviceDataChanged,
            this, &QBluetoothDeviceDiscoveryAgentPrivate::updateDeviceData);
    connect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::errorOccured,
            this, &QBluetoothDeviceDiscoveryAgentPrivate::onErrorOccured);
    connect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::scanFinished,
            this, &QBluetoothDeviceDiscoveryAgentPrivate::onScanFinished);
    worker->start();

    if (lowEnergySearchTimeout > 0 && methods & QBluetoothDeviceDiscoveryAgent::LowEnergyMethod) { // otherwise no timeout and stop() required
        if (!leScanTimer) {
            leScanTimer = new QTimer(this);
            leScanTimer->setSingleShot(true);
        }
        connect(leScanTimer, &QTimer::timeout,
            worker, &QWinRTBluetoothDeviceDiscoveryWorker::finishDiscovery);
        leScanTimer->setInterval(lowEnergySearchTimeout);
        leScanTimer->start();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    if (worker) {
        worker->stopLEWatcher();
        disconnectAndClearWorker();
        emit q->canceled();
    }
    if (leScanTimer)
        leScanTimer->stop();
}

void QBluetoothDeviceDiscoveryAgentPrivate::registerDevice(const QBluetoothDeviceInfo &info)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    for (QList<QBluetoothDeviceInfo>::iterator iter = discoveredDevices.begin();
        iter != discoveredDevices.end(); ++iter) {
        if (iter->address() == info.address()) {
            qCDebug(QT_BT_WINDOWS) << "Updating device" << iter->name() << iter->address();
            // merge service uuids
            QList<QBluetoothUuid> uuids = iter->serviceUuids();
            uuids.append(info.serviceUuids());
            const QSet<QBluetoothUuid> uuidSet(uuids.begin(), uuids.end());
            if (iter->serviceUuids().count() != uuidSet.count())
                iter->setServiceUuids(uuidSet.values().toVector());
            if (iter->coreConfigurations() != info.coreConfigurations())
                iter->setCoreConfigurations(QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration);
            return;
        }
    }

    discoveredDevices << info;
    emit q->deviceDiscovered(info);
}

void QBluetoothDeviceDiscoveryAgentPrivate::updateDeviceData(const QBluetoothAddress &address,
                                                             QBluetoothDeviceInfo::Fields fields,
                                                             qint16 rssi,
                                                             ManufacturerData manufacturerData)
{
    if (fields.testFlag(QBluetoothDeviceInfo::Field::None))
        return;

    Q_Q(QBluetoothDeviceDiscoveryAgent);
    for (QList<QBluetoothDeviceInfo>::iterator iter = discoveredDevices.begin();
        iter != discoveredDevices.end(); ++iter) {
        if (iter->address() == address) {
            qCDebug(QT_BT_WINDOWS) << "Updating data for device" << iter->name() << iter->address();
            if (fields.testFlag(QBluetoothDeviceInfo::Field::RSSI))
                iter->setRssi(rssi);
            if (fields.testFlag(QBluetoothDeviceInfo::Field::ManufacturerData))
                for (quint16 key : manufacturerData.keys())
                    iter->setManufacturerData(key, manufacturerData.value(key));
            emit q->deviceUpdated(*iter, fields);
            return;
        }
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::onErrorOccured(QBluetoothDeviceDiscoveryAgent::Error e)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    lastError = e;
    emit q->errorOccurred(e);
}

void QBluetoothDeviceDiscoveryAgentPrivate::onScanFinished()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    disconnectAndClearWorker();
    emit q->finished();
}

void QBluetoothDeviceDiscoveryAgentPrivate::disconnectAndClearWorker()
{
    if (!worker)
        return;

    disconnect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::scanFinished,
               this, &QBluetoothDeviceDiscoveryAgentPrivate::onScanFinished);
    disconnect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::deviceFound,
               this, &QBluetoothDeviceDiscoveryAgentPrivate::registerDevice);
    disconnect(worker, &QWinRTBluetoothDeviceDiscoveryWorker::deviceDataChanged,
               this, &QBluetoothDeviceDiscoveryAgentPrivate::updateDeviceData);
    if (leScanTimer) {
        disconnect(leScanTimer, &QTimer::timeout,
                   worker, &QWinRTBluetoothDeviceDiscoveryWorker::finishDiscovery);
    }
    worker.clear();
}

QT_END_NAMESPACE

#include <qbluetoothdevicediscoveryagent_winrt.moc>

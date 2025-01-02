// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlowenergycontroller_winrt_p.h"
#include "qbluetoothutils_winrt_p.h"

#include <QtBluetooth/qbluetoothlocaldevice.h>
#include <QtBluetooth/QLowEnergyCharacteristicData>
#include <QtBluetooth/QLowEnergyDescriptorData>
#include <QtBluetooth/private/qbluetoothutils_winrt_p.h>
#include <QtBluetooth/QLowEnergyService>

#include <QtCore/QtEndian>
#include <QtCore/QLoggingCategory>
#include <QtCore/private/qfunctions_winrt_p.h>
#include <QtCore/QDeadlineTimer>

#include <functional>
#include <robuffer.h>
#include <windows.devices.enumeration.h>
#include <windows.devices.bluetooth.h>
#include <windows.devices.bluetooth.genericattributeprofile.h>
#include <windows.foundation.collections.h>
#include <windows.foundation.metadata.h>
#include <windows.storage.streams.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Foundation::Metadata;
using namespace ABI::Windows::Devices;
using namespace ABI::Windows::Devices::Bluetooth;
using namespace ABI::Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace ABI::Windows::Devices::Enumeration;
using namespace ABI::Windows::Storage::Streams;

QT_BEGIN_NAMESPACE

typedef ITypedEventHandler<BluetoothLEDevice *, IInspectable *> StatusHandler;
typedef ITypedEventHandler<GattSession *, IInspectable *> MtuHandler;
typedef ITypedEventHandler<GattCharacteristic *, GattValueChangedEventArgs *> ValueChangedHandler;
typedef GattReadClientCharacteristicConfigurationDescriptorResult ClientCharConfigDescriptorResult;
typedef IGattReadClientCharacteristicConfigurationDescriptorResult IClientCharConfigDescriptorResult;

#define EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED(hr) \
    if (FAILED(hr)) { \
        emitErrorAndQuitThread(hr); \
        return; \
    }

#define EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, message) \
    if (FAILED(hr)) { \
        emitErrorAndQuitThread(message); \
        return; \
    }

#define WARN_AND_CONTINUE_IF_FAILED(hr, msg) \
    if (FAILED(hr)) { \
        qCWarning(QT_BT_WINDOWS) << msg; \
        continue; \
    }

#define DEC_CHAR_COUNT_AND_CONTINUE_IF_FAILED(hr, msg) \
    if (FAILED(hr)) { \
        qCWarning(QT_BT_WINDOWS) << msg; \
        --mCharacteristicsCountToBeDiscovered; \
        continue; \
    }

#define CHECK_FOR_DEVICE_CONNECTION_ERROR_IMPL(this, hr, msg, ret) \
    if (FAILED(hr)) { \
        this->handleConnectionError(msg); \
        ret; \
    }

#define CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, msg, ret) \
    CHECK_FOR_DEVICE_CONNECTION_ERROR_IMPL(this, hr, msg, ret)

#define CHECK_HR_AND_SET_SERVICE_ERROR(hr, msg, service, error, ret) \
    if (FAILED(hr)) { \
        qCDebug(QT_BT_WINDOWS) << msg; \
        service->setError(error); \
        ret; \
    }

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)
Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS_SERVICE_THREAD)

static constexpr qint64 kMaxConnectTimeout = 20000; // 20 sec

static QByteArray byteArrayFromGattResult(const ComPtr<IGattReadResult> &gattResult,
                                          bool isWCharString = false)
{
    ComPtr<ABI::Windows::Storage::Streams::IBuffer> buffer;
    HRESULT hr;
    hr = gattResult->get_Value(&buffer);
    if (FAILED(hr) || !buffer) {
        qCWarning(QT_BT_WINDOWS) << "Could not obtain buffer from GattReadResult";
        return QByteArray();
    }
    return byteArrayFromBuffer(buffer, isWCharString);
}

template <typename T>
static void closeDeviceService(ComPtr<T> service)
{
    ComPtr<IClosable> closableService;
    HRESULT hr = service.As(&closableService);
    RETURN_IF_FAILED("Could not cast type to closable", return);
    hr = closableService->Close();
    RETURN_IF_FAILED("Close() call failed", return);
}

class QWinRTLowEnergyServiceHandler : public QObject
{
    Q_OBJECT
public:
    QWinRTLowEnergyServiceHandler(const QBluetoothUuid &service,
                                     const ComPtr<IGattDeviceService3> &deviceService,
                                     QLowEnergyService::DiscoveryMode mode)
        : mService(service), mMode(mode), mDeviceService(deviceService)
    {
        qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    }

    ~QWinRTLowEnergyServiceHandler()
    {
        if (mAbortRequested)
            closeDeviceService(mDeviceService);
        qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    }

public slots:
    void obtainCharList()
    {
        auto exitCondition = [this]() { return mAbortRequested; };

        mIndicateChars.clear();
        qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
        ComPtr<IAsyncOperation<GattCharacteristicsResult *>> characteristicsOp;
        ComPtr<IGattCharacteristicsResult> characteristicsResult;
        HRESULT hr = mDeviceService->GetCharacteristicsAsync(&characteristicsOp);
        EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED(hr);
        hr = QWinRTFunctions::await(characteristicsOp, characteristicsResult.GetAddressOf(),
                                    QWinRTFunctions::ProcessMainThreadEvents, 5000,
                                    exitCondition);
        EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED(hr);
        GattCommunicationStatus status;
        hr = characteristicsResult->get_Status(&status);
        EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED(hr);
        if (status != GattCommunicationStatus_Success) {
            emitErrorAndQuitThread(QLatin1String("Could not obtain char list"));
            return;
        }
        ComPtr<IVectorView<GattCharacteristic *>> characteristics;
        hr = characteristicsResult->get_Characteristics(&characteristics);
        EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED(hr);

        uint characteristicsCount;
        hr = characteristics->get_Size(&characteristicsCount);
        EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED(hr);

        mCharacteristicsCountToBeDiscovered = characteristicsCount;
        for (uint i = 0; !mAbortRequested && (i < characteristicsCount); ++i) {
            ComPtr<IGattCharacteristic> characteristic;
            hr = characteristics->GetAt(i, &characteristic);
            if (FAILED(hr)) {
                qCWarning(QT_BT_WINDOWS) << "Could not obtain characteristic at" << i;
                --mCharacteristicsCountToBeDiscovered;
                continue;
            }

            ComPtr<IGattCharacteristic3> characteristic3;
            hr = characteristic.As(&characteristic3);
            if (FAILED(hr)) {
                qCWarning(QT_BT_WINDOWS) << "Could not cast characteristic";
                --mCharacteristicsCountToBeDiscovered;
                continue;
            }

            // For some strange reason, Windows doesn't discover descriptors of characteristics (if not paired).
            // Qt API assumes that all characteristics and their descriptors are discovered in one go.
            // So we start 'GetDescriptorsAsync' for each discovered characteristic and finish only
            // when GetDescriptorsAsync for all characteristics return.
            ComPtr<IAsyncOperation<GattDescriptorsResult *>> descAsyncOp;
            hr = characteristic3->GetDescriptorsAsync(&descAsyncOp);
            DEC_CHAR_COUNT_AND_CONTINUE_IF_FAILED(hr, "Could not obtain list of descriptors")

            ComPtr<IGattDescriptorsResult> descResult;
            hr = QWinRTFunctions::await(descAsyncOp, descResult.GetAddressOf(),
                                        QWinRTFunctions::ProcessMainThreadEvents, 5000,
                                        exitCondition);
            DEC_CHAR_COUNT_AND_CONTINUE_IF_FAILED(hr, "Could not obtain descriptor read result")

            quint16 handle;
            hr = characteristic->get_AttributeHandle(&handle);
            DEC_CHAR_COUNT_AND_CONTINUE_IF_FAILED(
                    hr, "Could not obtain characteristic's attribute handle")
            QLowEnergyServicePrivate::CharData charData;
            charData.valueHandle = handle + 1;
            if (mStartHandle == 0 || mStartHandle > handle)
                mStartHandle = handle;
            if (mEndHandle == 0 || mEndHandle < handle)
                mEndHandle = handle;
            GUID guuid;
            hr = characteristic->get_Uuid(&guuid);
            DEC_CHAR_COUNT_AND_CONTINUE_IF_FAILED(hr, "Could not obtain characteristic's Uuid")
            charData.uuid = QBluetoothUuid(guuid);
            GattCharacteristicProperties properties;
            hr = characteristic->get_CharacteristicProperties(&properties);
            DEC_CHAR_COUNT_AND_CONTINUE_IF_FAILED(hr,
                                                  "Could not obtain characteristic's properties")
            charData.properties = QLowEnergyCharacteristic::PropertyTypes(properties & 0xff);
            if (charData.properties & QLowEnergyCharacteristic::Read
                && mMode == QLowEnergyService::FullDiscovery) {
                ComPtr<IAsyncOperation<GattReadResult *>> readOp;
                hr = characteristic->ReadValueWithCacheModeAsync(BluetoothCacheMode_Uncached,
                                                                 &readOp);
                DEC_CHAR_COUNT_AND_CONTINUE_IF_FAILED(hr, "Could not read characteristic")
                ComPtr<IGattReadResult> readResult;
                hr = QWinRTFunctions::await(readOp, readResult.GetAddressOf(),
                                            QWinRTFunctions::ProcessMainThreadEvents, 5000,
                                            exitCondition);
                DEC_CHAR_COUNT_AND_CONTINUE_IF_FAILED(hr,
                                                      "Could not obtain characteristic read result")
                if (!readResult)
                    qCWarning(QT_BT_WINDOWS) << "Characteristic read result is null";
                else
                    charData.value = byteArrayFromGattResult(readResult);
            }
            mCharacteristicList.insert(handle, charData);

            ComPtr<IVectorView<GattDescriptor *>> descriptors;

            GattCommunicationStatus commStatus;
            hr = descResult->get_Status(&commStatus);
            if (FAILED(hr) || commStatus != GattCommunicationStatus_Success) {
                qCWarning(QT_BT_WINDOWS) << "Descriptor operation failed";
                --mCharacteristicsCountToBeDiscovered;
                continue;
            }

            hr = descResult->get_Descriptors(&descriptors);
            DEC_CHAR_COUNT_AND_CONTINUE_IF_FAILED(hr, "Could not obtain list of descriptors")

            uint descriptorCount;
            hr = descriptors->get_Size(&descriptorCount);
            DEC_CHAR_COUNT_AND_CONTINUE_IF_FAILED(hr, "Could not obtain list of descriptors' size")
            for (uint j = 0; !mAbortRequested && (j < descriptorCount); ++j) {
                QLowEnergyServicePrivate::DescData descData;
                ComPtr<IGattDescriptor> descriptor;
                hr = descriptors->GetAt(j, &descriptor);
                WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain descriptor")
                quint16 descHandle;
                hr = descriptor->get_AttributeHandle(&descHandle);
                WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain descriptor's attribute handle")
                GUID descriptorUuid;
                hr = descriptor->get_Uuid(&descriptorUuid);
                WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain descriptor's Uuid")
                descData.uuid = QBluetoothUuid(descriptorUuid);
                charData.descriptorList.insert(descHandle, descData);
                if (descData.uuid == QBluetoothUuid(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration)) {
                    if (mMode == QLowEnergyService::FullDiscovery) {
                        ComPtr<IAsyncOperation<ClientCharConfigDescriptorResult *>> readOp;
                        hr = characteristic->ReadClientCharacteristicConfigurationDescriptorAsync(
                                &readOp);
                        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not read descriptor value")
                        ComPtr<IClientCharConfigDescriptorResult> readResult;
                        hr = QWinRTFunctions::await(readOp, readResult.GetAddressOf(),
                                                    QWinRTFunctions::ProcessMainThreadEvents, 5000,
                                                    exitCondition);
                        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not await descriptor read result")
                        GattClientCharacteristicConfigurationDescriptorValue value;
                        hr = readResult->get_ClientCharacteristicConfigurationDescriptor(&value);
                        WARN_AND_CONTINUE_IF_FAILED(hr,
                                                    "Could not get descriptor value from result")
                        quint16 result = 0;
                        bool correct = false;
                        if (value & GattClientCharacteristicConfigurationDescriptorValue_Indicate) {
                            result |= GattClientCharacteristicConfigurationDescriptorValue_Indicate;
                            correct = true;
                        }
                        if (value & GattClientCharacteristicConfigurationDescriptorValue_Notify) {
                            result |= GattClientCharacteristicConfigurationDescriptorValue_Notify;
                            correct = true;
                        }
                        if (value == GattClientCharacteristicConfigurationDescriptorValue_None)
                            correct = true;
                        if (!correct)
                            continue;

                        descData.value = QByteArray(2, Qt::Uninitialized);
                        qToLittleEndian(result, descData.value.data());
                    }
                    mIndicateChars << charData.uuid;
                } else {
                    if (mMode == QLowEnergyService::FullDiscovery) {
                        ComPtr<IAsyncOperation<GattReadResult *>> readOp;
                        hr = descriptor->ReadValueWithCacheModeAsync(BluetoothCacheMode_Uncached,
                                                                     &readOp);
                        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not read descriptor value")
                        ComPtr<IGattReadResult> readResult;
                        hr = QWinRTFunctions::await(readOp, readResult.GetAddressOf(),
                                                    QWinRTFunctions::ProcessMainThreadEvents, 5000,
                                                    exitCondition);
                        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not await descriptor read result")
                        if (descData.uuid == QBluetoothUuid::DescriptorType::CharacteristicUserDescription)
                            descData.value = byteArrayFromGattResult(readResult, true);
                        else
                            descData.value = byteArrayFromGattResult(readResult);
                    }
                }
                charData.descriptorList.insert(descHandle, descData);
            }

            mCharacteristicList.insert(handle, charData);
            --mCharacteristicsCountToBeDiscovered;
        }
        checkAllCharacteristicsDiscovered();
    }

    void setAbortRequested()
    {
        mAbortRequested = true;
    }

private:
    void checkAllCharacteristicsDiscovered();
    void emitErrorAndQuitThread(HRESULT hr);
    void emitErrorAndQuitThread(const QString &error);

public:
    QBluetoothUuid mService;
    QLowEnergyService::DiscoveryMode mMode;
    ComPtr<IGattDeviceService3> mDeviceService;
    QHash<QLowEnergyHandle, QLowEnergyServicePrivate::CharData> mCharacteristicList;
    uint mCharacteristicsCountToBeDiscovered;
    quint16 mStartHandle = 0;
    quint16 mEndHandle = 0;
    QList<QBluetoothUuid> mIndicateChars;
    bool mAbortRequested = false;

signals:
    void charListObtained(const QBluetoothUuid &service,
                          QHash<QLowEnergyHandle, QLowEnergyServicePrivate::CharData> charList,
                          QList<QBluetoothUuid> indicateChars, QLowEnergyHandle startHandle,
                          QLowEnergyHandle endHandle);
    void errorOccured(const QString &error);
};

void QWinRTLowEnergyServiceHandler::checkAllCharacteristicsDiscovered()
{
    if (!mAbortRequested && (mCharacteristicsCountToBeDiscovered == 0)) {
        emit charListObtained(mService, mCharacteristicList, mIndicateChars,
                              mStartHandle, mEndHandle);
    }
    QThread::currentThread()->quit();
}

void QWinRTLowEnergyServiceHandler::emitErrorAndQuitThread(HRESULT hr)
{
    emitErrorAndQuitThread(qt_error_string(hr));
}

void QWinRTLowEnergyServiceHandler::emitErrorAndQuitThread(const QString &error)
{
    mAbortRequested = true; // so that the service is closed during cleanup
    emit errorOccured(error);
    QThread::currentThread()->quit();
}

class QWinRTLowEnergyConnectionHandler : public QObject
{
    Q_OBJECT
public:
    explicit QWinRTLowEnergyConnectionHandler(const QBluetoothAddress &address) : mAddress(address)
    {
        qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
        // This should be checked before the handler is created
        Q_ASSERT(!mAddress.isNull());
    }
    ~QWinRTLowEnergyConnectionHandler()
    {
        qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
        mDevice.Reset();
        mGattSession.Reset();
        // To close the COM library gracefully, each successful call to
        // CoInitialize, including those that return S_FALSE, must be balanced
        // by a corresponding call to CoUninitialize.
        if (mInitialized == S_OK || mInitialized == S_FALSE)
            CoUninitialize();
    }

public slots:
    void connectToDevice();
    void handleDeviceDisconnectRequest();

signals:
    void deviceConnected(ComPtr<IBluetoothLEDevice> device, ComPtr<IGattSession> session);
    void errorOccurred(const QString &error);

private:
    void connectToPairedDevice();
    void connectToUnpairedDevice();
    void emitErrorAndQuitThread(const QString &error);
    void emitErrorAndQuitThread(const char *error);
    void emitConnectedAndQuitThread();

    ComPtr<IBluetoothLEDevice> mDevice = nullptr;
    ComPtr<IGattSession> mGattSession = nullptr;
    const QBluetoothAddress mAddress;
    bool mAbortConnection = false;
    HRESULT mInitialized = E_UNEXPECTED; // some error code
};

void QWinRTLowEnergyConnectionHandler::connectToDevice()
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    mInitialized = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    qCDebug(QT_BT_WINDOWS) << qt_error_string(mInitialized);

    auto earlyExit = [this]() { return mAbortConnection; };
    ComPtr<IBluetoothLEDeviceStatics> deviceStatics;
    HRESULT hr = GetActivationFactory(
            HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_BluetoothLEDevice).Get(),
            &deviceStatics);
    EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not obtain device factory");
    ComPtr<IAsyncOperation<BluetoothLEDevice *>> deviceFromIdOperation;
    hr = deviceStatics->FromBluetoothAddressAsync(mAddress.toUInt64(), &deviceFromIdOperation);
    EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not find LE device from address");
    hr = QWinRTFunctions::await(deviceFromIdOperation, mDevice.GetAddressOf(),
                                QWinRTFunctions::ProcessMainThreadEvents, 5000, earlyExit);
    if (FAILED(hr) || !mDevice) {
        emitErrorAndQuitThread("Could not find LE device");
        return;
    }

    // get GattSession: 1. get device id
    ComPtr<IBluetoothLEDevice4> device4;
    hr = mDevice.As(&device4);
    EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not cast device");

    ComPtr<IBluetoothDeviceId> deviceId;
    hr = device4->get_BluetoothDeviceId(&deviceId);
    EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not get bluetooth device id");

    // get GattSession: 2. get session statics
    ComPtr<IGattSessionStatics> sessionStatics;
    hr = GetActivationFactory(
            HString::MakeReference(
                    RuntimeClass_Windows_Devices_Bluetooth_GenericAttributeProfile_GattSession)
                    .Get(),
            &sessionStatics);
    EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not obtain GattSession statics");

    // get GattSession: 3. get session
    ComPtr<IAsyncOperation<GattSession *>> gattSessionFromIdOperation;
    hr = sessionStatics->FromDeviceIdAsync(deviceId.Get(), &gattSessionFromIdOperation);
    EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not get GattSession from id");
    hr = QWinRTFunctions::await(gattSessionFromIdOperation, mGattSession.GetAddressOf(),
                                QWinRTFunctions::ProcessMainThreadEvents, 5000, earlyExit);
    EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not complete Gatt session acquire");

    BluetoothConnectionStatus status;
    hr = mDevice->get_ConnectionStatus(&status);
    EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not obtain device's connection status");
    if (status == BluetoothConnectionStatus::BluetoothConnectionStatus_Connected) {
        emitConnectedAndQuitThread();
        return;
    }

    QBluetoothLocalDevice localDevice;
    QBluetoothLocalDevice::Pairing pairing = localDevice.pairingStatus(mAddress);
    if (pairing == QBluetoothLocalDevice::Unpaired)
        connectToUnpairedDevice();
    else
        connectToPairedDevice();
}

void QWinRTLowEnergyConnectionHandler::handleDeviceDisconnectRequest()
{
    mAbortConnection = true;
    // Disconnect from the QLowEnergyControllerPrivateWinRT, so that it does
    // not get notifications. It's absolutely fine to keep doing smth in
    // background, as multiple connections to the same device should be handled
    // correctly by OS.
    disconnect(this, &QWinRTLowEnergyConnectionHandler::deviceConnected, nullptr, nullptr);
    disconnect(this, &QWinRTLowEnergyConnectionHandler::errorOccurred, nullptr, nullptr);
}

void QWinRTLowEnergyConnectionHandler::connectToPairedDevice()
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    ComPtr<IBluetoothLEDevice3> device3;
    HRESULT hr = mDevice.As(&device3);
    EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not cast device");
    ComPtr<IAsyncOperation<GattDeviceServicesResult *>> deviceServicesOp;
    auto earlyExit = [this]() { return mAbortConnection; };
    QDeadlineTimer deadline(kMaxConnectTimeout);
    while (!mAbortConnection && !deadline.hasExpired()) {
        hr = device3->GetGattServicesAsync(&deviceServicesOp);
        EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not obtain services");
        ComPtr<IGattDeviceServicesResult> deviceServicesResult;
        hr = QWinRTFunctions::await(deviceServicesOp, deviceServicesResult.GetAddressOf(),
                                    QWinRTFunctions::ProcessThreadEvents, 5000, earlyExit);
        EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not await services operation");

        GattCommunicationStatus commStatus;
        hr = deviceServicesResult->get_Status(&commStatus);
        if (FAILED(hr) || commStatus != GattCommunicationStatus_Success) {
            emitErrorAndQuitThread("Service operation failed");
            return;
        }

        ComPtr<IVectorView<GattDeviceService *>> deviceServices;
        hr = deviceServicesResult->get_Services(&deviceServices);
        EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not obtain list of services");
        uint serviceCount;
        hr = deviceServices->get_Size(&serviceCount);
        EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not obtain service count");

        if (serviceCount == 0) {
            emitErrorAndQuitThread("Found devices without services");
            return;
        }

        // Windows automatically connects to the device as soon as a service value is read/written.
        // Thus we read one value in order to establish the connection.
        for (uint i = 0; i < serviceCount; ++i) {
            ComPtr<IGattDeviceService> service;
            hr = deviceServices->GetAt(i, &service);
            EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not obtain service");
            ComPtr<IGattDeviceService3> service3;
            hr = service.As(&service3);
            EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not cast service");
            ComPtr<IAsyncOperation<GattCharacteristicsResult *>> characteristicsOp;
            hr = service3->GetCharacteristicsAsync(&characteristicsOp);
            EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not obtain characteristic");
            ComPtr<IGattCharacteristicsResult> characteristicsResult;
            hr = QWinRTFunctions::await(characteristicsOp, characteristicsResult.GetAddressOf(),
                                        QWinRTFunctions::ProcessThreadEvents, 5000, earlyExit);
            EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not await characteristic operation");
            GattCommunicationStatus commStatus;
            hr = characteristicsResult->get_Status(&commStatus);
            if (FAILED(hr) || commStatus != GattCommunicationStatus_Success) {
                qCWarning(QT_BT_WINDOWS) << "Characteristic operation failed";
                break;
            }
            ComPtr<IVectorView<GattCharacteristic *>> characteristics;
            hr = characteristicsResult->get_Characteristics(&characteristics);
            if (hr == E_ACCESSDENIED) {
                // Everything will work as expected up until this point if the
                // manifest capabilties for bluetooth LE are not set.
                emitErrorAndQuitThread("Could not obtain characteristic list. "
                                       "Please check your manifest capabilities");
                return;
            }
            EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not obtain characteristic list");
            uint characteristicsCount;
            hr = characteristics->get_Size(&characteristicsCount);
            EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr,
                                                   "Could not obtain characteristic list's size");
            for (uint j = 0; j < characteristicsCount; ++j) {
                ComPtr<IGattCharacteristic> characteristic;
                hr = characteristics->GetAt(j, &characteristic);
                EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not obtain characteristic");
                ComPtr<IAsyncOperation<GattReadResult *>> op;
                GattCharacteristicProperties props;
                hr = characteristic->get_CharacteristicProperties(&props);
                EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(
                        hr, "Could not obtain characteristic's properties");
                if (!(props & GattCharacteristicProperties_Read))
                    continue;
                hr = characteristic->ReadValueWithCacheModeAsync(
                        BluetoothCacheMode::BluetoothCacheMode_Uncached, &op);
                EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not read characteristic value");
                ComPtr<IGattReadResult> result;
                // Reading characteristics can take surprisingly long at the
                // first time, so we need to have a large the timeout here.
                hr = QWinRTFunctions::await(op, result.GetAddressOf(),
                                            QWinRTFunctions::ProcessThreadEvents, 5000, earlyExit);
                // E_ILLEGAL_METHOD_CALL will be the result for a device, that is not reachable at
                // the moment. In this case we should jump back into the outer loop and keep trying.
                if (hr == E_ILLEGAL_METHOD_CALL)
                    break;
                EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not await characteristic read");
                ComPtr<ABI::Windows::Storage::Streams::IBuffer> buffer;
                hr = result->get_Value(&buffer);
                EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not obtain characteristic value");
                if (!buffer) {
                    qCDebug(QT_BT_WINDOWS) << "Problem reading value";
                    break;
                }

                emitConnectedAndQuitThread();
                return;
            }
        }
    }
    // If we got here because of mAbortConnection == true, the error message
    // will not be delivered, so it does not matter. But we need to terminate
    // the thread anyway!
    emitErrorAndQuitThread("Connect to device failed due to timeout!");
}

void QWinRTLowEnergyConnectionHandler::connectToUnpairedDevice()
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    ComPtr<IBluetoothLEDevice3> device3;
    HRESULT hr = mDevice.As(&device3);
    EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not cast device");
    ComPtr<IGattDeviceServicesResult> deviceServicesResult;
    auto earlyExit = [this]() { return mAbortConnection; };
    QDeadlineTimer deadline(kMaxConnectTimeout);
    while (!mAbortConnection && !deadline.hasExpired()) {
        ComPtr<IAsyncOperation<GattDeviceServicesResult *>> deviceServicesOp;
        hr = device3->GetGattServicesAsync(&deviceServicesOp);
        EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not obtain services");
        hr = QWinRTFunctions::await(deviceServicesOp, deviceServicesResult.GetAddressOf(),
                                    QWinRTFunctions::ProcessMainThreadEvents, 0, earlyExit);
        EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED_2(hr, "Could not await services operation");

        GattCommunicationStatus commStatus;
        hr = deviceServicesResult->get_Status(&commStatus);
        if (commStatus == GattCommunicationStatus_Unreachable)
            continue;

        if (FAILED(hr) || commStatus != GattCommunicationStatus_Success) {
            emitErrorAndQuitThread("Service operation failed");
            return;
        }

        emitConnectedAndQuitThread();
        return;
    }
    // If we got here because of mAbortConnection == true, the error message
    // will not be delivered, so it does not matter. But we need to terminate
    // the thread anyway!
    emitErrorAndQuitThread("Connect to device failed due to timeout!");
}

void QWinRTLowEnergyConnectionHandler::emitErrorAndQuitThread(const QString &error)
{
    emit errorOccurred(error);
    QThread::currentThread()->quit();
}

void QWinRTLowEnergyConnectionHandler::emitErrorAndQuitThread(const char *error)
{
    emitErrorAndQuitThread(QString::fromUtf8(error));
}

void QWinRTLowEnergyConnectionHandler::emitConnectedAndQuitThread()
{
    emit deviceConnected(mDevice, mGattSession);
    QThread::currentThread()->quit();
}

QLowEnergyControllerPrivateWinRT::QLowEnergyControllerPrivateWinRT()
    : QLowEnergyControllerPrivate()
{
    registerQLowEnergyControllerMetaType();
    connect(this, &QLowEnergyControllerPrivateWinRT::characteristicChanged,
            this, &QLowEnergyControllerPrivateWinRT::handleCharacteristicChanged,
            Qt::QueuedConnection);
}

QLowEnergyControllerPrivateWinRT::~QLowEnergyControllerPrivateWinRT()
{
    unregisterFromStatusChanges();
    unregisterFromValueChanges();
}

void QLowEnergyControllerPrivateWinRT::init()
{
}

void QLowEnergyControllerPrivateWinRT::connectToDevice()
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    if (remoteDevice.isNull()) {
        qCWarning(QT_BT_WINDOWS) << "Invalid/null remote device address";
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        return;
    }
    setState(QLowEnergyController::ConnectingState);

    QWinRTLowEnergyConnectionHandler *worker = new QWinRTLowEnergyConnectionHandler(remoteDevice);
    QThread *thread = new QThread;
    worker->moveToThread(thread);
    connect(this, &QLowEnergyControllerPrivateWinRT::abortConnection, worker,
            &QWinRTLowEnergyConnectionHandler::handleDeviceDisconnectRequest);
    connect(thread, &QThread::started, worker, &QWinRTLowEnergyConnectionHandler::connectToDevice);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QObject::destroyed, thread, &QObject::deleteLater);
    connect(worker, &QWinRTLowEnergyConnectionHandler::errorOccurred, this,
            [this](const QString &msg) { handleConnectionError(msg.toUtf8().constData()); });
    connect(worker, &QWinRTLowEnergyConnectionHandler::deviceConnected, this,
            [this](ComPtr<IBluetoothLEDevice> device, ComPtr<IGattSession> session) {
                if (!device || !session) {
                    handleConnectionError("Failed to get device or gatt service");
                    return;
                }
                mDevice = device;
                mGattSession = session;

                if (!registerForStatusChanges() || !registerForMtuChanges()) {
                    handleConnectionError("Failed to register for changes");
                    return;
                }

                Q_Q(QLowEnergyController);
                setState(QLowEnergyController::ConnectedState);
                emit q->connected();
            });
    thread->start();
}

void QLowEnergyControllerPrivateWinRT::disconnectFromDevice()
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    Q_Q(QLowEnergyController);
    setState(QLowEnergyController::ClosingState);
    emit abortConnection();
    unregisterFromValueChanges();
    unregisterFromStatusChanges();
    unregisterFromMtuChanges();
    clearAllServices();
    mGattSession = nullptr;
    mDevice = nullptr;
    setState(QLowEnergyController::UnconnectedState);
    emit q->disconnected();
}

HRESULT QLowEnergyControllerPrivateWinRT::getNativeService(const QBluetoothUuid &serviceUuid,
                                                           NativeServiceCallback callback)
{
    if (m_openedServices.contains(serviceUuid)) {
        callback(m_openedServices.value(serviceUuid));
        return S_OK;
    }

    ComPtr<IBluetoothLEDevice3> device3;
    HRESULT hr = mDevice.As(&device3);
    RETURN_IF_FAILED("Could not convert to IBluetoothDevice3", return hr);

    ComPtr<IAsyncOperation<GattDeviceServicesResult *>> servicesResultOperation;
    hr = device3->GetGattServicesForUuidAsync(serviceUuid, &servicesResultOperation);
    RETURN_IF_FAILED("Could not start async services request", return hr);

    QPointer<QLowEnergyControllerPrivateWinRT> thisPtr(this);
    hr = servicesResultOperation->put_Completed(
            Callback<IAsyncOperationCompletedHandler<
                GenericAttributeProfile::GattDeviceServicesResult *>>(
                    [thisPtr, callback, &serviceUuid](
                        IAsyncOperation<GattDeviceServicesResult *> *op, AsyncStatus status)
    {
        if (thisPtr) {
            if (status != AsyncStatus::Completed) {
                qCDebug(QT_BT_WINDOWS) << "Failed to get result of async service request";
                return S_OK;
            }
            ComPtr<IGattDeviceServicesResult> result;
            ComPtr<IVectorView<GattDeviceService *>> deviceServices;
            HRESULT hr = op->GetResults(&result);
            RETURN_IF_FAILED("Failed to get result of async service request", return hr);

            ComPtr<IVectorView<GattDeviceService *>> services;
            hr = result->get_Services(&services);
            RETURN_IF_FAILED("Failed to extract services from the result", return hr);

            uint servicesCount = 0;
            hr = services->get_Size(&servicesCount);
            RETURN_IF_FAILED("Failed to extract services count", return hr);

            if (servicesCount > 0) {
                if (servicesCount > 1) {
                    qWarning() << "getNativeService: more than one service detected for UUID"
                               << serviceUuid << "The first service will be used.";
                }
                ComPtr<IGattDeviceService> service;
                hr = services->GetAt(0, &service);
                if (FAILED(hr)) {
                    qCDebug(QT_BT_WINDOWS) << "Could not obtain native service for Uuid"
                                           << serviceUuid;
                }
                if (service) {
                    thisPtr->m_openedServices[serviceUuid] = service;
                    callback(service); // Use the service in a custom callback
                }
            } else {
                qCWarning(QT_BT_WINDOWS) << "No services found for Uuid" << serviceUuid;
            }
        } else {
            qCWarning(QT_BT_WINDOWS) << "LE controller was removed while getting native service";
        }
        return S_OK;
    }).Get());

    return hr;
}

HRESULT QLowEnergyControllerPrivateWinRT::getNativeCharacteristic(
        const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid,
        NativeCharacteristicCallback callback)
{
    QPointer<QLowEnergyControllerPrivateWinRT> thisPtr(this);
    auto serviceCallback = [thisPtr, callback, charUuid](ComPtr<IGattDeviceService> service) {
        ComPtr<IGattDeviceService3> service3;
        HRESULT hr = service.As(&service3);
        RETURN_IF_FAILED("Could not cast service to service3", return);

        ComPtr<IAsyncOperation<GattCharacteristicsResult *>> characteristicRequestOp;
        hr = service3->GetCharacteristicsForUuidAsync(charUuid, &characteristicRequestOp);

        hr = characteristicRequestOp->put_Completed(
                    Callback<IAsyncOperationCompletedHandler<GattCharacteristicsResult *>>(
                        [thisPtr, callback](
                            IAsyncOperation<GattCharacteristicsResult *> *op, AsyncStatus status)
        {
            if (thisPtr) {
                if (status != AsyncStatus::Completed) {
                    qCDebug(QT_BT_WINDOWS) << "Failed to get result of async characteristic "
                                              "operation";
                    return S_OK;
                }
                ComPtr<IGattCharacteristicsResult> result;
                HRESULT hr = op->GetResults(&result);
                RETURN_IF_FAILED("Failed to get result of async characteristic operation",
                                 return hr);
                GattCommunicationStatus status;
                hr = result->get_Status(&status);
                if (FAILED(hr) || status != GattCommunicationStatus_Success) {
                    qErrnoWarning(hr, "Native characteristic operation failed.");
                    return S_OK;
                }
                ComPtr<IVectorView<GattCharacteristic *>> characteristics;
                hr = result->get_Characteristics(&characteristics);
                RETURN_IF_FAILED("Could not obtain characteristic list.", return S_OK);
                uint size;
                hr = characteristics->get_Size(&size);
                RETURN_IF_FAILED("Could not obtain characteristic list's size.", return S_OK);
                if (size != 1)
                    qErrnoWarning("More than 1 characteristic found.");
                ComPtr<IGattCharacteristic> characteristic;
                hr = characteristics->GetAt(0, &characteristic);
                RETURN_IF_FAILED("Could not obtain first characteristic for service", return S_OK);
                if (characteristic)
                    callback(characteristic); // use the characteristic in a custom callback
            }
            return S_OK;
        }).Get());
    };

    HRESULT hr = getNativeService(serviceUuid, serviceCallback);
    if (FAILED(hr))
        qCDebug(QT_BT_WINDOWS) << "Failed to get native service for" << serviceUuid;

    return hr;
}

void QLowEnergyControllerPrivateWinRT::registerForValueChanges(const QBluetoothUuid &serviceUuid,
                                                                  const QBluetoothUuid &charUuid)
{
    qCDebug(QT_BT_WINDOWS) << "Registering characteristic" << charUuid << "in service"
                         << serviceUuid << "for value changes";
    for (const ValueChangedEntry &entry : std::as_const(mValueChangedTokens)) {
        GUID guuid;
        HRESULT hr;
        hr = entry.characteristic->get_Uuid(&guuid);
        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain characteristic's Uuid")
        if (QBluetoothUuid(guuid) == charUuid)
            return;
    }

    auto callback = [this, charUuid, serviceUuid](ComPtr<IGattCharacteristic> characteristic) {
        EventRegistrationToken token;
        HRESULT hr;
        hr = characteristic->add_ValueChanged(
                    Callback<ValueChangedHandler>(
                        this, &QLowEnergyControllerPrivateWinRT::onValueChange).Get(),
                    &token);
        RETURN_IF_FAILED("Could not register characteristic for value changes", return)
        mValueChangedTokens.append(ValueChangedEntry(characteristic, token));
        qCDebug(QT_BT_WINDOWS) << "Characteristic" << charUuid << "in service"
                               << serviceUuid << "registered for value changes";
    };

    HRESULT hr = getNativeCharacteristic(serviceUuid, charUuid, callback);
    if (FAILED(hr)) {
        qCDebug(QT_BT_WINDOWS).nospace() << "Could not obtain native characteristic "
                                         << charUuid << " from service " << serviceUuid
                                         << ". Qt will not be able to signal"
                                         << " changes for this characteristic.";
    }
}

void QLowEnergyControllerPrivateWinRT::unregisterFromValueChanges()
{
    qCDebug(QT_BT_WINDOWS) << "Unregistering " << mValueChangedTokens.size() << " value change tokens";
    HRESULT hr;
    for (const ValueChangedEntry &entry : std::as_const(mValueChangedTokens)) {
        if (!entry.characteristic) {
            qCWarning(QT_BT_WINDOWS) << "Unregistering from value changes for characteristic failed."
                                   << "Characteristic has been deleted";
            continue;
        }
        hr = entry.characteristic->remove_ValueChanged(entry.token);
        if (FAILED(hr))
            qCWarning(QT_BT_WINDOWS) << "Unregistering from value changes for characteristic failed.";
    }
    mValueChangedTokens.clear();
}

HRESULT QLowEnergyControllerPrivateWinRT::onValueChange(IGattCharacteristic *characteristic, IGattValueChangedEventArgs *args)
{
    HRESULT hr;
    quint16 handle;
    hr = characteristic->get_AttributeHandle(&handle);
    RETURN_IF_FAILED("Could not obtain characteristic's handle", return S_OK)
    ComPtr<IBuffer> buffer;
    hr = args->get_CharacteristicValue(&buffer);
    RETURN_IF_FAILED("Could not obtain characteristic's value", return S_OK)
    emit characteristicChanged(handle, byteArrayFromBuffer(buffer));
    return S_OK;
}
HRESULT QLowEnergyControllerPrivateWinRT::onMtuChange(IGattSession *session, IInspectable *args)
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    if (session != mGattSession.Get()) {
        qCWarning(QT_BT_WINDOWS) << "Got MTU changed event for wrong or outdated GattSession.";
        return S_OK;
    }

    Q_Q(QLowEnergyController);
    emit q->mtuChanged(mtu());
    return S_OK;
}

bool QLowEnergyControllerPrivateWinRT::registerForStatusChanges()
{
    if (!mDevice)
        return false;

    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;

    HRESULT hr;
    hr = mDevice->add_ConnectionStatusChanged(
        Callback<StatusHandler>(this, &QLowEnergyControllerPrivateWinRT::onStatusChange).Get(),
                                &mStatusChangedToken);
    RETURN_IF_FAILED("Could not add status callback", return false)
    return true;
}

void QLowEnergyControllerPrivateWinRT::unregisterFromStatusChanges()
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    if (mDevice && mStatusChangedToken.value) {
        mDevice->remove_ConnectionStatusChanged(mStatusChangedToken);
        mStatusChangedToken.value = 0;
    }
}

bool QLowEnergyControllerPrivateWinRT::registerForMtuChanges()
{
    if (!mDevice || !mGattSession)
        return false;

    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;

    HRESULT hr;
    hr = mGattSession->add_MaxPduSizeChanged(
            Callback<MtuHandler>(this, &QLowEnergyControllerPrivateWinRT::onMtuChange).Get(),
            &mMtuChangedToken);
    RETURN_IF_FAILED("Could not add MTU callback", return false)
    return true;
}

void QLowEnergyControllerPrivateWinRT::unregisterFromMtuChanges()
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    if (mDevice && mGattSession && mMtuChangedToken.value) {
        mGattSession->remove_MaxPduSizeChanged(mMtuChangedToken);
        mMtuChangedToken.value = 0;
    }
}

HRESULT QLowEnergyControllerPrivateWinRT::onStatusChange(IBluetoothLEDevice *dev, IInspectable *)
{
    Q_Q(QLowEnergyController);
    BluetoothConnectionStatus status;
    HRESULT hr;
    hr = dev->get_ConnectionStatus(&status);
    RETURN_IF_FAILED("Could not obtain connection status", return S_OK)
    if (state == QLowEnergyController::ConnectingState
        && status == BluetoothConnectionStatus::BluetoothConnectionStatus_Connected) {
        setState(QLowEnergyController::ConnectedState);
        emit q->connected();
    } else if (state != QLowEnergyController::UnconnectedState
        && status == BluetoothConnectionStatus::BluetoothConnectionStatus_Disconnected) {
        invalidateServices();
        unregisterFromValueChanges();
        unregisterFromStatusChanges();
        unregisterFromMtuChanges();
        mGattSession = nullptr;
        mDevice = nullptr;
        setError(QLowEnergyController::RemoteHostClosedError);
        setState(QLowEnergyController::UnconnectedState);
        emit q->disconnected();
    }
    return S_OK;
}

void QLowEnergyControllerPrivateWinRT::obtainIncludedServices(
        QSharedPointer<QLowEnergyServicePrivate> servicePointer,
        ComPtr<IGattDeviceService> service)
{
    Q_Q(QLowEnergyController);
    ComPtr<IGattDeviceService3> service3;
    HRESULT hr = service.As(&service3);
    RETURN_IF_FAILED("Could not cast service", return);
    ComPtr<IAsyncOperation<GattDeviceServicesResult *>> op;
    hr = service3->GetIncludedServicesAsync(&op);
    // Some devices return ERROR_ACCESS_DISABLED_BY_POLICY
    RETURN_IF_FAILED("Could not obtain included services", return);
    ComPtr<IGattDeviceServicesResult> result;
    hr = QWinRTFunctions::await(op, result.GetAddressOf(), QWinRTFunctions::ProcessMainThreadEvents, 5000);
    RETURN_IF_FAILED("Could not await service operation", return);
    // The device can be disconnected by the time we return from await()
    if (state != QLowEnergyController::DiscoveringState)
        return;
    GattCommunicationStatus status;
    hr = result->get_Status(&status);
    if (FAILED(hr) || status != GattCommunicationStatus_Success) {
        qErrnoWarning("Could not obtain list of included services");
        return;
    }
    ComPtr<IVectorView<GattDeviceService *>> includedServices;
    hr = result->get_Services(&includedServices);
    RETURN_IF_FAILED("Could not obtain service list", return);

    uint count;
    hr = includedServices->get_Size(&count);
    RETURN_IF_FAILED("Could not obtain service list's size", return);
    for (uint i = 0; i < count; ++i) {
        ComPtr<IGattDeviceService> includedService;
        hr = includedServices->GetAt(i, &includedService);
        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain service from list");
        GUID guuid;
        hr = includedService->get_Uuid(&guuid);
        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain included service's Uuid");
        const QBluetoothUuid includedUuid(guuid);
        QSharedPointer<QLowEnergyServicePrivate> includedPointer;
        qCDebug(QT_BT_WINDOWS_SERVICE_THREAD) << __FUNCTION__
                                            << "Changing service pointer from thread"
                                            << QThread::currentThread();
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

HRESULT QLowEnergyControllerPrivateWinRT::onServiceDiscoveryFinished(ABI::Windows::Foundation::IAsyncOperation<GattDeviceServicesResult *> *op, AsyncStatus status)
{
    // Check if the device is in the proper state, because it can already be
    // disconnected when the callback arrives.
    // Also the callback can theoretically come when the connection is
    // reestablisheed again (for example, if the user quickly clicks
    // "Disconnect" and then "Connect" again in some UI). But we can probably
    // omit such details, as we are connecting to the same device anyway.
    if (state != QLowEnergyController::DiscoveringState)
        return S_OK;

    Q_Q(QLowEnergyController);
    if (status != AsyncStatus::Completed) {
        qCDebug(QT_BT_WINDOWS) << "Could not obtain services";
        return S_OK;
    }
    ComPtr<IGattDeviceServicesResult> result;
    ComPtr<IVectorView<GattDeviceService *>> deviceServices;
    HRESULT hr = op->GetResults(&result);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain service discovery result",
                                      return S_OK);
    GattCommunicationStatus commStatus;
    hr = result->get_Status(&commStatus);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain service discovery status",
                                      return S_OK);
    if (commStatus != GattCommunicationStatus_Success)
        return S_OK;

    hr = result->get_Services(&deviceServices);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain service list",
                                      return S_OK);

    uint serviceCount;
    hr = deviceServices->get_Size(&serviceCount);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain service list size",
                                      return S_OK);
    for (uint i = 0; i < serviceCount; ++i) {
        ComPtr<IGattDeviceService> deviceService;
        hr = deviceServices->GetAt(i, &deviceService);
        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain service");
        GUID guuid;
        hr = deviceService->get_Uuid(&guuid);
        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain service's Uuid");
        const QBluetoothUuid service(guuid);
        m_openedServices[service] = deviceService;

        qCDebug(QT_BT_WINDOWS_SERVICE_THREAD) << __FUNCTION__
                                            << "Changing service pointer from thread"
                                            << QThread::currentThread();
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
        // The obtainIncludedServices method calls await(), so the device can be
        // disconnected by the time we return from it. TODO - rewrite in an
        // async way!
        if (state != QLowEnergyController::DiscoveringState) {
            emit q->discoveryFinished(); // Probably not needed when the device
                                         // is already disconnected?
            return S_OK;
        }

        emit q->serviceDiscovered(service);
    }

    setState(QLowEnergyController::DiscoveredState);
    emit q->discoveryFinished();

    return S_OK;
}

void QLowEnergyControllerPrivateWinRT::clearAllServices()
{
    // These services will be closed in the respective
    // QWinRTLowEnergyServiceHandler workers (in background threads).
    for (auto &uuid : m_requestDetailsServiceUuids)
        m_openedServices.remove(uuid);
    m_requestDetailsServiceUuids.clear();

    for (auto service : m_openedServices) {
        closeDeviceService(service);
    }
    m_openedServices.clear();
}

void QLowEnergyControllerPrivateWinRT::closeAndRemoveService(const QBluetoothUuid &uuid)
{
    auto service = m_openedServices.take(uuid);
    if (service)
        closeDeviceService(service);
}

void QLowEnergyControllerPrivateWinRT::discoverServices()
{
    qCDebug(QT_BT_WINDOWS) << "Service discovery initiated";
    // clear the previous services cache, as we request the services again
    clearAllServices();
    ComPtr<IBluetoothLEDevice3> device3;
    HRESULT hr = mDevice.As(&device3);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not cast device", return);
    ComPtr<IAsyncOperation<GenericAttributeProfile::GattDeviceServicesResult *>> asyncResult;
    hr = device3->GetGattServicesAsync(&asyncResult);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain services", return);
    hr = asyncResult->put_Completed(
        Callback<IAsyncOperationCompletedHandler<GenericAttributeProfile::GattDeviceServicesResult *>>(
                this, &QLowEnergyControllerPrivateWinRT::onServiceDiscoveryFinished).Get());
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not register services discovery callback",
                                      return);
}

void QLowEnergyControllerPrivateWinRT::discoverServiceDetails(
        const QBluetoothUuid &service, QLowEnergyService::DiscoveryMode mode)
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__ << service;
    if (!serviceList.contains(service)) {
        qCWarning(QT_BT_WINDOWS) << "Discovery done of unknown service:"
            << service.toString();
        return;
    }

    // clear the cache to rediscover service details
    closeAndRemoveService(service);

    auto serviceCallback = [service, mode, this](ComPtr<IGattDeviceService> deviceService) {
        discoverServiceDetailsHelper(service, mode, deviceService);
    };

    HRESULT hr = getNativeService(service, serviceCallback);
    if (FAILED(hr))
        qCDebug(QT_BT_WINDOWS) << "Could not obtain native service for uuid " << service;
}

void QLowEnergyControllerPrivateWinRT::discoverServiceDetailsHelper(
        const QBluetoothUuid &service, QLowEnergyService::DiscoveryMode mode,
        GattDeviceServiceComPtr deviceService)
{
    auto reactOnDiscoveryError = [](QSharedPointer<QLowEnergyServicePrivate> service,
                                    const QString &msg)
    {
        qCDebug(QT_BT_WINDOWS) << msg;
        service->setError(QLowEnergyService::UnknownError);
        service->setState(QLowEnergyService::RemoteService);
    };
    //update service data
    QSharedPointer<QLowEnergyServicePrivate> pointer = serviceList.value(service);
    qCDebug(QT_BT_WINDOWS_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                          << QThread::currentThread();
    pointer->setState(QLowEnergyService::RemoteServiceDiscovering);
    ComPtr<IGattDeviceService3> deviceService3;
    HRESULT hr = deviceService.As(&deviceService3);
    if (FAILED(hr)) {
        reactOnDiscoveryError(pointer, QStringLiteral("Could not cast service: %1").arg(hr));
        return;
    }
    ComPtr<IAsyncOperation<GattDeviceServicesResult *>> op;
    hr = deviceService3->GetIncludedServicesAsync(&op);
    if (FAILED(hr)) {
        reactOnDiscoveryError(pointer,
                              QStringLiteral("Could not obtain included service list: %1").arg(hr));
        return;
    }
    ComPtr<IGattDeviceServicesResult> result;
    hr = QWinRTFunctions::await(op, result.GetAddressOf());
    if (FAILED(hr)) {
        reactOnDiscoveryError(pointer,
                              QStringLiteral("Could not await service operation: %1").arg(hr));
        return;
    }
    GattCommunicationStatus status;
    hr = result->get_Status(&status);
    if (FAILED(hr) || status != GattCommunicationStatus_Success) {
        reactOnDiscoveryError(pointer,
                              QStringLiteral("Obtaining list of included services failed: %1").
                              arg(hr));
        return;
    }
    ComPtr<IVectorView<GattDeviceService *>> deviceServices;
    hr = result->get_Services(&deviceServices);
    if (FAILED(hr)) {
        reactOnDiscoveryError(pointer,
                              QStringLiteral("Could not obtain service list from result: %1").
                              arg(hr));
        return;
    }
    uint serviceCount;
    hr = deviceServices->get_Size(&serviceCount);
    if (FAILED(hr)) {
        reactOnDiscoveryError(pointer,
                              QStringLiteral("Could not obtain included service list's size: %1").
                              arg(hr));
        return;
    }
    for (uint i = 0; i < serviceCount; ++i) {
        ComPtr<IGattDeviceService> includedService;
        hr = deviceServices->GetAt(i, &includedService);
        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain service from list")
        GUID guuid;
        hr = includedService->get_Uuid(&guuid);
        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain service Uuid")

        const QBluetoothUuid service(guuid);
        if (service.isNull()) {
            qCDebug(QT_BT_WINDOWS) << "Could not find service";
            continue;
        }

        pointer->includedServices.append(service);

        // update the type of the included service
        QSharedPointer<QLowEnergyServicePrivate> otherService = serviceList.value(service);
        if (!otherService.isNull())
            otherService->type |= QLowEnergyService::IncludedService;
    }

    QWinRTLowEnergyServiceHandler *worker =
            new QWinRTLowEnergyServiceHandler(service, deviceService3, mode);
    m_requestDetailsServiceUuids.insert(service);
    QThread *thread = new QThread;
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &QWinRTLowEnergyServiceHandler::obtainCharList);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QObject::destroyed, thread, &QObject::deleteLater);
    connect(this, &QLowEnergyControllerPrivateWinRT::abortConnection,
            worker, &QWinRTLowEnergyServiceHandler::setAbortRequested);
    connect(worker, &QWinRTLowEnergyServiceHandler::errorOccured,
            this, &QLowEnergyControllerPrivateWinRT::handleServiceHandlerError);
    connect(worker, &QWinRTLowEnergyServiceHandler::charListObtained, this,
            [this](const QBluetoothUuid &service, QHash<QLowEnergyHandle,
            QLowEnergyServicePrivate::CharData> charList, QList<QBluetoothUuid> indicateChars,
            QLowEnergyHandle startHandle, QLowEnergyHandle endHandle) {
        if (!serviceList.contains(service)) {
            qCWarning(QT_BT_WINDOWS)
                    << "Discovery complete for unknown service:" << service.toString();
            return;
        }
        m_requestDetailsServiceUuids.remove(service);

        QSharedPointer<QLowEnergyServicePrivate> pointer = serviceList.value(service);
        pointer->startHandle = startHandle;
        pointer->endHandle = endHandle;
        pointer->characteristicList = charList;

        for (const QBluetoothUuid &indicateChar : std::as_const(indicateChars))
            registerForValueChanges(service, indicateChar);

        pointer->setState(QLowEnergyService::RemoteServiceDiscovered);
    });
    thread->start();
}

void QLowEnergyControllerPrivateWinRT::startAdvertising(
        const QLowEnergyAdvertisingParameters &,
        const QLowEnergyAdvertisingData &,
        const QLowEnergyAdvertisingData &)
{
    setError(QLowEnergyController::AdvertisingError);
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivateWinRT::stopAdvertising()
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivateWinRT::requestConnectionUpdate(const QLowEnergyConnectionParameters &)
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivateWinRT::readCharacteristic(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle)
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__ << service << charHandle;
    qCDebug(QT_BT_WINDOWS_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();
    Q_ASSERT(!service.isNull());
    if (role == QLowEnergyController::PeripheralRole) {
        service->setError(QLowEnergyService::CharacteristicReadError);
        Q_UNIMPLEMENTED();
        return;
    }

    if (!service->characteristicList.contains(charHandle)) {
        qCDebug(QT_BT_WINDOWS) << charHandle << "could not be found in service" << service->uuid;
        service->setError(QLowEnergyService::CharacteristicReadError);
        return;
    }

    const auto charData = service->characteristicList.value(charHandle);
    if (!(charData.properties & QLowEnergyCharacteristic::Read))
        qCDebug(QT_BT_WINDOWS) << "Read flag is not set for characteristic" << charData.uuid;

    auto characteristicCallback = [charHandle, service, this](
            ComPtr<IGattCharacteristic> characteristic) {
        readCharacteristicHelper(service, charHandle, characteristic);
    };

    HRESULT hr = getNativeCharacteristic(service->uuid, charData.uuid, characteristicCallback);
    if (FAILED(hr)) {
        qCDebug(QT_BT_WINDOWS) << "Could not obtain native characteristic" << charData.uuid
                               << "from service" << service->uuid;
        service->setError(QLowEnergyService::CharacteristicReadError);
    }
}

void QLowEnergyControllerPrivateWinRT::readCharacteristicHelper(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        GattCharacteristicComPtr characteristic)
{
    ComPtr<IAsyncOperation<GattReadResult *>> readOp;
    HRESULT hr = characteristic->ReadValueWithCacheModeAsync(BluetoothCacheMode_Uncached, &readOp);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not read characteristic",
                                   service, QLowEnergyService::CharacteristicReadError, return)
    auto readCompletedLambda = [charHandle, service]
            (IAsyncOperation<GattReadResult*> *op, AsyncStatus status)
    {
        if (status == AsyncStatus::Canceled || status == AsyncStatus::Error) {
            qCDebug(QT_BT_WINDOWS) << "Characteristic" << charHandle << "read operation failed.";
            service->setError(QLowEnergyService::CharacteristicReadError);
            return S_OK;
        }
        ComPtr<IGattReadResult> characteristicValue;
        HRESULT hr;
        hr = op->GetResults(&characteristicValue);
        CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain result for characteristic", service,
                                       QLowEnergyService::CharacteristicReadError, return S_OK)

        const QByteArray value = byteArrayFromGattResult(characteristicValue);
        auto charData = service->characteristicList.value(charHandle);
        charData.value = value;
        service->characteristicList.insert(charHandle, charData);
        emit service->characteristicRead(QLowEnergyCharacteristic(service, charHandle), value);
        return S_OK;
    };
    hr = readOp->put_Completed(Callback<IAsyncOperationCompletedHandler<GattReadResult *>>(
                                   readCompletedLambda).Get());
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not register characteristic read callback",
                                   service, QLowEnergyService::CharacteristicReadError, return)
}

void QLowEnergyControllerPrivateWinRT::readDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QLowEnergyHandle descHandle)
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__ << service << charHandle << descHandle;
    qCDebug(QT_BT_WINDOWS_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();
    Q_ASSERT(!service.isNull());
    if (role == QLowEnergyController::PeripheralRole) {
        service->setError(QLowEnergyService::DescriptorReadError);
        Q_UNIMPLEMENTED();
        return;
    }

    if (!service->characteristicList.contains(charHandle)) {
        qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle << "in characteristic" << charHandle
                             << "cannot be found in service" << service->uuid;
        service->setError(QLowEnergyService::DescriptorReadError);
        return;
    }

    const auto charData = service->characteristicList.value(charHandle);

    auto characteristicCallback = [charHandle, descHandle, service, this](
            ComPtr<IGattCharacteristic> characteristic) {
        readDescriptorHelper(service, charHandle, descHandle, characteristic);
    };

    HRESULT hr = getNativeCharacteristic(service->uuid, charData.uuid, characteristicCallback);
    if (FAILED(hr)) {
        qCDebug(QT_BT_WINDOWS) << "Could not obtain native characteristic" << charData.uuid
                               << "from service" << service->uuid;
        service->setError(QLowEnergyService::DescriptorReadError);
    }
}

void QLowEnergyControllerPrivateWinRT::readDescriptorHelper(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QLowEnergyHandle descHandle,
        GattCharacteristicComPtr characteristic)
{
    // Get native descriptor
    const auto charData = service->characteristicList.value(charHandle);
    if (!charData.descriptorList.contains(descHandle)) {
        qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle << "cannot be found in characteristic"
                               << charHandle;
    }
    const auto descData = charData.descriptorList.value(descHandle);
    const QBluetoothUuid descUuid = descData.uuid;
    if (descUuid ==
            QBluetoothUuid(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration)) {
        ComPtr<IAsyncOperation<ClientCharConfigDescriptorResult *>> readOp;
        HRESULT hr = characteristic->ReadClientCharacteristicConfigurationDescriptorAsync(&readOp);
        CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not read client characteristic configuration",
                                       service, QLowEnergyService::DescriptorReadError, return)
        auto readCompletedLambda = [charHandle, descHandle, service]
                (IAsyncOperation<ClientCharConfigDescriptorResult *> *op, AsyncStatus status)
        {
            if (status == AsyncStatus::Canceled || status == AsyncStatus::Error) {
                qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle << "read operation failed";
                service->setError(QLowEnergyService::DescriptorReadError);
                return S_OK;
            }
            ComPtr<IClientCharConfigDescriptorResult> iValue;
            HRESULT hr;
            hr = op->GetResults(&iValue);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain result for descriptor", service,
                                           QLowEnergyService::DescriptorReadError, return S_OK)
            GattClientCharacteristicConfigurationDescriptorValue value;
            hr = iValue->get_ClientCharacteristicConfigurationDescriptor(&value);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain value for descriptor", service,
                                           QLowEnergyService::DescriptorReadError, return S_OK)
            quint16 result = 0;
            bool correct = false;
            if (value & GattClientCharacteristicConfigurationDescriptorValue_Indicate) {
                result |= QLowEnergyCharacteristic::Indicate;
                correct = true;
            }
            if (value & GattClientCharacteristicConfigurationDescriptorValue_Notify) {
                result |= QLowEnergyCharacteristic::Notify;
                correct = true;
            }
            if (value == GattClientCharacteristicConfigurationDescriptorValue_None)
                correct = true;
            if (!correct) {
                qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle
                                       << "read operation failed. Obtained unexpected value.";
                service->setError(QLowEnergyService::DescriptorReadError);
                return S_OK;
            }
            QLowEnergyServicePrivate::DescData descData;
            descData.uuid = QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration;
            descData.value = QByteArray(2, Qt::Uninitialized);
            qToLittleEndian(result, descData.value.data());
            service->characteristicList[charHandle].descriptorList[descHandle] = descData;
            emit service->descriptorRead(QLowEnergyDescriptor(service, charHandle, descHandle),
                                         descData.value);
            return S_OK;
        };
        hr = readOp->put_Completed(
                    Callback<IAsyncOperationCompletedHandler<ClientCharConfigDescriptorResult *>>(
                        readCompletedLambda).Get());
        CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not register descriptor read callback",
                                       service, QLowEnergyService::DescriptorReadError, return)
        return;
    }

    ComPtr<IGattCharacteristic3> characteristic3;
    HRESULT hr = characteristic.As(&characteristic3);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not cast characteristic",
                                   service, QLowEnergyService::DescriptorReadError, return)
    ComPtr<IAsyncOperation<GattDescriptorsResult *>> op;
    hr = characteristic3->GetDescriptorsForUuidAsync(descData.uuid, &op);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain descriptor for uuid",
                                   service, QLowEnergyService::DescriptorReadError, return)
    ComPtr<IGattDescriptorsResult> result;
    hr = QWinRTFunctions::await(op, result.GetAddressOf(),
                                QWinRTFunctions::ProcessMainThreadEvents, 5000);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not await descritpor read result",
                                   service, QLowEnergyService::DescriptorReadError, return)

    GattCommunicationStatus commStatus;
    hr = result->get_Status(&commStatus);
    if (FAILED(hr) || commStatus != GattCommunicationStatus_Success) {
        qErrnoWarning("Could not obtain list of descriptors");
        service->setError(QLowEnergyService::DescriptorReadError);
        return;
    }

    ComPtr<IVectorView<GattDescriptor *>> descriptors;
    hr = result->get_Descriptors(&descriptors);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain descriptor list",
                                   service, QLowEnergyService::DescriptorReadError, return)
    uint size;
    hr = descriptors->get_Size(&size);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not await descritpor list's size",
                                   service, QLowEnergyService::DescriptorReadError, return)
    if (size == 0) {
        qCWarning(QT_BT_WINDOWS) << "No descriptor with uuid" << descData.uuid << "was found.";
        service->setError(QLowEnergyService::DescriptorReadError);
        return;
    } else if (size > 1) {
        qCWarning(QT_BT_WINDOWS) << "There is more than 1 descriptor with uuid" << descData.uuid;
    }

    ComPtr<IGattDescriptor> descriptor;
    hr = descriptors->GetAt(0, &descriptor);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain descritpor from list",
                                   service, QLowEnergyService::DescriptorReadError, return)
    ComPtr<IAsyncOperation<GattReadResult*>> readOp;
    hr = descriptor->ReadValueWithCacheModeAsync(BluetoothCacheMode_Uncached, &readOp);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not read descriptor value",
                                   service, QLowEnergyService::DescriptorReadError, return)
    auto readCompletedLambda = [charHandle, descHandle, descUuid, service]
            (IAsyncOperation<GattReadResult*> *op, AsyncStatus status)
    {
        if (status == AsyncStatus::Canceled || status == AsyncStatus::Error) {
            qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle << "read operation failed";
            service->setError(QLowEnergyService::DescriptorReadError);
            return S_OK;
        }
        ComPtr<IGattReadResult> descriptorValue;
        HRESULT hr;
        hr = op->GetResults(&descriptorValue);
        if (FAILED(hr)) {
            qCDebug(QT_BT_WINDOWS) << "Could not obtain result for descriptor" << descHandle;
            service->setError(QLowEnergyService::DescriptorReadError);
            return S_OK;
        }
        QLowEnergyServicePrivate::DescData descData;
        descData.uuid = descUuid;
        if (descData.uuid == QBluetoothUuid::DescriptorType::CharacteristicUserDescription)
            descData.value = byteArrayFromGattResult(descriptorValue, true);
        else
            descData.value = byteArrayFromGattResult(descriptorValue);
        service->characteristicList[charHandle].descriptorList[descHandle] = descData;
        emit service->descriptorRead(QLowEnergyDescriptor(service, charHandle, descHandle),
                                     descData.value);
        return S_OK;
    };
    hr = readOp->put_Completed(Callback<IAsyncOperationCompletedHandler<GattReadResult *>>(
                                   readCompletedLambda).Get());
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not register descriptor read callback",
                                   service, QLowEnergyService::DescriptorReadError, return)
}

void QLowEnergyControllerPrivateWinRT::writeCharacteristic(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QByteArray &newValue,
        QLowEnergyService::WriteMode mode)
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__ << service << charHandle << newValue << mode;
    qCDebug(QT_BT_WINDOWS_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();
    Q_ASSERT(!service.isNull());
    if (role == QLowEnergyController::PeripheralRole) {
        service->setError(QLowEnergyService::CharacteristicWriteError);
        Q_UNIMPLEMENTED();
        return;
    }
    if (!service->characteristicList.contains(charHandle)) {
        qCDebug(QT_BT_WINDOWS) << "Characteristic" << charHandle << "cannot be found in service"
                             << service->uuid;
        service->setError(QLowEnergyService::CharacteristicWriteError);
        return;
    }

    QLowEnergyServicePrivate::CharData charData = service->characteristicList.value(charHandle);
    const bool writeWithResponse = mode == QLowEnergyService::WriteWithResponse;
    if (!(charData.properties & (writeWithResponse ? QLowEnergyCharacteristic::Write
             : QLowEnergyCharacteristic::WriteNoResponse)))
        qCDebug(QT_BT_WINDOWS) << "Write flag is not set for characteristic" << charHandle;

    auto characteristicCallback = [charHandle, service, newValue, writeWithResponse, this](
            ComPtr<IGattCharacteristic> characteristic) {
        writeCharacteristicHelper(service, charHandle, newValue, writeWithResponse,
                                  characteristic);
    };

    HRESULT hr = getNativeCharacteristic(service->uuid, charData.uuid, characteristicCallback);
    if (FAILED(hr)) {
        qCDebug(QT_BT_WINDOWS) << "Could not obtain native characteristic" << charData.uuid
                               << "from service" << service->uuid;
        service->setError(QLowEnergyService::CharacteristicWriteError);
    }
}

void QLowEnergyControllerPrivateWinRT::writeCharacteristicHelper(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle, const QByteArray &newValue,
        bool writeWithResponse, GattCharacteristicComPtr characteristic)
{
    ComPtr<ABI::Windows::Storage::Streams::IBufferFactory> bufferFactory;
    HRESULT hr = GetActivationFactory(
                    HStringReference(RuntimeClass_Windows_Storage_Streams_Buffer).Get(),
                    &bufferFactory);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain buffer factory",
                                   service, QLowEnergyService::CharacteristicWriteError, return)
    ComPtr<ABI::Windows::Storage::Streams::IBuffer> buffer;
    const quint32 length = quint32(newValue.length());
    hr = bufferFactory->Create(length, &buffer);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not create buffer",
                                   service, QLowEnergyService::CharacteristicWriteError, return)
    hr = buffer->put_Length(length);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not set buffer length",
                                   service, QLowEnergyService::CharacteristicWriteError, return)
    ComPtr<Windows::Storage::Streams::IBufferByteAccess> byteAccess;
    hr = buffer.As(&byteAccess);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not cast buffer",
                                   service, QLowEnergyService::CharacteristicWriteError, return)
    byte *bytes;
    hr = byteAccess->Buffer(&bytes);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not set buffer",
                                   service, QLowEnergyService::CharacteristicWriteError, return)
    memcpy(bytes, newValue, length);
    ComPtr<IAsyncOperation<GattCommunicationStatus>> writeOp;
    GattWriteOption option = writeWithResponse ? GattWriteOption_WriteWithResponse
                                               : GattWriteOption_WriteWithoutResponse;
    hr = characteristic->WriteValueWithOptionAsync(buffer.Get(), option, &writeOp);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could write characteristic",
                                   service, QLowEnergyService::CharacteristicWriteError, return)
    const auto charData = service->characteristicList.value(charHandle);
    QPointer<QLowEnergyControllerPrivateWinRT> thisPtr(this);
    auto writeCompletedLambda =
            [charData, charHandle, newValue, service, writeWithResponse, thisPtr]
                (IAsyncOperation<GattCommunicationStatus> *op, AsyncStatus status)
    {
        if (status == AsyncStatus::Canceled || status == AsyncStatus::Error) {
            qCDebug(QT_BT_WINDOWS) << "Characteristic" << charHandle << "write operation failed";
            service->setError(QLowEnergyService::CharacteristicWriteError);
            return S_OK;
        }
        GattCommunicationStatus result;
        HRESULT hr;
        hr = op->GetResults(&result);
        if (hr == E_BLUETOOTH_ATT_INVALID_ATTRIBUTE_VALUE_LENGTH) {
            qCDebug(QT_BT_WINDOWS) << "Characteristic" << charHandle
                                   << "write operation was tried with invalid value length";
            service->setError(QLowEnergyService::CharacteristicWriteError);
            return S_OK;
        }
        CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain characteristic write result", service,
                                       QLowEnergyService::CharacteristicWriteError, return S_OK)
        if (result != GattCommunicationStatus_Success) {
            qCDebug(QT_BT_WINDOWS) << "Characteristic" << charHandle << "write operation failed";
            service->setError(QLowEnergyService::CharacteristicWriteError);
            return S_OK;
        }
        // only update cache when property is readable. Otherwise it remains
        // empty.
        if (thisPtr && charData.properties & QLowEnergyCharacteristic::Read)
            thisPtr->updateValueOfCharacteristic(charHandle, newValue, false);
        if (writeWithResponse) {
            emit service->characteristicWritten(QLowEnergyCharacteristic(service, charHandle),
                                                newValue);
        }
        return S_OK;
    };
    hr = writeOp->put_Completed(
                Callback<IAsyncOperationCompletedHandler<GattCommunicationStatus>>(
                    writeCompletedLambda).Get());
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not register characteristic write callback",
                                   service, QLowEnergyService::CharacteristicWriteError, return)
}

void QLowEnergyControllerPrivateWinRT::writeDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QLowEnergyHandle descHandle,
        const QByteArray &newValue)
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__ << service << charHandle << descHandle << newValue;
    qCDebug(QT_BT_WINDOWS_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();
    Q_ASSERT(!service.isNull());
    if (role == QLowEnergyController::PeripheralRole) {
        service->setError(QLowEnergyService::DescriptorWriteError);
        Q_UNIMPLEMENTED();
        return;
    }

    if (!service->characteristicList.contains(charHandle)) {
        qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle << "in characteristic" << charHandle
                             << "could not be found in service" << service->uuid;
        service->setError(QLowEnergyService::DescriptorWriteError);
        return;
    }

    const auto charData = service->characteristicList.value(charHandle);

    auto characteristicCallback = [descHandle, charHandle, service, newValue, this](
            ComPtr<IGattCharacteristic> characteristic) {
        writeDescriptorHelper(service, charHandle, descHandle, newValue, characteristic);
    };

    HRESULT hr = getNativeCharacteristic(service->uuid, charData.uuid, characteristicCallback);
    if (FAILED(hr)) {
        qCDebug(QT_BT_WINDOWS) << "Could not obtain native characteristic" << charData.uuid
                               << "from service" << service->uuid;
        service->setError(QLowEnergyService::DescriptorWriteError);
    }
}

void QLowEnergyControllerPrivateWinRT::writeDescriptorHelper(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QLowEnergyHandle descHandle,
        const QByteArray &newValue,
        GattCharacteristicComPtr characteristic)
{
    // Get native descriptor
    const auto charData = service->characteristicList.value(charHandle);
    if (!charData.descriptorList.contains(descHandle)) {
        qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle
                               << "could not be found in Characteristic" << charHandle;
    }

    QLowEnergyServicePrivate::DescData descData = charData.descriptorList.value(descHandle);
    if (descData.uuid ==
            QBluetoothUuid(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration)) {
        GattClientCharacteristicConfigurationDescriptorValue value;
        quint16 intValue = qFromLittleEndian<quint16>(newValue);
        if (intValue & GattClientCharacteristicConfigurationDescriptorValue_Indicate
                && intValue & GattClientCharacteristicConfigurationDescriptorValue_Notify) {
            qCWarning(QT_BT_WINDOWS) << "Setting both Indicate and Notify "
                                        "is not supported on WinRT";
            value = GattClientCharacteristicConfigurationDescriptorValue(
                    (GattClientCharacteristicConfigurationDescriptorValue_Indicate
                     | GattClientCharacteristicConfigurationDescriptorValue_Notify));
        } else if (intValue & GattClientCharacteristicConfigurationDescriptorValue_Indicate) {
            value = GattClientCharacteristicConfigurationDescriptorValue_Indicate;
        } else if (intValue & GattClientCharacteristicConfigurationDescriptorValue_Notify) {
            value = GattClientCharacteristicConfigurationDescriptorValue_Notify;
        } else if (intValue == 0) {
            value = GattClientCharacteristicConfigurationDescriptorValue_None;
        } else {
            qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle
                                   << "write operation failed: Invalid value";
            service->setError(QLowEnergyService::DescriptorWriteError);
            return;
        }
        ComPtr<IAsyncOperation<enum GattCommunicationStatus>> writeOp;
        HRESULT hr =
            characteristic->WriteClientCharacteristicConfigurationDescriptorAsync(value, &writeOp);
        CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not write client characteristic configuration",
                                       service, QLowEnergyService::DescriptorWriteError, return)
        QPointer<QLowEnergyControllerPrivateWinRT> thisPtr(this);
        auto writeCompletedLambda = [charHandle, descHandle, newValue, service, thisPtr]
                (IAsyncOperation<GattCommunicationStatus> *op, AsyncStatus status)
        {
            if (status == AsyncStatus::Canceled || status == AsyncStatus::Error) {
                qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle << "write operation failed";
                service->setError(QLowEnergyService::DescriptorWriteError);
                return S_OK;
            }
            GattCommunicationStatus result;
            HRESULT hr;
            hr = op->GetResults(&result);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain result for descriptor", service,
                                           QLowEnergyService::DescriptorWriteError, return S_OK)
            if (result != GattCommunicationStatus_Success) {
                qCWarning(QT_BT_WINDOWS) << "Descriptor" << descHandle << "write operation failed";
                service->setError(QLowEnergyService::DescriptorWriteError);
                return S_OK;
            }
            if (thisPtr)
                thisPtr->updateValueOfDescriptor(charHandle, descHandle, newValue, false);
            emit service->descriptorWritten(QLowEnergyDescriptor(service, charHandle, descHandle),
                                            newValue);
            return S_OK;
        };
        hr = writeOp->put_Completed(
                    Callback<IAsyncOperationCompletedHandler<GattCommunicationStatus >>(
                        writeCompletedLambda).Get());
        CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not register descriptor write callback",
                                       service, QLowEnergyService::DescriptorWriteError, return)
        return;
    }

    ComPtr<IGattCharacteristic3> characteristic3;
    HRESULT hr = characteristic.As(&characteristic3);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not cast characteristic",
                                   service, QLowEnergyService::DescriptorWriteError, return)
    ComPtr<IAsyncOperation<GattDescriptorsResult *>> op;
    hr = characteristic3->GetDescriptorsForUuidAsync(descData.uuid, &op);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain descriptor from Uuid",
                                   service, QLowEnergyService::DescriptorWriteError, return)
    ComPtr<IGattDescriptorsResult> result;
    hr = QWinRTFunctions::await(op, result.GetAddressOf(),
                                QWinRTFunctions::ProcessMainThreadEvents, 5000);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not await descriptor operation",
                                   service, QLowEnergyService::DescriptorWriteError, return)
    GattCommunicationStatus commStatus;
    hr = result->get_Status(&commStatus);
    if (FAILED(hr) || commStatus != GattCommunicationStatus_Success) {
        qCWarning(QT_BT_WINDOWS) << "Descriptor operation failed";
        service->setError(QLowEnergyService::DescriptorWriteError);
        return;
    }
    ComPtr<IVectorView<GattDescriptor *>> descriptors;
    hr = result->get_Descriptors(&descriptors);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain list of descriptors",
                                   service, QLowEnergyService::DescriptorWriteError, return)
    uint size;
    hr = descriptors->get_Size(&size);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain list of descriptors' size",
                                   service, QLowEnergyService::DescriptorWriteError, return)
    if (size == 0) {
        qCWarning(QT_BT_WINDOWS) << "No descriptor with uuid" << descData.uuid << "was found.";
        return;
    } else if (size > 1) {
        qCWarning(QT_BT_WINDOWS) << "There is more than 1 descriptor with uuid" << descData.uuid;
    }
    ComPtr<IGattDescriptor> descriptor;
    hr = descriptors->GetAt(0, &descriptor);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain descriptor",
                                   service, QLowEnergyService::DescriptorWriteError, return)
    ComPtr<ABI::Windows::Storage::Streams::IBufferFactory> bufferFactory;
    hr = GetActivationFactory(
                HStringReference(RuntimeClass_Windows_Storage_Streams_Buffer).Get(),
                &bufferFactory);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain buffer factory",
                                   service, QLowEnergyService::DescriptorWriteError, return)
    ComPtr<ABI::Windows::Storage::Streams::IBuffer> buffer;
    const quint32 length = quint32(newValue.length());
    hr = bufferFactory->Create(length, &buffer);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not create buffer",
                                   service, QLowEnergyService::DescriptorWriteError, return)
    hr = buffer->put_Length(length);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not set buffer length",
                                   service, QLowEnergyService::DescriptorWriteError, return)
    ComPtr<Windows::Storage::Streams::IBufferByteAccess> byteAccess;
    hr = buffer.As(&byteAccess);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not cast buffer",
                                   service, QLowEnergyService::DescriptorWriteError, return)
    byte *bytes;
    hr = byteAccess->Buffer(&bytes);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not set buffer",
                                   service, QLowEnergyService::DescriptorWriteError, return)
    memcpy(bytes, newValue, length);
    ComPtr<IAsyncOperation<GattCommunicationStatus>> writeOp;
    hr = descriptor->WriteValueAsync(buffer.Get(), &writeOp);
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not write descriptor value",
                                   service, QLowEnergyService::DescriptorWriteError, return)
    QPointer<QLowEnergyControllerPrivateWinRT> thisPtr(this);
    auto writeCompletedLambda = [charHandle, descHandle, newValue, service, thisPtr]
            (IAsyncOperation<GattCommunicationStatus> *op, AsyncStatus status)
    {
        if (status == AsyncStatus::Canceled || status == AsyncStatus::Error) {
            qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle << "write operation failed";
            service->setError(QLowEnergyService::DescriptorWriteError);
            return S_OK;
        }
        GattCommunicationStatus result;
        HRESULT hr;
        hr = op->GetResults(&result);
        CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain result for descriptor", service,
                                       QLowEnergyService::DescriptorWriteError, return S_OK)
        if (result != GattCommunicationStatus_Success) {
            qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle << "write operation failed";
            service->setError(QLowEnergyService::DescriptorWriteError);
            return S_OK;
        }
        if (thisPtr)
            thisPtr->updateValueOfDescriptor(charHandle, descHandle, newValue, false);
        emit service->descriptorWritten(QLowEnergyDescriptor(service, charHandle, descHandle),
                                        newValue);
        return S_OK;
    };
    hr = writeOp->put_Completed(
                Callback<IAsyncOperationCompletedHandler<GattCommunicationStatus>>(
                    writeCompletedLambda).Get());
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not register descriptor write callback",
                                   service, QLowEnergyService::DescriptorWriteError, return)
}

void QLowEnergyControllerPrivateWinRT::addToGenericAttributeList(const QLowEnergyServiceData &,
                                                                    QLowEnergyHandle)
{
    Q_UNIMPLEMENTED();
}

int QLowEnergyControllerPrivateWinRT::mtu() const
{
    uint16_t mtu = 23;
    if (!mGattSession) {
        qCDebug(QT_BT_WINDOWS) << "mtu queried before GattSession available. Using default mtu.";
        return mtu;
    }

    HRESULT hr = mGattSession->get_MaxPduSize(&mtu);
    RETURN_IF_FAILED("could not obtain MTU size", return mtu);
    qCDebug(QT_BT_WINDOWS) << "mtu determined to be" << mtu;
    return mtu;
}

void QLowEnergyControllerPrivateWinRT::handleCharacteristicChanged(
        quint16 charHandle, const QByteArray &data)
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__ << charHandle << data;
    qCDebug(QT_BT_WINDOWS_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();
    QSharedPointer<QLowEnergyServicePrivate> service =
            serviceForHandle(charHandle);
    if (service.isNull())
        return;

    qCDebug(QT_BT_WINDOWS) << "Characteristic change notification" << service->uuid
                           << charHandle << data.toHex();

    QLowEnergyCharacteristic characteristic = characteristicForHandle(charHandle);
    if (!characteristic.isValid()) {
        qCWarning(QT_BT_WINDOWS) << "characteristicChanged: Cannot find characteristic";
        return;
    }

    // only update cache when property is readable. Otherwise it remains
    // empty.
    if (characteristic.properties() & QLowEnergyCharacteristic::Read)
        updateValueOfCharacteristic(characteristic.attributeHandle(),
                                data, false);
    emit service->characteristicChanged(characteristic, data);
}

void QLowEnergyControllerPrivateWinRT::handleServiceHandlerError(const QString &error)
{
    if (state != QLowEnergyController::DiscoveringState)
        return;

    qCWarning(QT_BT_WINDOWS) << "Error while discovering services:" << error;
    setState(QLowEnergyController::UnconnectedState);
    setError(QLowEnergyController::ConnectionError);
}

void QLowEnergyControllerPrivateWinRT::handleConnectionError(const char *logMessage)
{
    qCWarning(QT_BT_WINDOWS) << logMessage;
    setError(QLowEnergyController::ConnectionError);
    setState(QLowEnergyController::UnconnectedState);
    unregisterFromStatusChanges();
    unregisterFromMtuChanges();
}

QT_END_NAMESPACE

#include "qlowenergycontroller_winrt.moc"

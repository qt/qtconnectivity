/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2014 Denis Shienkov <denis.shienkov@gmail.com>
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
#include "qbluetoothuuid.h"
#include "qbluetoothdevicediscoveryagent_p.h"
#include "qbluetoothlocaldevice_p.h"

#include <QtCore/qmutex.h>

#include <qt_windows.h>
#include <setupapi.h>
#include <bluetoothapis.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

struct LeDeviceEntry {
    LeDeviceEntry(const QString &path, const QBluetoothAddress &address)
        : devicePath(path), deviceAddress(address) {}
    QString devicePath;
    QBluetoothAddress deviceAddress;
};

Q_GLOBAL_STATIC(QList<LeDeviceEntry>, cachedLeDeviceEntries)
Q_GLOBAL_STATIC(QMutex, cachedLeDeviceEntriesGuard)

static QString devicePropertyString(
        HDEVINFO hDeviceInfo,
        const PSP_DEVINFO_DATA deviceInfoData,
        DWORD registryProperty)
{
    DWORD propertyRegDataType = 0;
    DWORD propertyBufferSize = 0;
    QByteArray propertyBuffer;

    while (!::SetupDiGetDeviceRegistryProperty(
               hDeviceInfo,
               deviceInfoData,
               registryProperty,
               &propertyRegDataType,
               propertyBuffer.isEmpty() ? NULL : reinterpret_cast<PBYTE>(propertyBuffer.data()),
               propertyBuffer.size(),
               &propertyBufferSize)) {

        const DWORD error = ::GetLastError();
        if (error != ERROR_INSUFFICIENT_BUFFER
                || (propertyRegDataType != REG_SZ
                    && propertyRegDataType != REG_EXPAND_SZ)) {
            return QString();
        }

        // add +2 byte to allow to successfully convert to qstring
        propertyBuffer.fill(0, propertyBufferSize + sizeof(wchar_t));
    }

    return QString::fromWCharArray(reinterpret_cast<const wchar_t *>(
                                       propertyBuffer.constData()));
}

static QString deviceName(HDEVINFO hDeviceInfo, PSP_DEVINFO_DATA deviceInfoData)
{
    return devicePropertyString(hDeviceInfo, deviceInfoData, SPDRP_FRIENDLYNAME);
}

static QString deviceSystemPath(const PSP_INTERFACE_DEVICE_DETAIL_DATA detailData)
{
    return QString::fromWCharArray(detailData->DevicePath);
}

static QBluetoothAddress deviceAddress(const QString &devicePath)
{
    const int firstbound = devicePath.indexOf(QStringLiteral("dev_"));
    const int lastbound = devicePath.indexOf(QLatin1Char('#'), firstbound);
    const QString hex = devicePath.mid(firstbound + 4, lastbound - firstbound - 4);
    bool ok = false;
    return QBluetoothAddress(hex.toULongLong(&ok, 16));
}

static QBluetoothDeviceInfo createClassicDeviceInfo(const BLUETOOTH_DEVICE_INFO &foundDevice)
{
    QBluetoothDeviceInfo deviceInfo(
                QBluetoothAddress(foundDevice.Address.ullLong),
                QString::fromWCharArray(foundDevice.szName),
                foundDevice.ulClassofDevice);

    if (foundDevice.fRemembered)
        deviceInfo.setCached(true);
    return deviceInfo;
}

static QBluetoothDeviceInfo findFirstClassicDevice(
        int *systemErrorCode, HBLUETOOTH_DEVICE_FIND *searchHandle)
{
    BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams;
    ::ZeroMemory(&searchParams, sizeof(searchParams));
    searchParams.dwSize = sizeof(searchParams);
    searchParams.cTimeoutMultiplier = 10; // 12.8 sec
    searchParams.fIssueInquiry = TRUE;
    searchParams.fReturnAuthenticated = TRUE;
    searchParams.fReturnConnected = TRUE;
    searchParams.fReturnRemembered = TRUE;
    searchParams.fReturnUnknown = TRUE;
    searchParams.hRadio = NULL;

    BLUETOOTH_DEVICE_INFO deviceInfo;
    ::ZeroMemory(&deviceInfo, sizeof(deviceInfo));
    deviceInfo.dwSize = sizeof(deviceInfo);

    const HBLUETOOTH_DEVICE_FIND hFind = ::BluetoothFindFirstDevice(
                &searchParams, &deviceInfo);

    QBluetoothDeviceInfo foundDevice;
    if (hFind) {
        *searchHandle = hFind;
        *systemErrorCode = NO_ERROR;
        foundDevice = createClassicDeviceInfo(deviceInfo);
    } else {
        *systemErrorCode = ::GetLastError();
    }

    return foundDevice;
}

static QBluetoothDeviceInfo findNextClassicDevice(
        int *systemErrorCode, HBLUETOOTH_DEVICE_FIND searchHandle)
{
    BLUETOOTH_DEVICE_INFO deviceInfo;
    ::ZeroMemory(&deviceInfo, sizeof(deviceInfo));
    deviceInfo.dwSize = sizeof(deviceInfo);

    QBluetoothDeviceInfo foundDevice;
    if (!::BluetoothFindNextDevice(searchHandle, &deviceInfo)) {
        *systemErrorCode = ::GetLastError();
    } else {
        *systemErrorCode = NO_ERROR;
        foundDevice = createClassicDeviceInfo(deviceInfo);
    }

    return foundDevice;
}

static void closeClassicSearch(HBLUETOOTH_DEVICE_FIND *searchHandle)
{
    if (searchHandle && *searchHandle) {
        ::BluetoothFindDeviceClose(*searchHandle);
        *searchHandle = 0;
    }
}

static QList<QBluetoothDeviceInfo> enumerateLeDevices(
        int *systemErrorCode)
{
    const QUuid deviceInterfaceGuid("781aee18-7733-4ce4-add0-91f41c67b592");
    const HDEVINFO hDeviceInfo = ::SetupDiGetClassDevs(
                reinterpret_cast<const GUID *>(&deviceInterfaceGuid),
                NULL,
                0,
                DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (hDeviceInfo == INVALID_HANDLE_VALUE) {
        *systemErrorCode = ::GetLastError();
        return QList<QBluetoothDeviceInfo>();
    }

    QList<QBluetoothDeviceInfo> foundDevices;
    DWORD index = 0;

    QList<LeDeviceEntry> cachedEntries;

    forever {
        SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
        ::ZeroMemory(&deviceInterfaceData, sizeof(deviceInterfaceData));
        deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

        if (!::SetupDiEnumDeviceInterfaces(
                    hDeviceInfo,
                    NULL,
                    reinterpret_cast<const GUID *>(&deviceInterfaceGuid),
                    index++,
                    &deviceInterfaceData)) {
            *systemErrorCode = ::GetLastError();
            break;
        }

        DWORD deviceInterfaceDetailDataSize = 0;
        if (!::SetupDiGetDeviceInterfaceDetail(
                    hDeviceInfo,
                    &deviceInterfaceData,
                    NULL,
                    deviceInterfaceDetailDataSize,
                    &deviceInterfaceDetailDataSize,
                    NULL)) {
            if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
                *systemErrorCode = ::GetLastError();
                break;
            }
        }

        SP_DEVINFO_DATA deviceInfoData;
        ::ZeroMemory(&deviceInfoData, sizeof(deviceInfoData));
        deviceInfoData.cbSize = sizeof(deviceInfoData);

        QByteArray deviceInterfaceDetailDataBuffer(
                    deviceInterfaceDetailDataSize, 0);

        PSP_INTERFACE_DEVICE_DETAIL_DATA deviceInterfaceDetailData =
                reinterpret_cast<PSP_INTERFACE_DEVICE_DETAIL_DATA>
                (deviceInterfaceDetailDataBuffer.data());

        deviceInterfaceDetailData->cbSize =
                sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

        if (!::SetupDiGetDeviceInterfaceDetail(
                    hDeviceInfo,
                    &deviceInterfaceData,
                    deviceInterfaceDetailData,
                    deviceInterfaceDetailDataBuffer.size(),
                    &deviceInterfaceDetailDataSize,
                    &deviceInfoData)) {
            *systemErrorCode = ::GetLastError();
            break;
        }

        const QString systemPath = deviceSystemPath(deviceInterfaceDetailData);
        const QBluetoothAddress address = deviceAddress(systemPath);
        if (address.isNull())
            continue;
        const QString name = deviceName(hDeviceInfo, &deviceInfoData);

        QBluetoothDeviceInfo deviceInfo(address, name, QBluetoothDeviceInfo::MiscellaneousDevice);
        deviceInfo.setCoreConfigurations(QBluetoothDeviceInfo::LowEnergyCoreConfiguration);
        deviceInfo.setCached(true);

        foundDevices << deviceInfo;
        cachedEntries << LeDeviceEntry(systemPath, address);
    }

    QMutexLocker locker(cachedLeDeviceEntriesGuard());
    cachedLeDeviceEntries()->swap(cachedEntries);

    ::SetupDiDestroyDeviceInfoList(hDeviceInfo);
    return foundDevices;
}

QString QBluetoothDeviceDiscoveryAgentPrivate::discoveredLeDeviceSystemPath(
        const QBluetoothAddress &deviceAddress)
{
    // update LE devices cache
    int dummyErrorCode;
    enumerateLeDevices(&dummyErrorCode);

    QMutexLocker locker(cachedLeDeviceEntriesGuard());
    for (int i = 0; i < cachedLeDeviceEntries()->count(); ++i) {
        if (cachedLeDeviceEntries()->at(i).deviceAddress == deviceAddress)
            return cachedLeDeviceEntries()->at(i).devicePath;
    }
    return QString();
}

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(
        const QBluetoothAddress &deviceAdapter,
        QBluetoothDeviceDiscoveryAgent *parent)
    : inquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry)
    , lastError(QBluetoothDeviceDiscoveryAgent::NoError)
    , adapterAddress(deviceAdapter)
    , pendingCancel(false)
    , pendingStart(false)
    , scanWatcher(Q_NULLPTR)
    , active(false)
    , systemErrorCode(NO_ERROR)
    , searchHandle(0)
    , q_ptr(parent)
{
    scanWatcher = new QFutureWatcher<QBluetoothDeviceInfo>(this);
    connect(scanWatcher, &QFutureWatcher<QBluetoothDeviceInfo>::finished,
            this, &QBluetoothDeviceDiscoveryAgentPrivate::taskFinished);
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
    if (active)
        stop();

    scanWatcher->waitForFinished();
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    if (pendingStart)
        return true;
    if (pendingCancel)
        return false;
    return active;
}

void QBluetoothDeviceDiscoveryAgentPrivate::start()
{
    if (pendingCancel == true) {
        pendingStart = true;
        return;
    }

    const QList<QBluetoothHostInfo> foundLocalAdapters =
            QBluetoothLocalDevicePrivate::localAdapters();

    Q_Q(QBluetoothDeviceDiscoveryAgent);

    if (foundLocalAdapters.isEmpty()) {
        qCWarning(QT_BT_WINDOWS) << "Device does not support Bluetooth";
        lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Device does not support Bluetooth");
        emit q->error(lastError);
        return;
    }

    // Check for the local adapter address.
    bool foundLocalAdapter = false;
    foreach (const QBluetoothHostInfo &adapterInfo, foundLocalAdapters) {
        if (adapterAddress == QBluetoothAddress() || adapterAddress == adapterInfo.address()) {
            foundLocalAdapter = true;
            break;
        }
    }

    if (!foundLocalAdapter) {
        qCWarning(QT_BT_WINDOWS) << "Incorrect local adapter passed.";
        lastError = QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Passed address is not a local device.");
        emit q->error(lastError);
        return;
    }

    discoveredDevices.clear();
    active = true;

    // Start scan for first classic device.
    const QFuture<QBluetoothDeviceInfo> future = QtConcurrent::run(
                findFirstClassicDevice, &systemErrorCode, &searchHandle);
    scanWatcher->setFuture(future);
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    if (!active)
        return;

    pendingCancel = true;
    pendingStart = false;
}

void QBluetoothDeviceDiscoveryAgentPrivate::taskFinished()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    if (pendingCancel && !pendingStart) {
        closeClassicSearch(&searchHandle);
        active = false;
        pendingCancel = false;
        emit q->canceled();
    } else if (pendingStart) {
        closeClassicSearch(&searchHandle);
        pendingStart = pendingCancel = false;
        start();
    } else {
        if (systemErrorCode == ERROR_NO_MORE_ITEMS) {
                closeClassicSearch(&searchHandle);
                // Enumerate LE devices.
                const QList<QBluetoothDeviceInfo> foundDevices =
                        enumerateLeDevices(&systemErrorCode);
                if (systemErrorCode == ERROR_NO_MORE_ITEMS) {
                    foreach (const QBluetoothDeviceInfo &foundDevice, foundDevices)
                        processDiscoveredDevice(foundDevice);
                    active = false;
                    emit q->finished();
                } else {
                    pendingStart = pendingCancel = false;
                    active = false;
                    lastError = (systemErrorCode == ERROR_INVALID_HANDLE) ?
                                QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError
                              : QBluetoothDeviceDiscoveryAgent::InputOutputError;
                    errorString = qt_error_string(systemErrorCode);
                    emit q->error(lastError);
                }
        } else if (systemErrorCode == NO_ERROR) {
            processDiscoveredDevice(scanWatcher->result());
            // Start scan for next classic device.
            const QFuture<QBluetoothDeviceInfo> future = QtConcurrent::run(
                        findNextClassicDevice, &systemErrorCode, searchHandle);
            scanWatcher->setFuture(future);
        } else {
            closeClassicSearch(&searchHandle);
            pendingStart = pendingCancel = false;
            active = false;
            lastError = (systemErrorCode == ERROR_INVALID_HANDLE) ?
                        QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError
                      : QBluetoothDeviceDiscoveryAgent::InputOutputError;
            errorString = qt_error_string(systemErrorCode);
            emit q->error(lastError);
        }
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::processDiscoveredDevice(
        const QBluetoothDeviceInfo &foundDevice)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    for (int i = 0; i < discoveredDevices.size(); i++) {
        QBluetoothDeviceInfo mergedDevice = discoveredDevices[i];

        if (mergedDevice.address() == foundDevice.address()) {
            if (mergedDevice == foundDevice
                    || mergedDevice.coreConfigurations() == foundDevice.coreConfigurations()) {
                qCDebug(QT_BT_WINDOWS) << "Duplicate: " << foundDevice.address();
                return;
            }

            // We assume that if the existing device it is low energy, it means that
            // the found device should be as classic, because it is impossible to get
            // same low energy device.
            if (mergedDevice.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
                mergedDevice = foundDevice;

            // We assume that it is impossible to have multiple devices with same core
            // configurations, which have one address. This possible only in case a device
            // provided both low energy and classic features at the same time.
            mergedDevice.setCoreConfigurations(QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration);
            mergedDevice.setCached(foundDevice.isCached());

            discoveredDevices.replace(i, mergedDevice);
            Q_Q(QBluetoothDeviceDiscoveryAgent);
            qCDebug(QT_BT_WINDOWS) << "Updated: " << mergedDevice.address();

            emit q->deviceDiscovered(mergedDevice);
            return;
        }
    }

    qCDebug(QT_BT_WINDOWS) << "Emit: " << foundDevice.address();
    discoveredDevices.append(foundDevice);
    emit q->deviceDiscovered(foundDevice);
}

QT_END_NAMESPACE

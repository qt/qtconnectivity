/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#include <qbluetoothutils_winrt_p.h>

#include "qbluetoothlocaldevice.h"
#include "qbluetoothaddress.h"

#include "qbluetoothlocaldevice_p.h"
#include "qbluetoothutils_winrt_p.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Radios.h>
#include <wrl.h>

#include <QtCore/private/qfunctions_winrt_p.h>
#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <QtCore/QLoggingCategory>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Devices::Enumeration;
using namespace winrt::Windows::Devices::Bluetooth;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace winrt::Windows::Devices::Radios;

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

template <typename T>
static bool await(IAsyncOperation<T> &&asyncInfo, T &result, uint timeout = 0)
{
    using WinRtAsyncStatus = winrt::Windows::Foundation::AsyncStatus;
    WinRtAsyncStatus status;
    QElapsedTimer timer;
    if (timeout)
        timer.start();
    do {
        QCoreApplication::processEvents();
        status = asyncInfo.Status();
    } while (status == WinRtAsyncStatus::Started && (!timeout || !timer.hasExpired(timeout)));
    if (status == WinRtAsyncStatus::Completed) {
        result = asyncInfo.GetResults();
        return true;
    } else {
        return false;
    }
}

static QBluetoothLocalDevice::HostMode adjustHostMode(QBluetoothLocalDevice::HostMode mode)
{
    // Windows APIs do not seem to support HostDiscoverable and
    // HostDiscoverableLimitedInquiry, so we just treat them as HostConnectable
    return (mode == QBluetoothLocalDevice::HostPoweredOff)
                ? mode : QBluetoothLocalDevice::HostConnectable;
}

static QBluetoothLocalDevice::HostMode modeFromWindowsBluetoothState(RadioState state)
{
    return (state == RadioState::On) ? QBluetoothLocalDevice::HostConnectable
                                     : QBluetoothLocalDevice::HostPoweredOff;
}

static RadioState windowsStateFromMode(QBluetoothLocalDevice::HostMode mode)
{
    return (mode == QBluetoothLocalDevice::HostPoweredOff) ? RadioState::Off : RadioState::On;
}

class AdapterManager;

class WatcherWrapper : public QObject, public std::enable_shared_from_this<WatcherWrapper>
{
    Q_OBJECT
public:
    WatcherWrapper(winrt::hstring selector) : mWatcher(DeviceInformation::CreateWatcher(selector))
    {
    }
    ~WatcherWrapper()
    {
        if (mWatcher && mInitialized) {
            mWatcher.Stop();
            unsubscribeFromEvents();
        }
    }

    template<typename AddedSlot, typename RemovedSlot>
    bool init(AdapterManager *manager, AddedSlot onAdded, RemovedSlot onRemoved)
    {
        if (!mWatcher) {
            qWarning() << "Windows failed to create an instance of DeviceWatcher. "
                       << "QBluetoothLocalDevice might fail to provide some information.";
            return false;
        }

        subscribeToEvents();
        connect(this, &WatcherWrapper::deviceAdded, manager, onAdded, Qt::QueuedConnection);
        connect(this, &WatcherWrapper::deviceRemoved, manager, onRemoved, Qt::QueuedConnection);
        mInitialized = true;
        mWatcher.Start();
        return true;
    }

signals:
    void deviceAdded(winrt::hstring id);
    void deviceRemoved(winrt::hstring id);

private:
    void subscribeToEvents()
    {
        // The callbacks are triggered from separate threads. So we capture
        // thisPtr to make sure that the object is valid.
        auto thisPtr = shared_from_this();
        mAddedToken = mWatcher.Added([thisPtr](DeviceWatcher, const DeviceInformation &info) {
            emit thisPtr->deviceAdded(info.Id());
        });
        mRemovedToken =
                mWatcher.Removed([thisPtr](DeviceWatcher, const DeviceInformationUpdate &upd) {
                    emit thisPtr->deviceRemoved(upd.Id());
                });
    }
    void unsubscribeFromEvents()
    {
        mWatcher.Added(mAddedToken);
        mWatcher.Removed(mRemovedToken);
    }

    bool mInitialized = false;
    DeviceWatcher mWatcher = nullptr;
    winrt::event_token mAddedToken;
    winrt::event_token mRemovedToken;
};

/*
    This class is supposed to manage winrt::Windows::Devices::Radios::Radio
    instances. It looks like Windows behaves incorrectly when there are multiple
    instances representing the same physical device. So this class will be a
    single point for keeping track of all used Radios.
    At the same time this class takes care of monitoring adapter connections
    and disconnections.
    Note that access to internal structs should be protected, because all
    Windows callbacks come in separate threads. We also can't use
    Qt::QueuedConnection to "forward" the execution to the right thread, because
    Windows' IUnknown-based classes have a deleted operator new(), and so can't
    be used in QMetaType.
    However, we will still mostly use signals/slots to communicate with actual
    QBluetoothLocalDevice instances, so we do not need any protection on that
    side.
*/
class AdapterManager : public QObject
{
    Q_OBJECT
public:
    AdapterManager();
    ~AdapterManager();

    QList<QBluetoothAddress> connectedDevices();

public slots:
    QBluetoothLocalDevice::HostMode addClient(QBluetoothLocalDevicePrivate *client);
    void removeClient(winrt::hstring adapterId);
    void updateMode(winrt::hstring adapterId, QBluetoothLocalDevice::HostMode mode);

signals:
    void adapterAdded(winrt::hstring id);
    void adapterRemoved(winrt::hstring id);
    void modeChanged(winrt::hstring id, QBluetoothLocalDevice::HostMode mode);
    void deviceAdded(const QBluetoothAddress &address);
    void deviceRemoved(const QBluetoothAddress &address);

private:
    struct RadioInfo {
        Radio radio = nullptr;
        winrt::event_token stateToken;
        int numClients = 0;
        RadioState currentState = RadioState::Unknown;
    };

    Radio getRadioFromAdapterId(winrt::hstring id);
    Q_SLOT void onAdapterAdded(winrt::hstring id);
    Q_SLOT void onAdapterRemoved(winrt::hstring id);
    void subscribeToStateChanges(RadioInfo &info);
    void unsubscribeFromStateChanges(RadioInfo &info);
    void onStateChange(Radio radio);
    Q_SLOT void tryResubscribeToStateChanges(winrt::hstring id, int numAttempts);

    Q_SLOT void onDeviceAdded(winrt::hstring id);
    Q_SLOT void onDeviceRemoved(winrt::hstring id);

    std::shared_ptr<WatcherWrapper> mAdapterWatcher = nullptr;
    std::shared_ptr<WatcherWrapper> mLeDevicesWatcher = nullptr;
    std::shared_ptr<WatcherWrapper> mClassicDevicesWatcher = nullptr;
    QMutex mMutex;
    // Key for this map is BluetoothAdapter Id, *not* Radio Id.
    QMap<winrt::hstring, RadioInfo> mRadios;
    QList<QBluetoothAddress> mConnectedDevices;
};

AdapterManager::AdapterManager() : QObject()
{
    qRegisterMetaType<winrt::hstring>("winrt::hstring");

    const auto adapterSelector = BluetoothAdapter::GetDeviceSelector();
    mAdapterWatcher = std::make_shared<WatcherWrapper>(adapterSelector);
    mAdapterWatcher->init(this, &AdapterManager::onAdapterAdded, &AdapterManager::onAdapterRemoved);

    // Once created, device watchers will also populate the initial list of
    // connected devices.

    const auto leSelector = BluetoothLEDevice::GetDeviceSelectorFromConnectionStatus(
            BluetoothConnectionStatus::Connected);
    mLeDevicesWatcher = std::make_shared<WatcherWrapper>(leSelector);
    mLeDevicesWatcher->init(this, &AdapterManager::onDeviceAdded, &AdapterManager::onDeviceRemoved);

    const auto classicSelector = BluetoothDevice::GetDeviceSelectorFromConnectionStatus(
            BluetoothConnectionStatus::Connected);
    mClassicDevicesWatcher = std::make_unique<WatcherWrapper>(classicSelector);
    mClassicDevicesWatcher->init(this, &AdapterManager::onDeviceAdded,
                                 &AdapterManager::onDeviceRemoved);
}

AdapterManager::~AdapterManager()
{
}

QList<QBluetoothAddress> AdapterManager::connectedDevices()
{
    QMutexLocker locker(&mMutex);
    return mConnectedDevices;
}

QBluetoothLocalDevice::HostMode AdapterManager::addClient(QBluetoothLocalDevicePrivate *client)
{
    connect(this, &AdapterManager::modeChanged, client,
            &QBluetoothLocalDevicePrivate::radioModeChanged, Qt::QueuedConnection);
    connect(client, &QBluetoothLocalDevicePrivate::updateMode, this, &AdapterManager::updateMode,
            Qt::QueuedConnection);
    connect(this, &AdapterManager::adapterAdded, client,
            &QBluetoothLocalDevicePrivate::onAdapterAdded, Qt::QueuedConnection);
    connect(this, &AdapterManager::adapterRemoved, client,
            &QBluetoothLocalDevicePrivate::onAdapterRemoved, Qt::QueuedConnection);
    connect(this, &AdapterManager::deviceAdded, client,
            &QBluetoothLocalDevicePrivate::onDeviceAdded, Qt::QueuedConnection);
    connect(this, &AdapterManager::deviceRemoved, client,
            &QBluetoothLocalDevicePrivate::onDeviceRemoved, Qt::QueuedConnection);

    QMutexLocker locker(&mMutex);
    const auto adapterId = client->mDeviceId;
    if (mRadios.contains(adapterId)) {
        auto &radioInfo = mRadios[adapterId];
        radioInfo.numClients++;
        return modeFromWindowsBluetoothState(radioInfo.radio.State());
    } else {
        // Note that when we use await(), we need to unlock the mutex, because
        // it calls processEvents(), so other methods that demand the mutex can
        // be invoked.
        locker.unlock();
        Radio r = getRadioFromAdapterId(adapterId);
        if (r) {
            locker.relock();
            RadioInfo info;
            info.radio = r;
            info.numClients = 1;
            info.currentState = r.State();
            subscribeToStateChanges(info);
            mRadios.insert(adapterId, info);
            return modeFromWindowsBluetoothState(info.currentState);
        }
    }
    qCWarning(QT_BT_WINDOWS, "Failed to subscribe to adapter state changes");
    return QBluetoothLocalDevice::HostPoweredOff;
}

void AdapterManager::removeClient(winrt::hstring adapterId)
{
    QMutexLocker locker(&mMutex);
    if (mRadios.contains(adapterId)) {
        auto &radioInfo = mRadios[adapterId];
        if (--radioInfo.numClients == 0) {
            unsubscribeFromStateChanges(radioInfo);
            mRadios.remove(adapterId);
        }
    } else {
        qCWarning(QT_BT_WINDOWS) << "Removing client for an unknown adapter id"
                                 << QString::fromStdString(winrt::to_string(adapterId));
    }
}

void AdapterManager::updateMode(winrt::hstring adapterId, QBluetoothLocalDevice::HostMode mode)
{
    QMutexLocker locker(&mMutex);
    if (mRadios.contains(adapterId)) {
        RadioAccessStatus status = RadioAccessStatus::Unspecified;
        auto radio = mRadios[adapterId].radio; // can be nullptr
        locker.unlock();
        if (radio) {
            const bool res = await(radio.SetStateAsync(windowsStateFromMode(mode)), status);
            // If operation succeeds, we will update the state in the event handler.
            if (!res || status != RadioAccessStatus::Allowed) {
                qCWarning(QT_BT_WINDOWS, "Failed to update adapter state: SetStateAsync() failed!");
                if (status == RadioAccessStatus::DeniedBySystem) {
                    qCWarning(QT_BT_WINDOWS) << "Check that the user has permissions to manipulate"
                                                " the selected Bluetooth device";
                }
            }
        }
    }
}

Radio AdapterManager::getRadioFromAdapterId(winrt::hstring id)
{
    BluetoothAdapter a(nullptr);
    bool res = await(BluetoothAdapter::FromIdAsync(id), a);
    if (res && a) {
        Radio r(nullptr);
        res = await(a.GetRadioAsync(), r);
        if (res && r)
            return r;
    }
    return nullptr;
}

void AdapterManager::onStateChange(Radio radio)
{
    QMutexLocker locker(&mMutex);
    for (const auto &key : mRadios.keys()) {
        auto &info = mRadios[key];
        if (info.radio == radio) {
            if (info.currentState != radio.State()) {
                info.currentState = radio.State();
                emit modeChanged(key, modeFromWindowsBluetoothState(info.currentState));
            }
            break;
        }
    }
}

static const int kMaximumAttempts = 5;

// In practice when the adapter is reconnected, the Radio object can't be
// retrieved immediately. I'm not sure if such behavior is normal, or specific
// to my machine only. I also do not know what is the proper time to wait before
// the Radio instance can be retrieved. So we introduce a helper method, which
// tries to resubscribe several times with a 100ms interval between retries.
void AdapterManager::tryResubscribeToStateChanges(winrt::hstring id, int numAttempts)
{
    QMutexLocker locker(&mMutex);
    if (mRadios.contains(id)) {
        // The Added event can come when we first create and use adapter. Such
        // event should not be handled.
        if (mRadios[id].radio != nullptr)
            return;
        locker.unlock();
        Radio r = getRadioFromAdapterId(id);
        if (r) {
            locker.relock();
            // We have to check once again because the record could be deleted
            // while we were await'ing in getRadioFromAdapterId().
            if (mRadios.contains(id)) {
                auto &info = mRadios[id];
                info.radio = r;
                info.currentState = r.State();
                subscribeToStateChanges(info);
                emit modeChanged(id, modeFromWindowsBluetoothState(info.currentState));
            }
        } else {
            if (++numAttempts < kMaximumAttempts) {
                qCDebug(QT_BT_WINDOWS, "Trying to resubscribe for the state changes");
                QPointer<AdapterManager> thisPtr(this);
                QTimer::singleShot(100, [thisPtr, id, numAttempts]() {
                    if (thisPtr)
                        thisPtr->tryResubscribeToStateChanges(id, numAttempts);
                });
            } else {
                qCWarning(QT_BT_WINDOWS,
                          "Failed to resubscribe to the state changes after %d attempts!",
                          numAttempts);
            }
        }
    }
}

struct BluetoothInfo
{
    QBluetoothAddress address;
    bool isConnected;
};

static BluetoothInfo getBluetoothInfo(winrt::hstring id)
{
    // We do not know if it's a BT classic or BTLE device, so we try both.
    BluetoothDevice device(nullptr);
    bool res = await(BluetoothDevice::FromIdAsync(id), device, 5000);
    if (res && device) {
        return { QBluetoothAddress(device.BluetoothAddress()),
                 device.ConnectionStatus() == BluetoothConnectionStatus::Connected };
    }

    BluetoothLEDevice leDevice(nullptr);
    res = await(BluetoothLEDevice::FromIdAsync(id), leDevice, 5000);
    if (res && leDevice) {
        return { QBluetoothAddress(leDevice.BluetoothAddress()),
                 leDevice.ConnectionStatus() == BluetoothConnectionStatus::Connected };
    }

    return {};
}

void AdapterManager::onDeviceAdded(winrt::hstring id)
{
    const BluetoothInfo &info = getBluetoothInfo(id);
    // In practice this callback might come even for disconnected device.
    // So check status explicitly.
    if (!info.address.isNull() && info.isConnected) {
        bool found = false;
        {
            // A scope is needed, because we need to emit a signal when mutex is already unlocked
            QMutexLocker locker(&mMutex);
            found = mConnectedDevices.contains(info.address);
            if (!found) {
                mConnectedDevices.push_back(info.address);
            }
        }
        if (!found)
            emit deviceAdded(info.address);
    }
}

void AdapterManager::onDeviceRemoved(winrt::hstring id)
{
    const BluetoothInfo &info = getBluetoothInfo(id);
    if (!info.address.isNull() && !info.isConnected) {
        bool found = false;
        {
            QMutexLocker locker(&mMutex);
            found = mConnectedDevices.removeOne(info.address);
        }
        if (found)
            emit deviceRemoved(info.address);
    }
}

void AdapterManager::onAdapterAdded(winrt::hstring id)
{
    emit adapterAdded(id);
    // We need to invoke the method from a Qt thread, so that we could start a
    // timer there.
    QMetaObject::invokeMethod(this, "tryResubscribeToStateChanges", Qt::QueuedConnection,
                              Q_ARG(winrt::hstring, id), Q_ARG(int, 0));
}

void AdapterManager::onAdapterRemoved(winrt::hstring id)
{
    emit adapterRemoved(id);
    QMutexLocker locker(&mMutex);
    if (mRadios.contains(id)) {
        // here we can't simply remove the record from the map, because the
        // same adapter can later be reconnected, and we need to keep track of
        // the created clients.
        mRadios[id].radio = nullptr;
    }
}

void AdapterManager::subscribeToStateChanges(AdapterManager::RadioInfo &info)
{
    QPointer<AdapterManager> thisPtr(this);
    info.stateToken = info.radio.StateChanged([thisPtr](Radio r, const auto &) {
        // This callback fires twice. Looks like an MS bug.
        // This callback comes in a separate thread
        if (thisPtr) {
            thisPtr->onStateChange(r);
        }
    });
}

void AdapterManager::unsubscribeFromStateChanges(AdapterManager::RadioInfo &info)
{
    // This method can be called after the radio is disconnected
    if (info.radio)
        info.radio.StateChanged(info.stateToken);
}

Q_GLOBAL_STATIC(AdapterManager, adapterManager)

static DeviceInformationCollection getAvailableAdapters()
{
    const auto btSelector = BluetoothAdapter::GetDeviceSelector();
    DeviceInformationCollection deviceInfoCollection(nullptr);
    await(DeviceInformation::FindAllAsync(btSelector), deviceInfoCollection);
    return deviceInfoCollection;
}

DeviceInformationPairing pairingInfoFromAddress(const QBluetoothAddress &address)
{
    const quint64 addr64 = address.toUInt64();
    BluetoothDevice device(nullptr);
    bool res = await(BluetoothDevice::FromBluetoothAddressAsync(addr64), device, 5000);
    if (res && device)
        return device.DeviceInformation().Pairing();

    BluetoothLEDevice leDevice(nullptr);
    res = await(BluetoothLEDevice::FromBluetoothAddressAsync(addr64), leDevice, 5000);
    if (res && leDevice)
        return leDevice.DeviceInformation().Pairing();

    return nullptr;
}

struct PairingWorker
        : public winrt::implements<PairingWorker, winrt::Windows::Foundation::IInspectable>
{
    PairingWorker(QBluetoothLocalDevice *device): q(device) {}
    ~PairingWorker() = default;

    void pairAsync(const QBluetoothAddress &addr, QBluetoothLocalDevice::Pairing pairing);

private:
    QPointer<QBluetoothLocalDevice> q;
    void onPairingRequested(DeviceInformationCustomPairing const&,
                            DevicePairingRequestedEventArgs args);
};

void PairingWorker::pairAsync(const QBluetoothAddress &addr, QBluetoothLocalDevice::Pairing pairing)
{
    // Note: some of the functions called here use await() in their implementation.
    // Hence we need to verify that the 'q' is still valid after such calls, as
    // it may have been destroyed. In that scenario the ComPtr pointing to this
    // object is destroyed, but to protect this function's possible execution at that time,
    // the get_strong() call below will ensure we can execute until the end.
    auto ref = get_strong();
    DeviceInformationPairing pairingInfo = pairingInfoFromAddress(addr);
    switch (pairing) {
    case QBluetoothLocalDevice::Paired:
    case QBluetoothLocalDevice::AuthorizedPaired:
    {
        DeviceInformationCustomPairing customPairing = pairingInfo.Custom();
        auto token = customPairing.PairingRequested(
                    { get_weak(), &PairingWorker::onPairingRequested });
        DevicePairingResult result{nullptr};
        bool res = await(customPairing.PairAsync(DevicePairingKinds::ConfirmOnly), result, 30000);
        customPairing.PairingRequested(token);
        if (!res || result.Status() != DevicePairingResultStatus::Paired) {
            if (q)
                emit q->errorOccurred(QBluetoothLocalDevice::PairingError);
            return;
        }

        // Check the actual protection level used and signal the success
        if (q) {
            const auto resultingPairingStatus = q->pairingStatus(addr);
            // pairingStatus() uses await/spins eventloop => check 'q' validity again
            if (q)
                emit q->pairingFinished(addr, resultingPairingStatus);
        }
        return;
    }
    case QBluetoothLocalDevice::Unpaired:
        DeviceUnpairingResult unpairingResult{nullptr};
        bool res = await(pairingInfo.UnpairAsync(), unpairingResult, 10000);
        if (!res || unpairingResult.Status() != DeviceUnpairingResultStatus::Unpaired) {
            if (q)
                emit q->errorOccurred(QBluetoothLocalDevice::PairingError);
            return;
        }
        if (q)
            emit q->pairingFinished(addr, QBluetoothLocalDevice::Unpaired);
        return;
    }
}

void PairingWorker::onPairingRequested(const DeviceInformationCustomPairing &,
                                       DevicePairingRequestedEventArgs args)
{
    if (args.PairingKind() != DevicePairingKinds::ConfirmOnly) {
        Q_ASSERT(false);
        return;
    }

    args.Accept();
}

QBluetoothLocalDevice::QBluetoothLocalDevice(QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothLocalDevicePrivate(this, QBluetoothAddress()))
{
    registerQBluetoothLocalDeviceMetaType();
}

QBluetoothLocalDevice::QBluetoothLocalDevice(const QBluetoothAddress &address, QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothLocalDevicePrivate(this, address))
{
    registerQBluetoothLocalDeviceMetaType();
}

QBluetoothLocalDevicePrivate::QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *q,
                                                           QBluetoothAddress address)
    : q_ptr(q),
      mAdapter(nullptr),
      mMode(QBluetoothLocalDevice::HostPoweredOff)
{
    mPairingWorker = winrt::make_self<PairingWorker>(q);
    if (address.isNull()) {
        // use default adapter
        bool res = await(BluetoothAdapter::GetDefaultAsync(), mAdapter);
        if (res && mAdapter) {
            // get adapter name
            mDeviceId = mAdapter.DeviceId();
            DeviceInformation devInfo(nullptr);
            res = await(DeviceInformation::CreateFromIdAsync(mDeviceId), devInfo);
            if (res && devInfo)
                mAdapterName = QString::fromStdString(winrt::to_string(devInfo.Name()));
        }
    } else {
        // try to select a proper device
        const auto deviceInfoCollection = getAvailableAdapters();
        for (const auto &devInfo : deviceInfoCollection) {
            BluetoothAdapter adapter(nullptr);
            const bool res = await(BluetoothAdapter::FromIdAsync(devInfo.Id()), adapter);
            if (res && adapter) {
                QBluetoothAddress adapterAddress(adapter.BluetoothAddress());
                if (adapterAddress == address) {
                    mAdapter = adapter;
                    mDeviceId = adapter.DeviceId();
                    mAdapterName = QString::fromStdString(winrt::to_string(devInfo.Name()));
                    break;
                }
            }
        }
    }
    if (mAdapter) {
        mMode = adapterManager->addClient(this);
    } else {
        if (address.isNull()) {
            qCWarning(QT_BT_WINDOWS, "Failed to create QBluetoothLocalDevice - no adapter found");
        } else {
            qCWarning(QT_BT_WINDOWS) << "Failed to create QBluetoothLocalDevice for address"
                                     << address;
        }
    }
}

QBluetoothLocalDevicePrivate::~QBluetoothLocalDevicePrivate()
{
    adapterManager->removeClient(mDeviceId);
    mAdapter = nullptr;
}

bool QBluetoothLocalDevicePrivate::isValid() const
{
    return mAdapter != nullptr;
}

void QBluetoothLocalDevicePrivate::updateAdapterState(QBluetoothLocalDevice::HostMode mode)
{
    if (!mAdapter) {
        qCWarning(QT_BT_WINDOWS, "Trying to update state for an uninitialized adapter");
        return;
    }
    const auto desiredMode = adjustHostMode(mode);
    if (desiredMode != mMode) {
        // From the MS docs: Note that your code should call RequestAccessAsync
        // at least once, from the UI thread, before trying to call
        // SetStateAsync. This is because in some regions, with some user
        // settings choices, attempting to change radio state requires user
        // permission.
        RadioAccessStatus status = RadioAccessStatus::Unspecified;
        bool res = await(Radio::RequestAccessAsync(), status);
        if (res && status == RadioAccessStatus::Allowed) {
            // Now send a signal to the AdapterWatcher. That class will manage
            // the actual state change.
            emit updateMode(mDeviceId, desiredMode);
        } else {
            qCWarning(QT_BT_WINDOWS, "Failed to update adapter state: operation denied!");
        }
    }
}

void QBluetoothLocalDevicePrivate::onAdapterRemoved(winrt::hstring id)
{
    if (id == mDeviceId) {
        qCDebug(QT_BT_WINDOWS, "Current adapter is removed");
        mAdapter = nullptr;
        if (mMode != QBluetoothLocalDevice::HostPoweredOff) {
            mMode = QBluetoothLocalDevice::HostPoweredOff;
            emit q_ptr->hostModeStateChanged(mMode);
        }
    }
}

void QBluetoothLocalDevicePrivate::onAdapterAdded(winrt::hstring id)
{
    if (id == mDeviceId && !mAdapter) {
        // adapter was reconnected - try to recreate the internals
        qCDebug(QT_BT_WINDOWS, "Adapter reconnected - trying to restore QBluetoothLocalDevice");
        const bool res = await(BluetoothAdapter::FromIdAsync(mDeviceId), mAdapter);
        if (!res || !mAdapter)
            qCWarning(QT_BT_WINDOWS, "Failed to restore adapter");
    }
}

void QBluetoothLocalDevicePrivate::radioModeChanged(winrt::hstring id, QBluetoothLocalDevice::HostMode mode)
{
    if (id == mDeviceId && mAdapter) {
        if (mode != mMode) {
            mMode = mode;
            emit q_ptr->hostModeStateChanged(mMode);
        }
    }
}

void QBluetoothLocalDevicePrivate::onDeviceAdded(const QBluetoothAddress &address)
{
    if (isValid())
        emit q_ptr->deviceConnected(address);
}

void QBluetoothLocalDevicePrivate::onDeviceRemoved(const QBluetoothAddress &address)
{
    if (isValid())
        emit q_ptr->deviceDisconnected(address);
}

void QBluetoothLocalDevice::requestPairing(const QBluetoothAddress &address, Pairing pairing)
{
    Q_D(QBluetoothLocalDevice);
    if (!isValid() || address.isNull()) {
        QMetaObject::invokeMethod(this, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothLocalDevice::Error,
                                        QBluetoothLocalDevice::PairingError));
        return;
    }

    // Check current pairing status and determine if there is a need to do anything
    const Pairing currentPairingStatus = pairingStatus(address);
    if ((currentPairingStatus == Unpaired && pairing == Unpaired)
            || (currentPairingStatus != Unpaired && pairing != Unpaired)) {
           qCDebug(QT_BT_WINDOWS) << "requestPairing() no change needed to pairing" << address;
           QMetaObject::invokeMethod(this, "pairingFinished", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothAddress, address),
                                  Q_ARG(QBluetoothLocalDevice::Pairing,
                                  currentPairingStatus));
        return;
    }

    qCDebug(QT_BT_WINDOWS) << "requestPairing() initiating (un)pairing" << address << pairing;
    d->mPairingWorker->pairAsync(address, pairing);
}

QBluetoothLocalDevice::Pairing QBluetoothLocalDevice::pairingStatus(
    const QBluetoothAddress &address) const
{
    if (!isValid() || address.isNull())
        return Unpaired;

    const DeviceInformationPairing pairingInfo = pairingInfoFromAddress(address);
    if (!pairingInfo || !pairingInfo.IsPaired())
        return Unpaired;

    const DevicePairingProtectionLevel protection = pairingInfo.ProtectionLevel();
    if (protection == DevicePairingProtectionLevel::Encryption
            || protection == DevicePairingProtectionLevel::EncryptionAndAuthentication)
        return AuthorizedPaired;
    return Paired;
}

void QBluetoothLocalDevice::setHostMode(QBluetoothLocalDevice::HostMode mode)
{
    Q_D(QBluetoothLocalDevice);
    d->updateAdapterState(mode);
}

QBluetoothLocalDevice::HostMode QBluetoothLocalDevice::hostMode() const
{
    Q_D(const QBluetoothLocalDevice);
    return d->mMode;
}

QList<QBluetoothAddress> QBluetoothLocalDevice::connectedDevices() const
{
    // On windows we have only one Bluetooth adapter, but generally can create multiple
    // QBluetoothLocalDevice instances (both valid and invalid). Invalid instances shouldn't return
    // any connected devices, while all valid instances can use information from a global static.
    Q_D(const QBluetoothLocalDevice);
    return d->isValid() ? adapterManager->connectedDevices() : QList<QBluetoothAddress>();
}

void QBluetoothLocalDevice::powerOn()
{
    setHostMode(HostConnectable);
}

QString QBluetoothLocalDevice::name() const
{
    Q_D(const QBluetoothLocalDevice);
    return d->mAdapterName;
}

QBluetoothAddress QBluetoothLocalDevice::address() const
{
    Q_D(const QBluetoothLocalDevice);
    return d->mAdapter ? QBluetoothAddress(d->mAdapter.BluetoothAddress()) : QBluetoothAddress();
}

QList<QBluetoothHostInfo> QBluetoothLocalDevice::allDevices()
{
    QList<QBluetoothHostInfo> devices;
    const auto deviceInfoCollection = getAvailableAdapters();
    if (deviceInfoCollection) {
        for (const auto &devInfo : deviceInfoCollection) {
            BluetoothAdapter adapter(nullptr);
            const bool res = await(BluetoothAdapter::FromIdAsync(devInfo.Id()), adapter);
            if (res && adapter) {
                QBluetoothHostInfo info;
                info.setName(QString::fromStdString(winrt::to_string(devInfo.Name())));
                info.setAddress(QBluetoothAddress(adapter.BluetoothAddress()));
                devices.push_back(std::move(info));
            }
        }
    }
    return devices;
}

#include "qbluetoothlocaldevice_winrt.moc"

QT_END_NAMESPACE

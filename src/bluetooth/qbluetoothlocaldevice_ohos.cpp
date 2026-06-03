// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothlocaldevice_p.h"

#include <QtBluetooth/qbluetoothaddress.h>
#include <QtBluetooth/qbluetoothlocaldevice.h>
#include <QtBluetooth/private/qohosbluetoothaccess_p.h>
#include <QtBluetooth/private/qohosbluetoothcommon_p.h>
#include <QtBluetooth/private/qohosbluetoothlocaldevice_p.h>

#include <QtCore/qdeadlinetimer.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qtimer.h>

#include <algorithm>
#include <chrono>
#include <memory>
#include <optional>

QT_BEGIN_NAMESPACE

using QtOhosBluetooth::QOhosBluetoothErrorCode;

Q_DECLARE_LOGGING_CATEGORY(QT_BT_OHOS)

namespace {

constexpr auto hostModeRequestRetryDelay = std::chrono::milliseconds(500);
constexpr auto hostModeRequestGiveUpDelay = std::chrono::seconds(20);

QBluetoothLocalDevice::HostMode mapScanModeToQtHostMode(
    QtOhosBluetooth::QOhosBluetoothLocalDeviceProxy::ScanMode scanMode)
{
    using ScanMode = QtOhosBluetooth::QOhosBluetoothLocalDeviceProxy::ScanMode;

    switch (scanMode) {
    case ScanMode::SCAN_MODE_CONNECTABLE:
        return QBluetoothLocalDevice::HostMode::HostConnectable;
    case ScanMode::SCAN_MODE_GENERAL_DISCOVERABLE:
    case ScanMode::SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE:
        return QBluetoothLocalDevice::HostMode::HostDiscoverable;
    case ScanMode::SCAN_MODE_LIMITED_DISCOVERABLE:
    case ScanMode::SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE:
        return QBluetoothLocalDevice::HostMode::HostDiscoverableLimitedInquiry;
    case ScanMode::SCAN_MODE_NONE:
        return QBluetoothLocalDevice::HostMode::HostPoweredOff;
    }

    qOhosReportFatalErrorAndAbort(
        "%s: unexpected ScanMode value %d", Q_FUNC_INFO, static_cast<int>(scanMode));
}

QtOhosBluetooth::QOhosBluetoothLocalDeviceProxy::ScanMode mapHostModeToOhosScanMode(
    QBluetoothLocalDevice::HostMode hostMode)
{
    using ScanMode = QtOhosBluetooth::QOhosBluetoothLocalDeviceProxy::ScanMode;

    switch (hostMode) {
    case QBluetoothLocalDevice::HostMode::HostPoweredOff:
        return ScanMode::SCAN_MODE_NONE;
    case QBluetoothLocalDevice::HostMode::HostConnectable:
        return ScanMode::SCAN_MODE_CONNECTABLE;
    case QBluetoothLocalDevice::HostMode::HostDiscoverable:
        return ScanMode::SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE;
    case QBluetoothLocalDevice::HostMode::HostDiscoverableLimitedInquiry:
        return ScanMode::SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE;
    }

    qOhosReportFatalErrorAndAbort(
        "%s: unexpected HostMode value %d", Q_FUNC_INFO, static_cast<int>(hostMode));
}

bool isAdapterStateSettled(QtOhosBluetooth::QOhosBluetoothAccessProxy::BluetoothState bluetoothState)
{
    using BluetoothState = QtOhosBluetooth::QOhosBluetoothAccessProxy::BluetoothState;

    switch (bluetoothState) {
    case BluetoothState::STATE_BLE_TURNING_OFF:
    case BluetoothState::STATE_BLE_TURNING_ON:
    case BluetoothState::STATE_TURNING_OFF:
    case BluetoothState::STATE_TURNING_ON:
        return false;
    case BluetoothState::STATE_BLE_ON:
    case BluetoothState::STATE_OFF:
    case BluetoothState::STATE_ON:
        return true;
    }

    qOhosReportFatalErrorAndAbort(
        "%s: unexpected BluetoothState value %d", Q_FUNC_INFO, static_cast<int>(bluetoothState));
}

QBluetoothLocalDevice::HostMode getBluetoothLocalDeviceHostMode(
    QtOhosBluetooth::QOhosBluetoothAccessProxy::BluetoothState bluetoothLocalAdapterState)
{
    if (bluetoothLocalAdapterState != QtOhosBluetooth::QOhosBluetoothAccessProxy::BluetoothState::STATE_ON)
        return QBluetoothLocalDevice::HostMode::HostPoweredOff;

    const auto optScanMode = QtOhosBluetooth::QOhosBluetoothLocalDeviceProxy::instance()->tryGetScanMode();
    return optScanMode
        ? mapScanModeToQtHostMode(*optScanMode)
        : QBluetoothLocalDevice::HostMode::HostPoweredOff;
}

enum class HostModeUpdateSource { ChangeEvent, LiveRead };

class QOhosBluetoothLocalDevicePrivate : public QBluetoothLocalDevicePrivate
{
public:
    QOhosBluetoothLocalDevicePrivate(QBluetoothLocalDevice *qBluetoothLocalDevice, const QBluetoothAddress &address);

    bool isValid() const override;
    std::optional<QtOhosBluetooth::QOhosBluetoothAccessProxy::BluetoothState> tryGetBluetoothState() const;
    void requestHostMode(QBluetoothLocalDevice::HostMode requestedHostMode);
    void requestPowerOn();
    void cancelHostModeRequest();
    void handleAdapterHostModeChanged(
        QBluetoothLocalDevice::HostMode hostMode,
        std::optional<QtOhosBluetooth::QOhosBluetoothAccessProxy::BluetoothState> optBluetoothState,
        HostModeUpdateSource source = HostModeUpdateSource::ChangeEvent);

private:
    struct HostModeRequest
    {
        QBluetoothLocalDevice::HostMode target = QBluetoothLocalDevice::HostMode::HostPoweredOff;
        bool anyPoweredOnAccepted = false;
        bool awaitingOutcome = false;
        bool powerSwitchIssued = false;
        bool scanModeIssued = false;
        QDeadlineTimer deadline;
    };

    void registerHostModeRequest(
        QBluetoothLocalDevice::HostMode target, bool anyPoweredOnAccepted);
    void issueHostModeRequest();
    void resolveHostModeRequestFromAdapterState();
    void scheduleHostModeRetry();
    void awaitHostModeRequestOutcome(bool powerSwitchIssued);
    void giveUpHostModeRequest();
    static bool isRequestSatisfiedBy(
        const HostModeRequest &request, QBluetoothLocalDevice::HostMode hostMode,
        bool adapterPoweredOn);
    static bool isPowerSwitchOutcomePending(
        const HostModeRequest &request, bool adapterPoweredOn);
    void emitAsyncError(QBluetoothLocalDevice::Error error);

    QBluetoothLocalDevice *m_qBluetoothLocalDevice;
    bool m_isValid = false;
    QBluetoothLocalDevice::HostMode m_lastHostMode =
        QBluetoothLocalDevice::HostMode::HostPoweredOff;
    std::optional<HostModeRequest> m_optHostModeRequest = std::nullopt;
    QTimer m_hostModeRequestTimer;
};

QOhosBluetoothLocalDevicePrivate::QOhosBluetoothLocalDevicePrivate(
    QBluetoothLocalDevice *qBluetoothLocalDevice, const QBluetoothAddress &address)
    : m_qBluetoothLocalDevice(qBluetoothLocalDevice)
{
    if (!address.isNull()) {
        qCWarning(
            QT_BT_OHOS, "%s: HarmonyOS does not support device adapter setting. Ignoring address %ls",
            Q_FUNC_INFO, qUtf16Printable(address.toString()));

        return;
    }

    m_isValid = true;

    if (const auto optBluetoothState = tryGetBluetoothState())
        m_lastHostMode = getBluetoothLocalDeviceHostMode(*optBluetoothState);

    QObject::connect(
        QtOhosBluetooth::QOhosBluetoothAccessProxy::instance().get(),
        &QtOhosBluetooth::QOhosBluetoothAccessProxy::stateChanged,
        this,
        [this](auto state) {
            handleAdapterHostModeChanged(getBluetoothLocalDeviceHostMode(state), state);
        });

    QObject::connect(
        QtOhosBluetooth::QOhosBluetoothAccessProxy::instance().get(),
        &QtOhosBluetooth::QOhosBluetoothAccessProxy::setBluetoothEnabledFailed,
        this,
        [this](QOhosBluetoothErrorCode errorCode) {
            if (!m_optHostModeRequest || !m_optHostModeRequest->powerSwitchIssued)
                return;

            qCWarning(
                QT_BT_OHOS, "%s: the bluetooth switch was not performed, error code: %u",
                Q_FUNC_INFO, qToUnderlying(errorCode));
            giveUpHostModeRequest();
        });

    QObject::connect(
        QtOhosBluetooth::QOhosBluetoothLocalDeviceProxy::instance().get(),
        &QtOhosBluetooth::QOhosBluetoothLocalDeviceProxy::scanModeChanged,
        this,
        [this](auto scanMode) {
            const auto optBluetoothState = tryGetBluetoothState();
            if (optBluetoothState != QtOhosBluetooth::QOhosBluetoothAccessProxy::BluetoothState::STATE_ON)
                return;

            handleAdapterHostModeChanged(mapScanModeToQtHostMode(scanMode), optBluetoothState);
        });

    auto emitMissingPermissionsError = [this]() {
        emitAsyncError(QBluetoothLocalDevice::MissingPermissionsError);
    };
    QObject::connect(
        QtOhosBluetooth::QOhosBluetoothAccessProxy::instance().get(),
        &QtOhosBluetooth::QOhosBluetoothAccessProxy::missingPermission,
        this, emitMissingPermissionsError);
    QObject::connect(
        QtOhosBluetooth::QOhosBluetoothLocalDeviceProxy::instance().get(),
        &QtOhosBluetooth::QOhosBluetoothLocalDeviceProxy::missingPermission,
        this, emitMissingPermissionsError);
    m_hostModeRequestTimer.setSingleShot(true);
    QObject::connect(
        &m_hostModeRequestTimer, &QTimer::timeout, this,
        [this]() {
            if (!m_optHostModeRequest)
                return;

            if (m_optHostModeRequest->awaitingOutcome
                || m_optHostModeRequest->deadline.hasExpired()) {
                resolveHostModeRequestFromAdapterState();

                if (m_optHostModeRequest && !m_hostModeRequestTimer.isActive())
                    giveUpHostModeRequest();

                return;
            }

            issueHostModeRequest();
        });
}

bool QOhosBluetoothLocalDevicePrivate::isValid() const
{
    return m_isValid;
}

std::optional<QtOhosBluetooth::QOhosBluetoothAccessProxy::BluetoothState> QOhosBluetoothLocalDevicePrivate::tryGetBluetoothState() const
{
    return QtOhosBluetooth::QOhosBluetoothAccessProxy::instance()->tryGetBluetoothState();
}

void QOhosBluetoothLocalDevicePrivate::requestHostMode(
    QBluetoothLocalDevice::HostMode requestedHostMode)
{
    registerHostModeRequest(requestedHostMode, false);
}

void QOhosBluetoothLocalDevicePrivate::requestPowerOn()
{
    registerHostModeRequest(QBluetoothLocalDevice::HostMode::HostConnectable, true);
}

void QOhosBluetoothLocalDevicePrivate::registerHostModeRequest(
    QBluetoothLocalDevice::HostMode target, bool anyPoweredOnAccepted)
{
    m_hostModeRequestTimer.stop();
    m_optHostModeRequest = HostModeRequest {
        .target = target,
        .anyPoweredOnAccepted = anyPoweredOnAccepted,
        .awaitingOutcome = false,
        .powerSwitchIssued = false,
        .scanModeIssued = false,
        .deadline = QDeadlineTimer(hostModeRequestGiveUpDelay),
    };

    issueHostModeRequest();
}

bool QOhosBluetoothLocalDevicePrivate::isRequestSatisfiedBy(
    const HostModeRequest &request, QBluetoothLocalDevice::HostMode hostMode,
    bool adapterPoweredOn)
{
    if (request.anyPoweredOnAccepted)
        return adapterPoweredOn && hostMode != QBluetoothLocalDevice::HostMode::HostPoweredOff;

    if (request.target == QBluetoothLocalDevice::HostMode::HostPoweredOff)
        return !adapterPoweredOn;

    if (request.target == QBluetoothLocalDevice::HostMode::HostDiscoverableLimitedInquiry)
        return hostMode == QBluetoothLocalDevice::HostMode::HostDiscoverableLimitedInquiry
            || hostMode == QBluetoothLocalDevice::HostMode::HostDiscoverable;

    return hostMode == request.target;
}

void QOhosBluetoothLocalDevicePrivate::resolveHostModeRequestFromAdapterState()
{
    const auto optBluetoothState = tryGetBluetoothState();
    if (!optBluetoothState)
        return;

    handleAdapterHostModeChanged(
        getBluetoothLocalDeviceHostMode(*optBluetoothState), optBluetoothState,
        HostModeUpdateSource::LiveRead);
}

void QOhosBluetoothLocalDevicePrivate::issueHostModeRequest()
{
    Q_ASSERT(m_optHostModeRequest);
    using BluetoothState = QtOhosBluetooth::QOhosBluetoothAccessProxy::BluetoothState;

    const auto target = m_optHostModeRequest->target;
    const auto optBluetoothState = tryGetBluetoothState();

    if (optBluetoothState && isAdapterStateSettled(*optBluetoothState)
        && isRequestSatisfiedBy(
            *m_optHostModeRequest, getBluetoothLocalDeviceHostMode(*optBluetoothState),
            *optBluetoothState == BluetoothState::STATE_ON)) {
        cancelHostModeRequest();
        return;
    }

    const auto handleOutcome =
        [this](std::optional<QOhosBluetoothErrorCode> optErrorCode, bool isPowerSwitchCall) {
            if (!optErrorCode) {
                if (isPowerSwitchCall) {
                    awaitHostModeRequestOutcome(true);
                    return;
                }

                m_optHostModeRequest->scanModeIssued = true;
                resolveHostModeRequestFromAdapterState();

                if (m_optHostModeRequest && !m_hostModeRequestTimer.isActive())
                    awaitHostModeRequestOutcome(false);

                return;
            }

            if (*optErrorCode == QOhosBluetoothErrorCode::OperationFailed
                || *optErrorCode == QOhosBluetoothErrorCode::BluetoothDisabled) {
                if (isPowerSwitchCall)
                    awaitHostModeRequestOutcome(true);
                else
                    scheduleHostModeRetry();

                return;
            }

            if (*optErrorCode == QOhosBluetoothErrorCode::PermissionDenied) {
                cancelHostModeRequest();
                return;
            }

            qCWarning(
                QT_BT_OHOS, "%s: the host mode request was rejected with error code: %u",
                Q_FUNC_INFO, qToUnderlying(*optErrorCode));
            giveUpHostModeRequest();
        };

    if (!optBluetoothState) {
        scheduleHostModeRetry();
        return;
    }

    if (!isAdapterStateSettled(*optBluetoothState)) {
        awaitHostModeRequestOutcome(false);
        return;
    }

    m_optHostModeRequest->scanModeIssued = false;

    if (target == QBluetoothLocalDevice::HostMode::HostPoweredOff) {
        Q_ASSERT(*optBluetoothState == BluetoothState::STATE_ON);
        handleOutcome(
            QtOhosBluetooth::QOhosBluetoothAccessProxy::instance()->trySetBluetoothEnabled(false),
            true);
        return;
    }

    if (*optBluetoothState != BluetoothState::STATE_ON) {
        handleOutcome(
            QtOhosBluetooth::QOhosBluetoothAccessProxy::instance()->trySetBluetoothEnabled(true),
            true);
        return;
    }

    handleOutcome(
        QtOhosBluetooth::QOhosBluetoothLocalDeviceProxy::instance()->trySetBluetoothScanMode(
            mapHostModeToOhosScanMode(target)),
        false);
}

void QOhosBluetoothLocalDevicePrivate::scheduleHostModeRetry()
{
    Q_ASSERT(m_optHostModeRequest);
    if (m_optHostModeRequest->deadline.hasExpired()) {
        giveUpHostModeRequest();
        return;
    }

    m_optHostModeRequest->awaitingOutcome = false;
    m_optHostModeRequest->powerSwitchIssued = false;
    m_hostModeRequestTimer.start(hostModeRequestRetryDelay);
}

bool QOhosBluetoothLocalDevicePrivate::isPowerSwitchOutcomePending(
    const HostModeRequest &request, bool adapterPoweredOn)
{
    if (!request.powerSwitchIssued)
        return false;

    return request.target == QBluetoothLocalDevice::HostMode::HostPoweredOff
        ? adapterPoweredOn
        : !adapterPoweredOn;
}

void QOhosBluetoothLocalDevicePrivate::awaitHostModeRequestOutcome(bool powerSwitchIssued)
{
    Q_ASSERT(m_optHostModeRequest);
    const auto remainingTime = m_optHostModeRequest->deadline.remainingTimeAsDuration();
    if (remainingTime <= std::chrono::milliseconds::zero()) {
        giveUpHostModeRequest();
        return;
    }

    m_optHostModeRequest->awaitingOutcome = true;
    m_optHostModeRequest->powerSwitchIssued = powerSwitchIssued;
    m_hostModeRequestTimer.start(
        std::chrono::ceil<std::chrono::milliseconds>(remainingTime));
}

void QOhosBluetoothLocalDevicePrivate::giveUpHostModeRequest()
{
    Q_ASSERT(m_optHostModeRequest);
    qCWarning(
        QT_BT_OHOS, "%s: giving up on the requested host mode %d, the adapter stays in %d",
        Q_FUNC_INFO, static_cast<int>(m_optHostModeRequest->target),
        static_cast<int>(m_lastHostMode));

    cancelHostModeRequest();

    emitAsyncError(QBluetoothLocalDevice::Error::UnknownError);
}

void QOhosBluetoothLocalDevicePrivate::cancelHostModeRequest()
{
    m_hostModeRequestTimer.stop();
    m_optHostModeRequest.reset();
}

void QOhosBluetoothLocalDevicePrivate::handleAdapterHostModeChanged(
    QBluetoothLocalDevice::HostMode hostMode,
    std::optional<QtOhosBluetooth::QOhosBluetoothAccessProxy::BluetoothState> optBluetoothState,
    HostModeUpdateSource source)
{
    const auto hostModeChanged = m_lastHostMode != hostMode;
    m_lastHostMode = hostMode;

    const auto adapterSettled = optBluetoothState && isAdapterStateSettled(*optBluetoothState);
    const auto adapterPoweredOn =
        optBluetoothState == QtOhosBluetooth::QOhosBluetoothAccessProxy::BluetoothState::STATE_ON;

    if (m_optHostModeRequest && adapterSettled) {
        const auto scanModeApplied = source == HostModeUpdateSource::LiveRead
            || m_optHostModeRequest->anyPoweredOnAccepted
            || m_optHostModeRequest->target == QBluetoothLocalDevice::HostMode::HostPoweredOff
            || m_optHostModeRequest->scanModeIssued;

        if (scanModeApplied
            && isRequestSatisfiedBy(*m_optHostModeRequest, hostMode, adapterPoweredOn)) {
            cancelHostModeRequest();
        } else if (!isPowerSwitchOutcomePending(*m_optHostModeRequest, adapterPoweredOn)) {
            if (m_optHostModeRequest->powerSwitchIssued && adapterPoweredOn)
                m_optHostModeRequest->deadline = QDeadlineTimer(hostModeRequestGiveUpDelay);

            scheduleHostModeRetry();
        }
    }

    if (hostModeChanged) {
        QMetaObject::invokeMethod(
            m_qBluetoothLocalDevice,
            [qBluetoothLocalDevice = m_qBluetoothLocalDevice, hostMode]() {
                Q_EMIT qBluetoothLocalDevice->hostModeStateChanged(hostMode);
            },
            Qt::QueuedConnection);
    }
}

void QOhosBluetoothLocalDevicePrivate::emitAsyncError(QBluetoothLocalDevice::Error error)
{
    QMetaObject::invokeMethod(
        m_qBluetoothLocalDevice,
        [this, error]() {
            Q_EMIT m_qBluetoothLocalDevice->errorOccurred(error);
        },
        Qt::QueuedConnection);
}

}

QBluetoothLocalDevicePrivate::QBluetoothLocalDevicePrivate() = default;

QBluetoothLocalDevicePrivate::~QBluetoothLocalDevicePrivate() = default;

QBluetoothLocalDevice::QBluetoothLocalDevice(QObject *parent)
    : QBluetoothLocalDevice(QBluetoothAddress(), parent)
{
}

QBluetoothLocalDevice::QBluetoothLocalDevice(const QBluetoothAddress &address, QObject *parent)
    : QObject(parent)
    , d_ptr(new QOhosBluetoothLocalDevicePrivate(this, address))
{
    registerQBluetoothLocalDeviceMetaType();
}

QString QBluetoothLocalDevice::name() const
{
    if (!d_ptr->isValid()) {
        qCWarning(
            QT_BT_OHOS, "%s: local device is not valid. Can't get local device name.", Q_FUNC_INFO);
        return QString();
    }

    const auto optLocalDeviceName = QtOhosBluetooth::QOhosBluetoothLocalDeviceProxy::instance()->tryGetLocalDeviceName();

    return optLocalDeviceName
        ? *optLocalDeviceName
        : QString();
}

QBluetoothAddress QBluetoothLocalDevice::address() const
{
    return {};
}

void QBluetoothLocalDevice::powerOn()
{
    if (!d_ptr->isValid()) {
        qCWarning(QT_BT_OHOS, "%s: local device is not valid. Ignoring...", Q_FUNC_INFO);
        return;
    }

    if (hostMode() != HostMode::HostPoweredOff) {
        qCWarning(QT_BT_OHOS, "%s: bluetooth is not powered off. Ignoring...", Q_FUNC_INFO);
        return;
    }

    static_cast<QOhosBluetoothLocalDevicePrivate *>(d_ptr)->requestPowerOn();
}

void QBluetoothLocalDevice::setHostMode(QBluetoothLocalDevice::HostMode mode)
{
    if (!d_ptr->isValid()) {
        qCWarning(QT_BT_OHOS, "%s: local device is not valid. Ignoring...", Q_FUNC_INFO);
        return;
    }

    auto *ohosBluetoothLocalDevicePriv = static_cast<QOhosBluetoothLocalDevicePrivate *>(d_ptr);

    const auto optBluetoothState = ohosBluetoothLocalDevicePriv->tryGetBluetoothState();
    const auto adapterSettled = optBluetoothState && isAdapterStateSettled(*optBluetoothState);

    if (adapterSettled && mode == QBluetoothLocalDevice::HostMode::HostPoweredOff
        && *optBluetoothState != QtOhosBluetooth::QOhosBluetoothAccessProxy::BluetoothState::STATE_ON) {
        qCWarning(
            QT_BT_OHOS,
            "%s: Bluetooth is not in powered on state. Cancelling the pending host mode request.",
            Q_FUNC_INFO);
        ohosBluetoothLocalDevicePriv->cancelHostModeRequest();
        return;
    }

    ohosBluetoothLocalDevicePriv->requestHostMode(mode);
}

QBluetoothLocalDevice::HostMode QBluetoothLocalDevice::hostMode() const
{
    if (!d_ptr->isValid()) {
        qCWarning(
            QT_BT_OHOS, "%s: local device is not valid. Reporting powered off host mode.",
            Q_FUNC_INFO);
        return QBluetoothLocalDevice::HostPoweredOff;
    }

    const auto optBluetoothState = static_cast<QOhosBluetoothLocalDevicePrivate *>(d_ptr)->tryGetBluetoothState();
    if (!optBluetoothState)
        return QBluetoothLocalDevice::HostPoweredOff;

    return getBluetoothLocalDeviceHostMode(*optBluetoothState);
}

QList<QBluetoothAddress> QBluetoothLocalDevice::connectedDevices() const
{
    return {};
}

QList<QBluetoothHostInfo> QBluetoothLocalDevice::allDevices()
{
    const auto optLocalDeviceName =
        QtOhosBluetooth::QOhosBluetoothLocalDeviceProxy::instance()->tryGetLocalDeviceName();
    if (!optLocalDeviceName)
        return {};

    QBluetoothHostInfo bluetoothHostInfo;
    bluetoothHostInfo.setName(*optLocalDeviceName);
    return { bluetoothHostInfo };
}

void QBluetoothLocalDevice::requestPairing(const QBluetoothAddress &, Pairing)
{
}

QBluetoothLocalDevice::Pairing QBluetoothLocalDevice::pairingStatus(const QBluetoothAddress &) const
{
    return Unpaired;
}

QT_END_NAMESPACE

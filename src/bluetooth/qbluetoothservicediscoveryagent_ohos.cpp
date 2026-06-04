// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothservicediscoveryagent_p.h"

#include <QtBluetooth/qbluetoothaddress.h>
#include <QtBluetooth/qbluetoothdeviceinfo.h>
#include <QtBluetooth/qbluetoothserviceinfo.h>
#include <QtBluetooth/qbluetoothservicediscoveryagent.h>
#include <QtBluetooth/qbluetoothuuid.h>
#include <QtBluetooth/private/qohosbluetoothaccess_p.h>
#include <QtBluetooth/private/qohosbluetoothremotedevice_p.h>

#include <QtCore/qloggingcategory.h>
#include <QtCore/qvariant.h>

#include <algorithm>
#include <optional>

QT_BEGIN_NAMESPACE

using QtOhosBluetooth::QOhosBluetoothErrorCode;

Q_DECLARE_LOGGING_CATEGORY(QT_BT_OHOS)

namespace {

bool uuidFilteredOut(const QList<QBluetoothUuid> &uuidFilter, const QBluetoothUuid &uuid)
{
    return !uuidFilter.isEmpty() && !uuidFilter.contains(uuid);
}

QBluetoothServiceInfo makeRfcommServiceInfo(
    const QBluetoothDeviceInfo &deviceInfo, const QBluetoothUuid &serviceUuid)
{
    constexpr quint8 unknownRfcommServerChannel = 0;
    constexpr quint16 serialPortProfileVersion = 0x0100;

    QBluetoothServiceInfo serviceInfo;
    serviceInfo.setDevice(deviceInfo);

    QBluetoothServiceInfo::Sequence l2capProtocol;
    l2capProtocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::L2cap));

    QBluetoothServiceInfo::Sequence rfcommProtocol;
    rfcommProtocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::Rfcomm));
    rfcommProtocol << QVariant::fromValue(unknownRfcommServerChannel);

    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    protocolDescriptorList << QVariant::fromValue(l2capProtocol);
    protocolDescriptorList << QVariant::fromValue(rfcommProtocol);
    serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);

    QBluetoothServiceInfo::Sequence serialPortProfile;
    QBluetoothServiceInfo::Sequence serialPortProfileClassId;
    serialPortProfileClassId
        << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));
    serialPortProfileClassId << QVariant::fromValue(serialPortProfileVersion);
    serialPortProfile << QVariant::fromValue(serialPortProfileClassId);
    serviceInfo.setAttribute(
        QBluetoothServiceInfo::BluetoothProfileDescriptorList, serialPortProfile);

    QBluetoothServiceInfo::Sequence browseGroup;
    browseGroup << QVariant::fromValue(
        QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::PublicBrowseGroup));
    serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList, browseGroup);

    QBluetoothServiceInfo::Sequence serviceClassIds;
    serviceClassIds << QVariant::fromValue(serviceUuid);
    serviceClassIds << QVariant::fromValue(
        QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, serviceClassIds);

    serviceInfo.setServiceUuid(serviceUuid);
    serviceInfo.setServiceProvider(QBluetoothServiceDiscoveryAgent::tr("QtOhosBluetooth"));
    serviceInfo.setServiceName(QBluetoothServiceDiscoveryAgent::tr("Serial Port Profile"));

    return serviceInfo;
}

}

QBluetoothServiceDiscoveryAgentPrivate::QBluetoothServiceDiscoveryAgentPrivate(
    QBluetoothServiceDiscoveryAgent *qp, const QBluetoothAddress &)
    : error(QBluetoothServiceDiscoveryAgent::NoError)
    , state(Inactive)
    , mode(QBluetoothServiceDiscoveryAgent::MinimalDiscovery)
    , singleDevice(false)
    , m_remoteDeviceProxy(QtOhosBluetooth::QOhosBluetoothRemoteDeviceProxy::instance())
    , q_ptr(qp)
{
    QObject::connect(
        m_remoteDeviceProxy.get(),
        &QtOhosBluetooth::QOhosBluetoothRemoteDeviceProxy::remoteDeviceServicesRead,
        q_ptr,
        [this](const QtOhosBluetooth::RemoteDeviceServices &remoteDeviceServices) {
            reportRemoteDeviceServices(remoteDeviceServices);
        });
}

QBluetoothServiceDiscoveryAgentPrivate::~QBluetoothServiceDiscoveryAgentPrivate() = default;

void QBluetoothServiceDiscoveryAgentPrivate::reportErrorAsync(
    QBluetoothServiceDiscoveryAgent::Error discoveryError, const QString &discoveryErrorString)
{
    error = discoveryError;
    errorString = discoveryErrorString;
    QMetaObject::invokeMethod(
        q_ptr,
        [this, discoveryError]() {
            Q_EMIT q_ptr->errorOccurred(discoveryError);
        },
        Qt::QueuedConnection);
}

void QBluetoothServiceDiscoveryAgentPrivate::finishDeviceDiscoveryAsync()
{
    QMetaObject::invokeMethod(
        q_ptr,
        [this]() {
            if (discoveryState() != ServiceDiscovery)
                return;

            _q_serviceDiscoveryFinished();
        },
        Qt::QueuedConnection);
}

void QBluetoothServiceDiscoveryAgentPrivate::reportRemoteDeviceServices(
    const QtOhosBluetooth::RemoteDeviceServices &remoteDeviceServices)
{
    if (m_optRequestedDeviceId != remoteDeviceServices.deviceId)
        return;

    m_optRequestedDeviceId.reset();

    const auto addressStr = QString::fromStdString(remoteDeviceServices.deviceId);
    const QBluetoothDeviceInfo bluetoothDeviceInfo(
        QBluetoothAddress(addressStr),
        remoteDeviceServices.optDeviceName
            ? QString::fromStdString(*remoteDeviceServices.optDeviceName)
            : QString(),
        remoteDeviceServices.optClassOfDevice.value_or(0));

    const auto discoveredServicesCountBeforeDevice = discoveredServices.size();

    auto reportServiceForUuid = [&](const QBluetoothUuid &uuid) {
        const auto serviceInfo = makeRfcommServiceInfo(bluetoothDeviceInfo, uuid);
        if (isDuplicatedService(serviceInfo))
            return;

        discoveredServices.append(serviceInfo);
        QMetaObject::invokeMethod(
            q_ptr,
            [this, serviceInfo]() {
                Q_EMIT q_ptr->serviceDiscovered(serviceInfo);
            },
            Qt::QueuedConnection);
    };

    if (remoteDeviceServices.optProfileUuids) {
        for (const auto &profileUuid : *remoteDeviceServices.optProfileUuids) {
            const auto uuid = QBluetoothUuid(QString::fromStdString(profileUuid));
            if (uuidFilteredOut(uuidFilter, uuid))
                continue;

            reportServiceForUuid(uuid);
        }
    } else {
        qCWarning(
            QT_BT_OHOS,
            "%s: could not read the remote standard profile UUIDs of the device %ls. Continuing...",
            Q_FUNC_INFO, qUtf16Printable(addressStr));
    }

    if (singleDevice && !uuidFilter.isEmpty() && !remoteDeviceServices.lowEnergyOnly) {
        for (const auto &requestedUuid : uuidFilter)
            reportServiceForUuid(requestedUuid);
    }

    if (!remoteDeviceServices.optProfileUuids && singleDevice
            && discoveredServices.size() == discoveredServicesCountBeforeDevice) {
        reportErrorAsync(
            QBluetoothServiceDiscoveryAgent::InputOutputError,
            QBluetoothServiceDiscoveryAgent::tr("Cannot obtain service uuids"));
    }

    finishDeviceDiscoveryAsync();
}

void QBluetoothServiceDiscoveryAgentPrivate::start(const QBluetoothAddress &address)
{
    auto accessProxy = QtOhosBluetooth::QOhosBluetoothAccessProxy::instance();
    if (accessProxy->tryGetBluetoothState() != QtOhosBluetooth::QOhosBluetoothAccessProxy::BluetoothState::STATE_ON) {
        qCWarning(QT_BT_OHOS, "%s: bluetooth not in powered on state. Ignoring...", Q_FUNC_INFO);
        discoveredDevices.clear();
        reportErrorAsync(
            QBluetoothServiceDiscoveryAgent::PoweredOffError,
            QBluetoothServiceDiscoveryAgent::tr("Device is powered off"));
        finishDeviceDiscoveryAsync();
        return;
    }

    m_optRequestedDeviceId = address.toString().toStdString();

    const auto optErrorCode = m_remoteDeviceProxy->requestRemoteDeviceServices(address.toString());
    if (!optErrorCode)
        return;

    m_optRequestedDeviceId.reset();

    qCWarning(
        QT_BT_OHOS, "%s: could not start reading the services of the device %ls, error code: %u",
        Q_FUNC_INFO, qUtf16Printable(address.toString()), qToUnderlying(*optErrorCode));

    if (singleDevice) {
        if (*optErrorCode == QOhosBluetoothErrorCode::PermissionDenied) {
            reportErrorAsync(
                QBluetoothServiceDiscoveryAgent::MissingPermissionsError,
                QBluetoothServiceDiscoveryAgent::tr("Missing bluetooth permission"));
        } else {
            reportErrorAsync(
                QBluetoothServiceDiscoveryAgent::InputOutputError,
                QBluetoothServiceDiscoveryAgent::tr("Cannot obtain service uuids"));
        }
    }

    finishDeviceDiscoveryAsync();
}

void QBluetoothServiceDiscoveryAgentPrivate::stop()
{
    m_optRequestedDeviceId.reset();
    QMetaObject::invokeMethod(q_ptr, &QBluetoothServiceDiscoveryAgent::canceled, Qt::QueuedConnection);
}

QT_END_NAMESPACE

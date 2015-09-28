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
#include "qlowenergycontroller_p.h"
#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

QLowEnergyControllerPrivate::QLowEnergyControllerPrivate()
    : QObject(),
      state(QLowEnergyController::UnconnectedState),
      error(QLowEnergyController::NoError),
      hRemoteDevice(INVALID_HANDLE_VALUE),
      primaryServicesDiscoveryWatcher(0)
{
}

QLowEnergyControllerPrivate::~QLowEnergyControllerPrivate()
{
}

void QLowEnergyControllerPrivate::connectToDevice()
{
    Q_Q(QLowEnergyController);

    // required to pass unit test on default backend
    if (remoteDevice.isNull()) {
        qWarning() << "Invalid/null remote device address";
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        return;
    }

    if (!WinLowEnergyBluetooth::isSupported()) {
        qWarning() << "Low energy is not supported by OS";
        setError(QLowEnergyController::UnknownError);
        return;
    }

    if (isConnected()) {
        qCDebug(QT_BT_WINDOWS) << "Already is connected";
        return;
    }

    setState(QLowEnergyController::ConnectingState);

    const WinLowEnergyBluetooth::DeviceDiscoveryResult result =
            WinLowEnergyBluetooth::startDiscoveryOfRemoteDevices();

    if (result.error != NO_ERROR
            && result.error != ERROR_NO_MORE_ITEMS) {
        qCWarning(QT_BT_WINDOWS) << qt_error_string(::GetLastError());
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        setState(QLowEnergyController::UnconnectedState);
        return;
    }

    foreach (const WinLowEnergyBluetooth::DeviceInfo &info, result.devices) {

        if (info.address != remoteDevice)
            continue;

        const DWORD desiredAccess = GENERIC_READ | GENERIC_WRITE;
        const DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;

        hRemoteDevice = ::CreateFile(
                    reinterpret_cast<const wchar_t *>(info.systemPath.utf16()),
                    desiredAccess,
                    shareMode,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

        if (hRemoteDevice == INVALID_HANDLE_VALUE) {
            qCWarning(QT_BT_WINDOWS) << qt_error_string(::GetLastError());
            setError(QLowEnergyController::ConnectionError);
            setState(QLowEnergyController::UnconnectedState);
            return;
        }

        setState(QLowEnergyController::ConnectedState);
        emit q->connected();
        return;
    }

    qCWarning(QT_BT_WINDOWS) << qt_error_string(ERROR_FILE_NOT_FOUND);
    setError(QLowEnergyController::UnknownRemoteDeviceError);
    setState(QLowEnergyController::UnconnectedState);
}

void QLowEnergyControllerPrivate::disconnectFromDevice()
{
    Q_Q(QLowEnergyController);

    if (!WinLowEnergyBluetooth::isSupported()) {
        qWarning() << "Low energy is not supported by OS";
        setError(QLowEnergyController::UnknownError);
        return;
    }

    if (!isConnected()) {
        qCDebug(QT_BT_WINDOWS) << "Already is disconnected";
        return;
    }

    setState(QLowEnergyController::ClosingState);

    if (!::CloseHandle(hRemoteDevice))
        qCWarning(QT_BT_WINDOWS) << qt_error_string(::GetLastError());

    hRemoteDevice = INVALID_HANDLE_VALUE;
    setState(QLowEnergyController::UnconnectedState);
    emit q->disconnected();
}

void QLowEnergyControllerPrivate::discoverServices()
{
    if (!WinLowEnergyBluetooth::isSupported()) {
        qWarning() << "Low energy is not supported by OS";
        setError(QLowEnergyController::UnknownError);
        return;
    }

    startDiscoveryOfPrimaryServices();
}

void QLowEnergyControllerPrivate::discoverServiceDetails(
        const QBluetoothUuid &/*service*/)
{

}

void QLowEnergyControllerPrivate::readCharacteristic(
        const QSharedPointer<QLowEnergyServicePrivate> /*service*/,
        const QLowEnergyHandle /*charHandle*/)
{

}

void QLowEnergyControllerPrivate::writeCharacteristic(
        const QSharedPointer<QLowEnergyServicePrivate> /*service*/,
        const QLowEnergyHandle /*charHandle*/,
        const QByteArray &/*newValue*/,
        bool /*writeWithResponse*/)
{

}

void QLowEnergyControllerPrivate::readDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate> /*service*/,
        const QLowEnergyHandle /*charHandle*/,
        const QLowEnergyHandle /*descriptorHandle*/)
{

}

void QLowEnergyControllerPrivate::writeDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate> /*service*/,
        const QLowEnergyHandle /*charHandle*/,
        const QLowEnergyHandle /*descriptorHandle*/,
        const QByteArray &/*newValue*/)
{

}

void QLowEnergyControllerPrivate::primaryServicesDiscoveryCompleted()
{
    Q_Q(QLowEnergyController);

    const WinLowEnergyBluetooth::ServicesDiscoveryResult result =
            primaryServicesDiscoveryWatcher->result();

    if (result.error != NO_ERROR) {
        qCWarning(QT_BT_WINDOWS) << qt_error_string(result.error);
        setError(QLowEnergyController::UnknownError);
        return;
    }

    foreach (const WinLowEnergyBluetooth::BTH_LE_GATT_SERVICE &service,
             result.services) {

        const QBluetoothUuid uuid(
                    service.ServiceUuid.IsShortUuid
                    ? QBluetoothUuid(service.ServiceUuid.Value.ShortUuid)
                    : QBluetoothUuid(service.ServiceUuid.Value.LongUuid));

        qCDebug(QT_BT_WINDOWS) << "Found uuid:" << uuid;

        QLowEnergyServicePrivate *priv = new QLowEnergyServicePrivate();
        priv->uuid = uuid;
        priv->type = QLowEnergyService::PrimaryService;
        priv->startHandle = service.AttributeHandle;
        priv->setController(this);

        QSharedPointer<QLowEnergyServicePrivate> pointer(priv);

        serviceList.insert(uuid, pointer);
        emit q->serviceDiscovered(uuid);
    }

    setState(QLowEnergyController::DiscoveredState);
    emit q->discoveryFinished();
}

void QLowEnergyControllerPrivate::startDiscoveryOfPrimaryServices()
{
    if (!primaryServicesDiscoveryWatcher) {
        primaryServicesDiscoveryWatcher = new QFutureWatcher<
                WinLowEnergyBluetooth::ServicesDiscoveryResult>(this);

        connect(primaryServicesDiscoveryWatcher, &QFutureWatcher<WinLowEnergyBluetooth::ServicesDiscoveryResult>::finished,
                this, &QLowEnergyControllerPrivate::primaryServicesDiscoveryCompleted);
    }

    if (primaryServicesDiscoveryWatcher->isRunning())
        return;

    const QFuture<WinLowEnergyBluetooth::ServicesDiscoveryResult> future =
            QtConcurrent::run(
                WinLowEnergyBluetooth::startDiscoveryOfPrimaryServices,
                hRemoteDevice);

    primaryServicesDiscoveryWatcher->setFuture(future);
}

bool QLowEnergyControllerPrivate::isConnected() const
{
    return hRemoteDevice && (hRemoteDevice != INVALID_HANDLE_VALUE);
}

QT_END_NAMESPACE

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
#include "qbluetoothdevicediscoveryagent_p.h"

#include <QtCore/QLoggingCategory>

#include <windows/qwinlowenergybluetooth_p.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

Q_GLOBAL_STATIC(QLibrary, bluetoothapis)

static bool gattFunctionsResolved = false;

static HANDLE openSystemDevice(const QString &systemPath, int *systemErrorCode)
{
    const DWORD desiredAccess = GENERIC_READ | GENERIC_WRITE;
    const DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    const HANDLE serviceHandle = ::CreateFile(
                reinterpret_cast<const wchar_t *>(systemPath.utf16()),
                desiredAccess,
                shareMode,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

    *systemErrorCode = (INVALID_HANDLE_VALUE == serviceHandle)
            ? ::GetLastError() : NO_ERROR;
    return serviceHandle;
}

static void closeSystemDevice(HANDLE deviceHandle)
{
    if (deviceHandle && deviceHandle != INVALID_HANDLE_VALUE)
        ::CloseHandle(deviceHandle);
}

static QVector<BTH_LE_GATT_SERVICE> enumeratePrimaryGattServices(
        HANDLE deviceHandle, int *systemErrorCode)
{
    if (!gattFunctionsResolved) {
        *systemErrorCode = ERROR_NOT_SUPPORTED;
        return QVector<BTH_LE_GATT_SERVICE>();
    }

    QVector<BTH_LE_GATT_SERVICE> foundServices;
    USHORT servicesCount = 0;
    forever {
        const HRESULT hr = ::BluetoothGATTGetServices(
                    deviceHandle,
                    servicesCount,
                    foundServices.isEmpty() ? NULL : &foundServices[0],
                    &servicesCount,
                    BLUETOOTH_GATT_FLAG_NONE);

        if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
            foundServices.resize(servicesCount);
        } else if (hr == S_OK) {
            *systemErrorCode = NO_ERROR;
            return foundServices;
        } else {
            *systemErrorCode = ::GetLastError();
            return QVector<BTH_LE_GATT_SERVICE>();
        }
    }
}

QLowEnergyControllerPrivate::QLowEnergyControllerPrivate()
    : QObject()
    , state(QLowEnergyController::UnconnectedState)
    , error(QLowEnergyController::NoError)
{
    gattFunctionsResolved = resolveFunctions(bluetoothapis());
    if (!gattFunctionsResolved) {
        qCWarning(QT_BT_WINDOWS) << "LE is not supported on this OS";
        return;
    }
}

QLowEnergyControllerPrivate::~QLowEnergyControllerPrivate()
{
}

void QLowEnergyControllerPrivate::connectToDevice()
{
    // required to pass unit test on default backend
    if (remoteDevice.isNull()) {
        qWarning() << "Invalid/null remote device address";
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        return;
    }

    if (!deviceSystemPath.isEmpty()) {
        qCDebug(QT_BT_WINDOWS) << "Already is connected";
        return;
    }

    setState(QLowEnergyController::ConnectingState);

    deviceSystemPath =
            QBluetoothDeviceDiscoveryAgentPrivate::discoveredLeDeviceSystemPath(
                remoteDevice);

    if (deviceSystemPath.isEmpty()) {
        qCWarning(QT_BT_WINDOWS) << qt_error_string(ERROR_PATH_NOT_FOUND);
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        setState(QLowEnergyController::UnconnectedState);
        return;
    }

    setState(QLowEnergyController::ConnectedState);

    Q_Q(QLowEnergyController);
    emit q->connected();
}

void QLowEnergyControllerPrivate::disconnectFromDevice()
{
    if (deviceSystemPath.isEmpty()) {
        qCDebug(QT_BT_WINDOWS) << "Already is disconnected";
        return;
    }

    setState(QLowEnergyController::ClosingState);
    deviceSystemPath.clear();
    setState(QLowEnergyController::UnconnectedState);

    Q_Q(QLowEnergyController);
    emit q->disconnected();
}

void QLowEnergyControllerPrivate::discoverServices()
{
    int systemErrorCode = NO_ERROR;

    const HANDLE deviceHandle = openSystemDevice(deviceSystemPath, &systemErrorCode);

    if (systemErrorCode != NO_ERROR) {
        qCWarning(QT_BT_WINDOWS) << qt_error_string(systemErrorCode);
        setError(QLowEnergyController::NetworkError);
        setState(QLowEnergyController::ConnectedState);
        return;
    }

    const QVector<BTH_LE_GATT_SERVICE> foundServices =
            enumeratePrimaryGattServices(deviceHandle, &systemErrorCode);

    closeSystemDevice(deviceHandle);

    if (systemErrorCode != NO_ERROR) {
        qCWarning(QT_BT_WINDOWS) << qt_error_string(systemErrorCode);
        setError(QLowEnergyController::NetworkError);
        setState(QLowEnergyController::ConnectedState);
        return;
    }

    setState(QLowEnergyController::DiscoveringState);

    Q_Q(QLowEnergyController);

    foreach (const BTH_LE_GATT_SERVICE &service, foundServices) {

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

QT_END_NAMESPACE

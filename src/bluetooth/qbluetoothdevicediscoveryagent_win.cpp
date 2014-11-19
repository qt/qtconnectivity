/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Copyright (C) 2014 Denis Shienkov <denis.shienkov@gmail.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothdevicediscoveryagent_p.h"
#include "qbluetoothaddress.h"
#include "qbluetoothuuid.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(
        const QBluetoothAddress &deviceAdapter,
        QBluetoothDeviceDiscoveryAgent *parent)
    : inquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry)
    , lastError(QBluetoothDeviceDiscoveryAgent::NoError)
    , classicDiscoveryWatcher(0)
    , pendingCancel(false)
    , pendingStart(false)
    , isClassicActive(false)
    , isClassicValid(false)
    , q_ptr(parent)
{
    initialize(deviceAdapter);
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
    if (isClassicActive) {
        stop();
        classicDiscoveryWatcher->waitForFinished();
    }
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    if (pendingStart)
        return true;
    if (pendingCancel)
        return false;

    return isClassicActive;
}

void QBluetoothDeviceDiscoveryAgentPrivate::start()
{
    if (!isClassicValid) {
        setError(ERROR_INVALID_HANDLE,
                 QBluetoothDeviceDiscoveryAgent::tr("Passed address is not a local device."));
        return;
    }

    if (pendingCancel == true) {
        pendingStart = true;
        return;
    }

    discoveredDevices.clear();

    startDiscoveryForFirstClassicDevice();
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    if (!isClassicActive)
        return;

    pendingCancel = true;
    pendingStart = false;
}

void QBluetoothDeviceDiscoveryAgentPrivate::classicDeviceDiscovered()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    const WinClassicBluetooth::RemoteDeviceDiscoveryResult result =
            classicDiscoveryWatcher->result();

    if (result.error == ERROR_SUCCESS
            || result.error == ERROR_NO_MORE_ITEMS) {

        if (pendingCancel && !pendingStart) {
            emit q->canceled();
            pendingCancel = false;
        } else if (pendingStart) {
            pendingCancel = false;
            pendingStart = false;
            start();
        } else {
            if (result.error != ERROR_NO_MORE_ITEMS) {
                acceptDiscoveredClassicDevice(result.device);
                startDiscoveryForNextClassicDevice(result.hSearch);
                return;
            }
        }

    } else {
        setError(result.error);
        pendingCancel = false;
        pendingStart = false;
    }

    completeClassicDiscovery(result.hSearch);
}

void QBluetoothDeviceDiscoveryAgentPrivate::initialize(
        const QBluetoothAddress &deviceAdapter)
{
    isClassicValid = isClassicAdapterValid(deviceAdapter);
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isClassicAdapterValid(
        const QBluetoothAddress &deviceAdapter)
{
    const WinClassicBluetooth::LocalRadiosDiscoveryResult result =
            WinClassicBluetooth::enumerateLocalRadios();

    if (result.error != NO_ERROR
            && result.error != ERROR_NO_MORE_ITEMS) {
        qCWarning(QT_BT_WINDOWS) << "Occurred error during search of classic local radios";
        return false;
    } else if (result.radios.isEmpty()) {
        qCWarning(QT_BT_WINDOWS) << "No any classic local radio found";
        return false;
    }

    foreach (const BLUETOOTH_RADIO_INFO &radio, result.radios) {
        if (deviceAdapter == QBluetoothAddress()
             || deviceAdapter == QBluetoothAddress(radio.address.ullLong)) {
            return true;
        }
    }

    qCWarning(QT_BT_WINDOWS) << "No matching for classic local radio:" << deviceAdapter;
    return false;
}

void QBluetoothDeviceDiscoveryAgentPrivate::startDiscoveryForFirstClassicDevice()
{
    isClassicActive = true;

    if (!classicDiscoveryWatcher) {
        classicDiscoveryWatcher = new QFutureWatcher<
                WinClassicBluetooth::RemoteDeviceDiscoveryResult>(this);
        QObject::connect(classicDiscoveryWatcher, SIGNAL(finished()),
                         this, SLOT(classicDeviceDiscovered()));
    }

    const QFuture<WinClassicBluetooth::RemoteDeviceDiscoveryResult> future =
            QtConcurrent::run(WinClassicBluetooth::startDiscoveryOfFirstRemoteDevice);
    classicDiscoveryWatcher->setFuture(future);
}

void QBluetoothDeviceDiscoveryAgentPrivate::startDiscoveryForNextClassicDevice(
        HBLUETOOTH_DEVICE_FIND hSearch)
{
    Q_ASSERT(classicDiscoveryWatcher);

    const QFuture<WinClassicBluetooth::RemoteDeviceDiscoveryResult> future =
            QtConcurrent::run(WinClassicBluetooth::startDiscoveryOfNextRemoteDevice, hSearch);
    classicDiscoveryWatcher->setFuture(future);
}

void QBluetoothDeviceDiscoveryAgentPrivate::completeClassicDiscovery(
        HBLUETOOTH_DEVICE_FIND hSearch)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    WinClassicBluetooth::cancelRemoteDevicesDiscovery(hSearch);
    isClassicActive = false;
    emit q->finished();
}

void QBluetoothDeviceDiscoveryAgentPrivate::acceptDiscoveredClassicDevice(
        const BLUETOOTH_DEVICE_INFO &device)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    QBluetoothDeviceInfo deviceInfo(
                QBluetoothAddress(device.Address.ullLong),
                QString::fromWCharArray(device.szName),
                device.ulClassofDevice);

    if (device.fRemembered)
        deviceInfo.setCached(true);

    discoveredDevices.append(deviceInfo);
    emit q->deviceDiscovered(deviceInfo);
}

void QBluetoothDeviceDiscoveryAgentPrivate::setError(DWORD error, const QString &str)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    lastError = (error == ERROR_INVALID_HANDLE) ?
                QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError
              : QBluetoothDeviceDiscoveryAgent::InputOutputError;
    errorString = str.isEmpty() ? qt_error_string(error) : str;
    emit q->error(lastError);
}

QT_END_NAMESPACE

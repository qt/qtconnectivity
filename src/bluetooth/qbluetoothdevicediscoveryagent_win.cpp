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

#define QT_DEVICEDISCOVERY_DEBUG

QT_BEGIN_NAMESPACE

struct NativeFindResult
{
    NativeFindResult();

    BLUETOOTH_DEVICE_INFO deviceInfo;
    HBLUETOOTH_DEVICE_FIND findHandle;
    DWORD errorCode;
};

NativeFindResult::NativeFindResult()
    : findHandle(NULL)
    , errorCode(ERROR_SUCCESS)
{
    ::ZeroMemory(&deviceInfo, sizeof(deviceInfo));
    deviceInfo.dwSize = sizeof(deviceInfo);
}

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(
        const QBluetoothAddress &deviceAdapter,
        QBluetoothDeviceDiscoveryAgent *parent)
    : QBluetoothLocalDevicePrivateData(deviceAdapter)
    , inquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry)
    , lastError(QBluetoothDeviceDiscoveryAgent::NoError)
    , findWatcher(0)
    , pendingCancel(false)
    , pendingStart(false)
    , q_ptr(parent)
{
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
    if (isRunning()) {
        stop();
        findWatcher->waitForFinished();
    }
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    if (pendingStart)
        return true;
    if (pendingCancel)
        return false;

    return isRunning();
}

void QBluetoothDeviceDiscoveryAgentPrivate::start()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    if (pendingCancel == true) {
        pendingStart = true;
        return;
    }

    discoveredDevices.clear();

    if (isValid() == false) {
        lastError = QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError;
        errorString = qt_error_string(ERROR_INVALID_HANDLE);
        emit q->error(lastError);
        return;
    }

    if (!findWatcher) {
        findWatcher = new QFutureWatcher<QVariant>(q);
        QObject::connect(findWatcher, SIGNAL(finished()), q, SLOT(_q_handleFindResult()));
    }

    const QFuture<QVariant> future = QtConcurrent::run(findFirstDevice, deviceHandle);
    findWatcher->setFuture(future);
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    if (!isRunning())
        return;

    pendingCancel = true;
    pendingStart = false;
}

void QBluetoothDeviceDiscoveryAgentPrivate::_q_handleFindResult()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    const QVariant result = findWatcher->result();
    const NativeFindResult nativeFindResult = result.value<NativeFindResult>();

    if (nativeFindResult.errorCode == ERROR_SUCCESS
            || nativeFindResult.errorCode == ERROR_NO_MORE_ITEMS) {

        if (pendingCancel && !pendingStart) {
            emit q->canceled();
            pendingCancel = false;
        } else if (pendingStart) {
            pendingCancel = false;
            pendingStart = false;
            start();
        } else {
            if (nativeFindResult.errorCode == ERROR_NO_MORE_ITEMS) {
                emit q->finished();
            } else {
                processDiscoveredDevices(nativeFindResult.deviceInfo);
                const QFuture<QVariant> future =
                        QtConcurrent::run(findNextDevice, nativeFindResult.findHandle);
                findWatcher->setFuture(future);
                return;
            }
        }

    } else {
        handleErrors(nativeFindResult.errorCode);
        pendingCancel = false;
        pendingStart = false;
    }

    findClose(nativeFindResult.findHandle);
}

void QBluetoothDeviceDiscoveryAgentPrivate::processDiscoveredDevices(
        const BLUETOOTH_DEVICE_INFO &info)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    QBluetoothDeviceInfo deviceInfo(
                QBluetoothAddress(info.Address.ullLong),
                QString::fromWCharArray(info.szName),
                info.ulClassofDevice);

    if (info.fRemembered)
        deviceInfo.setCached(true);

    discoveredDevices.append(deviceInfo);
    emit q->deviceDiscovered(deviceInfo);
}

void QBluetoothDeviceDiscoveryAgentPrivate::handleErrors(DWORD errorCode)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    lastError = (errorCode == ERROR_INVALID_HANDLE) ?
                QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError
              : QBluetoothDeviceDiscoveryAgent::InputOutputError;
    errorString = qt_error_string(errorCode);
    emit q->error(lastError);
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isRunning() const
{
    return findWatcher && findWatcher->isRunning();
}

QVariant QBluetoothDeviceDiscoveryAgentPrivate::findFirstDevice(HANDLE radioHandle)
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
    searchParams.hRadio = radioHandle;

    NativeFindResult nativeFindResult;
    nativeFindResult.findHandle = ::BluetoothFindFirstDevice(
                &searchParams, &nativeFindResult.deviceInfo);
    if (!nativeFindResult.findHandle)
        nativeFindResult.errorCode = ::GetLastError();

    QVariant result;
    result.setValue(nativeFindResult);
    return result;
}

QVariant QBluetoothDeviceDiscoveryAgentPrivate::findNextDevice(HBLUETOOTH_DEVICE_FIND findHandle)
{
    NativeFindResult nativeFindResult;
    nativeFindResult.findHandle = findHandle;
    if (!::BluetoothFindNextDevice(findHandle, &nativeFindResult.deviceInfo))
        nativeFindResult.errorCode = ::GetLastError();

    QVariant result;
    result.setValue(nativeFindResult);
    return result;
}

void QBluetoothDeviceDiscoveryAgentPrivate::findClose(HBLUETOOTH_DEVICE_FIND findHandle)
{
    if (findHandle)
        ::BluetoothFindDeviceClose(findHandle);
}

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE(NativeFindResult))

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

#include "qbluetoothlocaldevice.h"
#include "qbluetoothaddress.h"

#include <qt_windows.h>
#include <bluetoothapis.h>

QT_BEGIN_NAMESPACE

QBluetoothLocalDevice::QBluetoothLocalDevice(QObject *parent) :
    QObject(parent),
    d_ptr(0)
{
}

QBluetoothLocalDevice::QBluetoothLocalDevice(const QBluetoothAddress &, QObject *parent) :
    QObject(parent),
    d_ptr(0)
{
}

QString QBluetoothLocalDevice::name() const
{
    return QString();
}

QBluetoothAddress QBluetoothLocalDevice::address() const
{
    return QBluetoothAddress();
}

void QBluetoothLocalDevice::powerOn()
{
}

void QBluetoothLocalDevice::setHostMode(QBluetoothLocalDevice::HostMode mode)
{
    Q_UNUSED(mode);
}

QBluetoothLocalDevice::HostMode QBluetoothLocalDevice::hostMode() const
{
    return HostPoweredOff;
}

QList<QBluetoothAddress> QBluetoothLocalDevice::connectedDevices() const
{
    return QList<QBluetoothAddress>();
}

QList<QBluetoothHostInfo> QBluetoothLocalDevice::allDevices()
{
    BLUETOOTH_FIND_RADIO_PARAMS findRadioParams;
    ::ZeroMemory(&findRadioParams, sizeof(findRadioParams));
    findRadioParams.dwSize = sizeof(findRadioParams);

    HANDLE radioHandle = NULL;
    const HBLUETOOTH_RADIO_FIND radioFindHandle = ::BluetoothFindFirstRadio(&findRadioParams,
                                                                            &radioHandle);
    if (!radioFindHandle)
        return QList<QBluetoothHostInfo>();

    QList<QBluetoothHostInfo> localDevices;

    forever {
        BLUETOOTH_RADIO_INFO radioInfo;
        ::ZeroMemory(&radioInfo, sizeof(radioInfo));
        radioInfo.dwSize = sizeof(radioInfo);

        const DWORD retval = ::BluetoothGetRadioInfo(radioHandle, &radioInfo);
        ::CloseHandle(radioHandle);

        if (retval != ERROR_SUCCESS)
            break;

        QBluetoothHostInfo localDevice;
        localDevice.setAddress(QBluetoothAddress(radioInfo.address.ullLong));
        localDevice.setName(QString::fromWCharArray(radioInfo.szName));
        localDevices.append(localDevice);

        if (!::BluetoothFindNextRadio(radioFindHandle, &radioHandle))
            break;
    }

    ::BluetoothFindRadioClose(radioFindHandle);
    return localDevices;
}

void QBluetoothLocalDevice::requestPairing(const QBluetoothAddress &address, Pairing pairing)
{
    Q_UNUSED(address);
    Q_UNUSED(pairing);
}

QBluetoothLocalDevice::Pairing QBluetoothLocalDevice::pairingStatus(
    const QBluetoothAddress &address) const
{
    Q_UNUSED(address);
    return Unpaired;
}

void QBluetoothLocalDevice::pairingConfirmation(bool confirmation)
{
    Q_UNUSED(confirmation);
}

QT_END_NAMESPACE

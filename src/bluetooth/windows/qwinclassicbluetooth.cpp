/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Copyright (C) 2014 Denis Shienkov <denis.shienkov@gmail.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
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
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwinclassicbluetooth_p.h"

QT_BEGIN_NAMESPACE

namespace WinClassicBluetooth {

LocalRadiosDiscoveryResult::LocalRadiosDiscoveryResult()
    : error(NO_ERROR)
{
}

RemoteDeviceDiscoveryResult::RemoteDeviceDiscoveryResult()
    : hSearch(0)
    , error(NO_ERROR)
{
    ::ZeroMemory(&device, sizeof(device));
    device.dwSize = sizeof(device);
}

LocalRadiosDiscoveryResult enumerateLocalRadios()
{
    BLUETOOTH_FIND_RADIO_PARAMS params;
    ::ZeroMemory(&params, sizeof(params));
    params.dwSize = sizeof(params);

    HANDLE hRadio = 0;
    const HBLUETOOTH_RADIO_FIND hSearch =
            ::BluetoothFindFirstRadio(&params, &hRadio);

    LocalRadiosDiscoveryResult result;

    if (!hSearch) {
        result.error = ::GetLastError();
        return result;
    }

    forever {
        BLUETOOTH_RADIO_INFO radio;
        ::ZeroMemory(&radio, sizeof(radio));
        radio.dwSize = sizeof(radio);

        const DWORD retval = ::BluetoothGetRadioInfo(hRadio, &radio);
        ::CloseHandle(hRadio);

        if (retval != ERROR_SUCCESS) {
            result.error = ::GetLastError();
            break;
        }

        result.radios.append(radio);

        if (!::BluetoothFindNextRadio(hSearch, &hRadio)) {
            result.error = ::GetLastError();
            break;
        }
    }

    ::BluetoothFindRadioClose(hSearch);
    return result;
}

RemoteDeviceDiscoveryResult startDiscoveryOfFirstRemoteDevice()
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

    RemoteDeviceDiscoveryResult result;
    result.hSearch = ::BluetoothFindFirstDevice(
                &searchParams, &result.device);

    if (!result.hSearch)
        result.error = ::GetLastError();
    return result;
}

RemoteDeviceDiscoveryResult startDiscoveryOfNextRemoteDevice(
        HBLUETOOTH_DEVICE_FIND hSearch)
{
    RemoteDeviceDiscoveryResult result;
    result.hSearch = hSearch;
    if (!::BluetoothFindNextDevice(hSearch, &result.device))
        result.error = ::GetLastError();
    return result;
}

void cancelRemoteDevicesDiscovery(HBLUETOOTH_DEVICE_FIND hSearch)
{
    if (hSearch)
        ::BluetoothFindDeviceClose(hSearch);
}

} // namespace WinClassicBluetooth

QT_END_NAMESPACE

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

#include "qwinlowenergybluetooth_p.h"

#include <QtCore/quuid.h>
#include <QtCore/qlibrary.h>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

namespace WinLowEnergyBluetooth {

#define DEFINEFUNC(ret, func, ...) \
    typedef ret (WINAPI *fp_##func)(__VA_ARGS__); \
    static fp_##func func;

#define RESOLVEFUNC(func) \
    func = (fp_##func)resolveFunction(library, #func); \
    if (!func) \
        return false;

DEFINEFUNC(HRESULT, BluetoothGATTGetServices, HANDLE, USHORT, PBTH_LE_GATT_SERVICE, PUSHORT, ULONG)
DEFINEFUNC(HRESULT, BluetoothGATTGetIncludedServices, HANDLE, PBTH_LE_GATT_SERVICE, USHORT, PBTH_LE_GATT_SERVICE, PUSHORT, ULONG)
DEFINEFUNC(HRESULT, BluetoothGATTGetCharacteristics, HANDLE, PBTH_LE_GATT_SERVICE, USHORT, PBTH_LE_GATT_CHARACTERISTIC, PUSHORT, ULONG)
DEFINEFUNC(HRESULT, BluetoothGATTGetDescriptors, HANDLE, PBTH_LE_GATT_CHARACTERISTIC, USHORT, PBTH_LE_GATT_DESCRIPTOR, PUSHORT, ULONG)
DEFINEFUNC(HRESULT, BluetoothGATTGetCharacteristicValue, HANDLE, PBTH_LE_GATT_CHARACTERISTIC, ULONG, PBTH_LE_GATT_CHARACTERISTIC_VALUE, PUSHORT, ULONG)
DEFINEFUNC(HRESULT, BluetoothGATTGetDescriptorValue, HANDLE, PBTH_LE_GATT_DESCRIPTOR, ULONG, PBTH_LE_GATT_DESCRIPTOR_VALUE, PUSHORT, ULONG)
DEFINEFUNC(HRESULT, BluetoothGATTBeginReliableWrite, HANDLE, PBTH_LE_GATT_RELIABLE_WRITE_CONTEXT, ULONG)
DEFINEFUNC(HRESULT, BluetoothGATTEndReliableWrite, HANDLE, BTH_LE_GATT_RELIABLE_WRITE_CONTEXT, ULONG)
DEFINEFUNC(HRESULT, BluetoothGATTAbortReliableWrite, HANDLE, BTH_LE_GATT_RELIABLE_WRITE_CONTEXT, ULONG)
DEFINEFUNC(HRESULT, BluetoothGATTSetCharacteristicValue, HANDLE, PBTH_LE_GATT_CHARACTERISTIC, PBTH_LE_GATT_CHARACTERISTIC_VALUE, BTH_LE_GATT_RELIABLE_WRITE_CONTEXT, ULONG)
DEFINEFUNC(HRESULT, BluetoothGATTSetDescriptorValue, HANDLE, PBTH_LE_GATT_DESCRIPTOR, PBTH_LE_GATT_DESCRIPTOR_VALUE, ULONG)
DEFINEFUNC(HRESULT, BluetoothGATTRegisterEvent, HANDLE, BTH_LE_GATT_EVENT_TYPE, PVOID, PFNBLUETOOTH_GATT_EVENT_CALLBACK, PVOID, PHANDLE, ULONG)
DEFINEFUNC(HRESULT, BluetoothGATTUnregisterEvent, HANDLE, ULONG)

static inline QFunctionPointer resolveFunction(QLibrary *library, const char *func)
{
    QFunctionPointer symbolFunctionPointer = library->resolve(func);
    if (!symbolFunctionPointer)
        qWarning("Cannot resolve '%s' in '%s'.", func, qPrintable(library->fileName()));
    return symbolFunctionPointer;
}

static inline bool resolveFunctions(QLibrary *library)
{
    if (!library->isLoaded()) {
        library->setFileName(QStringLiteral("bluetoothapis"));
        if (!library->load()) {
            qWarning("Unable to load '%s' library.", qPrintable(library->fileName()));
            return false;
        }
    }

    RESOLVEFUNC(BluetoothGATTGetServices)
    RESOLVEFUNC(BluetoothGATTGetIncludedServices)
    RESOLVEFUNC(BluetoothGATTGetCharacteristics)
    RESOLVEFUNC(BluetoothGATTGetDescriptors)
    RESOLVEFUNC(BluetoothGATTGetCharacteristicValue)
    RESOLVEFUNC(BluetoothGATTGetDescriptorValue)
    RESOLVEFUNC(BluetoothGATTBeginReliableWrite)
    RESOLVEFUNC(BluetoothGATTEndReliableWrite)
    RESOLVEFUNC(BluetoothGATTAbortReliableWrite)
    RESOLVEFUNC(BluetoothGATTSetCharacteristicValue)
    RESOLVEFUNC(BluetoothGATTSetDescriptorValue)
    RESOLVEFUNC(BluetoothGATTRegisterEvent)
    RESOLVEFUNC(BluetoothGATTUnregisterEvent)

    return true;
}

Q_GLOBAL_STATIC(QLibrary, bluetoothapis)

ServicesDiscoveryResult::ServicesDiscoveryResult()
    : error(NO_ERROR)
{
}

ServicesDiscoveryResult startDiscoveryOfPrimaryServices(
        HANDLE hDevice)
{
    ServicesDiscoveryResult result;
    USHORT servicesCount = 0;
    forever {
        const HRESULT hr = BluetoothGATTGetServices(
                    hDevice,
                    result.services.count(),
                    result.services.isEmpty() ? NULL : &result.services[0],
                    &servicesCount,
                    BLUETOOTH_GATT_FLAG_NONE);

        if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
            result.services.resize(servicesCount);
        } else if (hr == S_OK) {
            break;
        } else {
            result.error = ::GetLastError();
            result.services.clear();
            break;
        }
    }
    return result;
}

bool isSupported()
{
    static bool resolved = resolveFunctions(bluetoothapis());
    return resolved;
}

} // namespace WinLowEnergyBluetooth

QT_END_NAMESPACE

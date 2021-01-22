/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include <QtCore/private/qeventdispatcher_winrt_p.h>

#define Q_OS_WINRT
#include <QtCore/qfunctions_winrt.h>

#include <wrl.h>
#include <windows.devices.bluetooth.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Devices::Bluetooth;
using namespace ABI::Windows::Foundation;

QT_BEGIN_NAMESPACE

#pragma warning (push)
#pragma warning (disable: 4273)
HRESULT QEventDispatcherWinRT::runOnXamlThread(const std::function<HRESULT()> &delegate, bool waitForRun)
{
    Q_UNUSED(waitForRun)
    return delegate();
}
#pragma warning (pop)

extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH: {
        // Check if we are running on a recent enough Windows
        HRESULT  hr = OleInitialize(NULL);
        if (FAILED(hr)) {
            MessageBox(NULL, (LPCWSTR)L"OleInitialize failed.", (LPCWSTR)L"Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
            exit(-1);
        }
        ComPtr<IBluetoothDeviceStatics> deviceStatics;
        hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_BluetoothDevice).Get(), &deviceStatics);
        if (hr == REGDB_E_CLASSNOTREG) {
            QString error ("This Windows version (" + QSysInfo::kernelVersion() + ") does not "
                "support the required Bluetooth API. Consider updating to a more recent Windows "
                "(10.0.10586 or above).");
            MessageBox(NULL, (LPCWSTR)error.constData(), (LPCWSTR)L"Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
            CoUninitialize();
            exit(-1);
        }
        break;
    }
    case DLL_PROCESS_DETACH:
        CoUninitialize();
    }

    return TRUE;
}

QT_END_NAMESPACE

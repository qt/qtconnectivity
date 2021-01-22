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

#include "qbluetoothutils_winrt_p.h"
#include <QtBluetooth/private/qtbluetoothglobal_p.h>

#include <QtCore/qfunctions_winrt.h>

#include <robuffer.h>
#include <wrl.h>
#include <windows.foundation.metadata.h>
#include <windows.storage.streams.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation::Metadata;
using namespace ABI::Windows::Storage::Streams;

QT_BEGIN_NAMESPACE

bool supportsNewLEApi()
{
    static bool initialized = false;
    static boolean apiPresent = false;
    if (initialized)
        return apiPresent;

    initialized = true;
#if !QT_CONFIG(winrt_btle_no_pairing)
    return apiPresent;
#endif

    ComPtr<IApiInformationStatics> apiInformationStatics;
    HRESULT hr = RoGetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Foundation_Metadata_ApiInformation).Get(),
                                IID_PPV_ARGS(&apiInformationStatics));
    if (FAILED(hr))
        return apiPresent;

    const HStringReference valueRef(L"Windows.Foundation.UniversalApiContract");
    hr = apiInformationStatics->IsApiContractPresentByMajor(
                valueRef.Get(), 4, &apiPresent);
    apiPresent = SUCCEEDED(hr) && apiPresent;
    return apiPresent;
}

QByteArray byteArrayFromBuffer(const ComPtr<NativeBuffer> &buffer, bool isWCharString)
{
    if (!buffer) {
        qErrnoWarning("nullptr passed to byteArrayFromBuffer");
        return QByteArray();
    }
    ComPtr<Windows::Storage::Streams::IBufferByteAccess> byteAccess;
    HRESULT hr = buffer.As(&byteAccess);
    RETURN_IF_FAILED("Could not cast buffer", return QByteArray())
    char *data;
    hr = byteAccess->Buffer(reinterpret_cast<byte **>(&data));
    RETURN_IF_FAILED("Could not obtain buffer data", return QByteArray())
    UINT32 size;
    hr = buffer->get_Length(&size);
    RETURN_IF_FAILED("Could not obtain buffer size", return QByteArray())
    if (isWCharString) {
        QString valueString = QString::fromUtf16(reinterpret_cast<ushort *>(data)).left(size / 2);
        return valueString.toUtf8();
    }
    return QByteArray(data, qint32(size));
}

QT_END_NAMESPACE

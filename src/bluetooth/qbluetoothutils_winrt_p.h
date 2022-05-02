/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

#ifndef QBLUETOOTHUTILS_WINRT_P_H
#define QBLUETOOTHUTILS_WINRT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Workaround for Windows SDK bug.
// See https://github.com/microsoft/Windows.UI.Composition-Win32-Samples/issues/47
 #include <winrt/base.h>
namespace winrt::impl
{
    template <typename Async>
    auto wait_for(Async const& async, Windows::Foundation::TimeSpan const& timeout);
}

#include <QtCore/QtGlobal>

#include <wrl/client.h>

namespace ABI {
    namespace Windows {
        namespace Storage {
            namespace Streams {
                struct IBuffer;
            }
        }
    }
}

QT_BEGIN_NAMESPACE

using NativeBuffer = ABI::Windows::Storage::Streams::IBuffer;
QByteArray byteArrayFromBuffer(const Microsoft::WRL::ComPtr<NativeBuffer> &buffer,
                               bool isWCharString = false);

// The calls to Co(Un)init must be balanced
void mainThreadCoInit(void* caller);
void mainThreadCoUninit(void* caller);

QT_END_NAMESPACE

#endif // QBLUETOOTHSOCKET_WINRT_P_H

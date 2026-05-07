// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
#include <QtCore/private/qfactorycacheregistration_p.h>
namespace winrt::impl
{
    template <typename Async>
    auto wait_for(Async const& async, Windows::Foundation::TimeSpan const& timeout);
}

#include <QtCore/QtGlobal>
#include <QtCore/private/qglobal_p.h>

#include <wrl/client.h>

#include <functional>
#include <type_traits>

// Macros for guarding C++/WinRT projection calls. Each accessor on a winrt
// projection runs check_hresult() on the underlying ABI HRESULT and throws
// winrt::hresult_error on failure. After the Bluetooth stack tears down state
// (suspend/resume, radio toggle, service restart), those calls routinely fail
// and the exception propagates out of completion lambdas, crashing the process.
//
// SAFE(expr): returns expr's value on success, a logged-and-defaulted value on
//   a caught hresult_error. Default-constructible types (primitives, enums,
//   winrt::hstring, generic interfaces like IAsyncOperation<T>) take T{};
//   runtimeclass projections that only declare an explicit Class(nullptr_t)
//   constructor fall back to T(nullptr).
// HR(stmt): runs stmt and returns the resulting winrt::hresult (S_OK on
//   success, e.code() on failure).
// TRY(stmt): runs stmt and returns true on success, false on failure.
// LOG_HRESULT(hr): warning-log helper used by the wrappers.
//
// Callsites must have the QT_BT_WINDOWS logging category in scope.

#define LOG_HRESULT(hr) qCWarning(QT_BT_WINDOWS) << "HRESULT:" << quint32(hr)

#define HR(x) \
    std::invoke([&]() { \
        try { \
            x; \
        } catch (winrt::hresult_error const &e) { \
            LOG_HRESULT(e.code()) << "/*" << #x << "*/"; \
            return e.code(); \
        } \
        return winrt::hresult{ S_OK }; \
    })

#define TRY(x) \
    std::invoke([&]() { \
        try { \
            x; \
        } catch (winrt::hresult_error const &e) { \
            LOG_HRESULT(e.code()) << "/*" << #x << "*/"; \
            return false; \
        } \
        return true; \
    })

#define SAFE(x) \
    std::invoke([&]() -> decltype(x) { \
        try { return (x); } \
        catch (winrt::hresult_error const &e) { \
            LOG_HRESULT(e.code()) << "/*" << #x << "*/"; \
            if constexpr (std::is_default_constructible_v<decltype(x)>) \
                return decltype(x){}; \
            else \
                return decltype(x)(nullptr); \
        } \
    })

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
void threadCoInit(void* caller);
void threadCoUninit(void* caller);

QT_END_NAMESPACE

#endif // QBLUETOOTHSOCKET_WINRT_P_H

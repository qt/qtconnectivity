// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothutils_winrt_p.h"
#include <QtBluetooth/private/qtbluetoothglobal_p.h>
#include <QtCore/private/qfunctions_winrt_p.h>
#include <QtCore/qhash.h>
#include <QtCore/QLoggingCategory>
#include <QtCore/qmutex.h>

#include <robuffer.h>
#include <wrl.h>
#include <winrt/windows.foundation.metadata.h>
#include <windows.storage.streams.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace winrt::Windows::Foundation::Metadata;
using namespace ABI::Windows::Storage::Streams;

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

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
        QString valueString = QString::fromUtf16(reinterpret_cast<char16_t *>(data)).left(size / 2);
        return valueString.toUtf8();
    }
    return QByteArray(data, qint32(size));
}

static QHash<void *, QThread *> successfulInits;
static QBasicMutex initsMutex;

void threadCoInit(void* caller)
{
    Q_ASSERT(caller);

    if (QMutexLocker locker(&initsMutex); successfulInits.contains(caller)) {
        qCWarning(QT_BT_WINDOWS) << "Multiple COM inits by the same object";
        return;
    }

    // This *may* execute in the main thread which may run Gui, so we request
    // the apartment-threaded model. However, QtBluetooth does not strictly
    // require it, so if the thread is already initialized with a different
    // model, that's also totally fine for us.
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (!SUCCEEDED(hr)) {
        // RPC_E_CHANGED_MODE means that the thread is already initialized
        // in a different mode. Do not warn about it.
        if (hr != RPC_E_CHANGED_MODE)
            qCWarning(QT_BT_WINDOWS) << "Unexpected COM initialization result";
        return;
    }

    QMutexLocker locker(&initsMutex);
    successfulInits.insert(caller, QThread::currentThread());
}

void threadCoUninit(void* caller)
{
    Q_ASSERT(caller);

    QThread *thread = nullptr;
    {
        QMutexLocker locker(&initsMutex);
        thread = successfulInits.value(caller, nullptr);
    }
    // Valid case: thread could be initialized outside of Qt
    if (!thread)
        return;

    if (QThread::currentThread() != thread) {
        qCWarning(QT_BT_WINDOWS) << "COM uninit tried from another thread";
        return;
    }

    CoUninitialize();

    QMutexLocker locker(&initsMutex);
    successfulInits.remove(caller);

}

QT_END_NAMESPACE

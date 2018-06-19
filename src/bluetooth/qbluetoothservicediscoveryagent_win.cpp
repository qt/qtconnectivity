/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbluetoothservicediscoveryagent.h"
#include "qbluetoothservicediscoveryagent_p.h"

#include <QtCore/QByteArray>
#include <QtCore/QLibrary>
#include <QtCore/QLoggingCategory>
#include <QtCore/QThread>
#include <QtCore/QUrl>

#include <initguid.h>
#include <WinSock2.h>
#include <qt_windows.h>
#include <bluetoothapis.h>
#include <ws2bth.h>

Q_GLOBAL_STATIC(QLibrary, bluetoothapis)

#define DEFINEFUNC(ret, func, ...) \
    typedef ret (WINAPI *fp_##func)(__VA_ARGS__); \
    static fp_##func p##func;

#define RESOLVEFUNC(func) \
    p##func = (fp_##func)resolveFunction(library, #func); \
    if (!p##func) \
        return false;

DEFINEFUNC(DWORD, BluetoothSdpGetElementData, LPBYTE, ULONG, PSDP_ELEMENT_DATA)

QT_BEGIN_NAMESPACE

struct FindServiceArguments {
    QBluetoothAddress address;
    Qt::HANDLE hSearch;
    int systemError;
};

struct FindServiceResult {
    QBluetoothServiceInfo info;
    Qt::HANDLE hSearch;
    int systemError;
};

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

static inline QFunctionPointer resolveFunction(QLibrary *library, const char *func)
{
    const QFunctionPointer symbolFunctionPointer = library->resolve(func);
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

    RESOLVEFUNC(BluetoothSdpGetElementData)

    return true;
}

static QList<QVariant> spdContainerToVariantList(LPBYTE containerStream, ULONG containerLength);

static QVariant spdElementToVariant(const SDP_ELEMENT_DATA &element)
{
    QVariant variant;

    switch (element.type) {
    case SDP_TYPE_UINT: {
        switch (element.specificType) {
        case SDP_ST_UINT128:
            //Not supported!!!
            break;
        case SDP_ST_UINT64:
            variant =  QVariant::fromValue<quint64>(element.data.uint64);
            break;
        case SDP_ST_UINT32:
            variant =  QVariant::fromValue<quint32>(element.data.uint32);
            break;
        case SDP_ST_UINT16:
            variant =  QVariant::fromValue<quint16>(element.data.uint16);
            break;
        case SDP_ST_UINT8:
            variant =  QVariant::fromValue<quint8>(element.data.uint8);
            break;
        default:
            break;
        }
        break;
    }
    case SDP_TYPE_INT: {
        switch (element.specificType) {
        case SDP_ST_INT128: {
            //Not supported!!!
            break;
        }
        case SDP_ST_INT64:
            variant = QVariant::fromValue<qint64>(element.data.int64);
            break;
        case SDP_ST_INT32:
            variant = QVariant::fromValue<qint32>(element.data.int32);
            break;
        case SDP_ST_INT16:
            variant = QVariant::fromValue<qint16>(element.data.int16);
            break;
        case SDP_ST_INT8:
            variant = QVariant::fromValue<qint8>(element.data.int8);
            break;
        default:
            break;
        }
        break;
    }
    case SDP_TYPE_UUID: {
        switch (element.specificType) {
        case SDP_ST_UUID128: {
            //Not sure if this will work, might be evil
            Q_ASSERT(sizeof(element.data.uuid128) == sizeof(quint128));

            quint128 uuid128;
            memcpy(&uuid128, &(element.data.uuid128), sizeof(quint128));

            variant = QVariant::fromValue(QBluetoothUuid(uuid128));
        }
        case SDP_ST_UUID32:
            variant = QVariant::fromValue(QBluetoothUuid(quint32(element.data.uuid32)));
        case SDP_ST_UUID16:
            variant = QVariant::fromValue(QBluetoothUuid(quint16(element.data.uuid16)));
        default:
            break;
        }
        break;
    }
    case SDP_TYPE_STRING: {
        const QByteArray stringBuffer(reinterpret_cast<const char*>(element.data.string.value), element.data.string.length);
        variant = QVariant::fromValue<QString>(QString::fromLocal8Bit(stringBuffer));
        break;
    }
    case SDP_TYPE_URL: {
        const QString urlString = QString::fromLocal8Bit(reinterpret_cast<const char*>(element.data.url.value),
                                                   (int)element.data.url.length);
        const QUrl url(urlString);
        if (url.isValid())
            variant = QVariant::fromValue<QUrl>(url);
        break;
    }
    case SDP_TYPE_SEQUENCE: {
        const QList<QVariant> sequenceList = spdContainerToVariantList(element.data.sequence.value,
                                                                 element.data.sequence.length);
        const QBluetoothServiceInfo::Sequence sequence(sequenceList);
        variant = QVariant::fromValue(sequence);
        break;
    }
    case SDP_TYPE_ALTERNATIVE: {
        const QList<QVariant> alternativeList = spdContainerToVariantList(element.data.alternative.value,
                                                                    element.data.alternative.length);
        const QBluetoothServiceInfo::Alternative alternative(alternativeList);
        variant = QVariant::fromValue(alternative);
        break;
    }
    case SDP_TYPE_BOOLEAN:
        variant = QVariant::fromValue<bool>((bool)element.data.booleanVal);
        break;
    case SDP_TYPE_NIL:
        break;
    default:
        break;
    }

    return variant;
}

static QList<QVariant> spdContainerToVariantList(LPBYTE containerStream, ULONG containerLength)
{
    HBLUETOOTH_CONTAINER_ELEMENT iter = NULL;
    SDP_ELEMENT_DATA element;

    QList<QVariant> sequence;

    for (;;) {
        const DWORD result = BluetoothSdpGetContainerElementData(containerStream,
                                                           containerLength,
                                                           &iter,
                                                           &element);

        if (result == ERROR_SUCCESS) {
            const QVariant variant = spdElementToVariant(element);
            sequence.append(variant);
        } else if (result == ERROR_NO_MORE_ITEMS) {
            break;
        } else if (result == ERROR_INVALID_PARAMETER) {
            break;
        }
    }

    return sequence;
}

#if defined(Q_CC_MINGW)
# define SDP_CALLBACK
#else
# define SDP_CALLBACK QT_WIN_CALLBACK
#endif
static BOOL SDP_CALLBACK bluetoothSdpCallback(ULONG attributeId, LPBYTE valueStream, ULONG streamSize, LPVOID param)
{
    QBluetoothServiceInfo *result = static_cast<QBluetoothServiceInfo*>(param);

    SDP_ELEMENT_DATA element;

    if (pBluetoothSdpGetElementData(valueStream, streamSize, &element) == ERROR_SUCCESS)
        switch (element.type) {
        case SDP_TYPE_UINT:
        case SDP_TYPE_INT:
        case SDP_TYPE_UUID:
        case SDP_TYPE_STRING:
        case SDP_TYPE_URL:
        case SDP_TYPE_BOOLEAN:
        case SDP_TYPE_SEQUENCE:
        case SDP_TYPE_ALTERNATIVE: {
            const QVariant variant = spdElementToVariant(element);
            result->setAttribute(attributeId, variant);
            break;
        }
        case SDP_TYPE_NIL:
            break;
        default:
            break;
        }

    return true;
}

static void cleanupServiceDiscovery(HANDLE hSearch)
{
    if (hSearch != INVALID_HANDLE_VALUE)
        WSALookupServiceEnd(hSearch);
    WSACleanup();
}

enum {
    WSAControlFlags = LUP_FLUSHCACHE
          | LUP_RETURN_NAME
          | LUP_RETURN_TYPE
          | LUP_RETURN_ADDR
          | LUP_RETURN_BLOB
          | LUP_RETURN_COMMENT
};

static FindServiceResult findNextService(HANDLE hSearch, int systemError)
{
    FindServiceResult result;
    result.systemError = systemError;
    result.hSearch = INVALID_HANDLE_VALUE;

    QByteArray resultBuffer(2048, 0);
    WSAQUERYSET *resultQuery = reinterpret_cast<WSAQUERYSET*>(resultBuffer.data());
    DWORD resultBufferSize = DWORD(resultBuffer.size());
    const int resultCode = WSALookupServiceNext(hSearch,
                                                WSAControlFlags,
                                                &resultBufferSize,
                                                resultQuery);

    if (resultCode == SOCKET_ERROR) {
        result.systemError = ::WSAGetLastError();
        if (result.systemError == WSA_E_NO_MORE)
            cleanupServiceDiscovery(hSearch);
        return result;
     }

    if (resultQuery->lpBlob
            && BluetoothSdpEnumAttributes(resultQuery->lpBlob->pBlobData,
                                          resultQuery->lpBlob->cbSize,
                                          bluetoothSdpCallback,
                                          &result.info)) {
        return result;
    } else {
        result.systemError = GetLastError();
    }
    return result;
}

static FindServiceResult findFirstService(HANDLE hSearch, const QBluetoothAddress &address)
{
    //### should we try for 2.2 on all platforms ??
    WSAData wsadata;
    FindServiceResult result;
    result.systemError = NO_ERROR;
    result.hSearch = INVALID_HANDLE_VALUE;

    // IPv6 requires Winsock v2.0 or better.
    if (WSAStartup(MAKEWORD(2, 0), &wsadata) != 0) {
        result.systemError = ::WSAGetLastError();
        return result;
    }

    const QString addressAsString = QStringLiteral("(%1)").arg(address.toString());

    QVector<WCHAR> addressAsWChar(addressAsString.size());
    addressAsString.toWCharArray(addressAsWChar.data());

    GUID protocol = L2CAP_PROTOCOL_UUID; //Search for L2CAP and RFCOMM services

    WSAQUERYSET serviceQuery = {0};
    serviceQuery.dwSize = sizeof(WSAQUERYSET); //As specified by the windows documentation
    serviceQuery.lpServiceClassId = &protocol; //The protocal of the service what is being queried
    serviceQuery.dwNameSpace = NS_BTH; //As specified by the windows documentation
    serviceQuery.dwNumberOfCsAddrs = 0;  //As specified by the windows documentation
    serviceQuery.lpszContext = addressAsWChar.data(); //The remote address that query will run on

    const int resultCode = WSALookupServiceBegin(&serviceQuery,
                                                 WSAControlFlags,
                                                 &hSearch);
    if (resultCode == SOCKET_ERROR) {
        result.systemError = ::WSAGetLastError();
        cleanupServiceDiscovery(&hSearch);
        return result;
    }
    result.systemError = NO_ERROR;
    return findNextService(hSearch, result.systemError);
}

QBluetoothServiceDiscoveryAgentPrivate::QBluetoothServiceDiscoveryAgentPrivate(
        QBluetoothServiceDiscoveryAgent *qp, const QBluetoothAddress &deviceAdapter)
    :  error(QBluetoothServiceDiscoveryAgent::NoError),
      state(Inactive),
      deviceDiscoveryAgent(0),
      mode(QBluetoothServiceDiscoveryAgent::MinimalDiscovery),
      singleDevice(false),
      pendingStop(false),
      pendingFinish(false),
      q_ptr(qp)
{
    Q_UNUSED(deviceAdapter);

    resolveFunctions(bluetoothapis());

    threadFind = new QThread;
    threadWorkerFind = new ThreadWorkerFind;
    threadWorkerFind->moveToThread(threadFind);
    connect(threadWorkerFind, &ThreadWorkerFind::findFinished, this, &QBluetoothServiceDiscoveryAgentPrivate::_q_nextSdpScan);
    connect(threadFind, &QThread::finished, threadWorkerFind, &ThreadWorkerFind::deleteLater);
    connect(threadFind, &QThread::finished, threadFind, &QThread::deleteLater);
    threadFind->start();
}

QBluetoothServiceDiscoveryAgentPrivate::~QBluetoothServiceDiscoveryAgentPrivate()
{
    if (pendingFinish) {
        stop();
    }
    if (threadFind)
        threadFind->quit();
}

void QBluetoothServiceDiscoveryAgentPrivate::start(const QBluetoothAddress &address)
{
    if (!pendingFinish) {
        pendingFinish = true;
        pendingStop = false;

        FindServiceArguments data;
        data.address = address;
        data.hSearch = INVALID_HANDLE_VALUE;
        QMetaObject::invokeMethod(threadWorkerFind, "findFirst", Qt::QueuedConnection,
                                  Q_ARG(QVariant, QVariant::fromValue(data)));
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::stop()
{
    pendingStop = true;
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_nextSdpScan(const QVariant &input)
{
    Q_Q(QBluetoothServiceDiscoveryAgent);
    auto result = input.value<FindServiceResult>();

    if (pendingStop) {
        pendingStop = false;
        pendingFinish = false;
        emit q->canceled();
    } else {
        if (result.systemError == WSA_E_NO_MORE) {
            result.systemError = NO_ERROR;
        } else if (result.systemError != NO_ERROR) {
            error = (result.systemError == ERROR_INVALID_HANDLE) ?
                        QBluetoothServiceDiscoveryAgent::InvalidBluetoothAdapterError
                      : QBluetoothServiceDiscoveryAgent::InputOutputError;
            errorString = qt_error_string(result.systemError);
            qCWarning(QT_BT_WINDOWS) << errorString;
            emit q->error(this->error);
        } else {
            result.info.setDevice(discoveredDevices.at(0));
            if (result.info.isValid()) {
                if (!isDuplicatedService(result.info)) {
                    discoveredServices.append(result.info);
                    emit q->serviceDiscovered(result.info);
                }
            }
            FindServiceArguments data;
            data.hSearch = result.hSearch;
            data.systemError = result.systemError;
            QMetaObject::invokeMethod(threadWorkerFind, "findNext", Qt::QueuedConnection,
                                      Q_ARG(QVariant, QVariant::fromValue(data)));
            return;
        }
        pendingFinish = false;
        _q_serviceDiscoveryFinished();
    }
}

void ThreadWorkerFind::findFirst(const QVariant &data)
{
    auto args = data.value<FindServiceArguments>();
    FindServiceResult result = findFirstService(args.hSearch, args.address);
    result.hSearch = args.hSearch;
    emit findFinished(QVariant::fromValue(result));
}

 void ThreadWorkerFind::findNext(const QVariant &data)
{
    auto args = data.value<FindServiceArguments>();
    FindServiceResult result = findNextService(args.hSearch, args.systemError);
    result.hSearch = args.hSearch;
    emit findFinished(QVariant::fromValue(result));
}

QT_END_NAMESPACE

Q_DECLARE_METATYPE(FindServiceArguments)
Q_DECLARE_METATYPE(FindServiceResult)

/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#include "qbluetoothservicediscoveryagent.h"
#include "qbluetoothservicediscoveryagent_p.h"

#include <QtCore/QByteArray>
#include <QtConcurrent>

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

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

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
        QByteArray stringBuffer(reinterpret_cast<const char*>(element.data.string.value), element.data.string.length);
        variant = QVariant::fromValue<QString>(QString::fromLocal8Bit(stringBuffer));
        break;
    }
    case SDP_TYPE_URL: {
        QString urlString = QString::fromLocal8Bit(reinterpret_cast<const char*>(element.data.url.value),
                                                   (int)element.data.url.length);
        QUrl url(urlString);
        if (url.isValid())
            variant = QVariant::fromValue<QUrl>(url);
        break;
    }
    case SDP_TYPE_SEQUENCE: {
        QList<QVariant> sequenceList = spdContainerToVariantList(element.data.sequence.value,
                                                                 element.data.sequence.length);
        QBluetoothServiceInfo::Sequence sequence(sequenceList);
        variant = QVariant::fromValue(sequence);
        break;
    }
    case SDP_TYPE_ALTERNATIVE: {
        QList<QVariant> alternativeList = spdContainerToVariantList(element.data.alternative.value,
                                                                    element.data.alternative.length);
        QBluetoothServiceInfo::Alternative alternative(alternativeList);
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
        DWORD result = BluetoothSdpGetContainerElementData(containerStream,
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

static QBluetoothServiceInfo findNextService(HANDLE hSearch, int *systemError)
{
    QBluetoothServiceInfo result;

    QByteArray resultBuffer(2048, 0);
    WSAQUERYSET *resultQuery = reinterpret_cast<WSAQUERYSET*>(resultBuffer.data());
    DWORD resultBufferSize = DWORD(resultBuffer.size());
    const int resultCode = WSALookupServiceNext(hSearch,
                                                WSAControlFlags,
                                                &resultBufferSize,
                                                resultQuery);

    if (resultCode == SOCKET_ERROR) {
        *systemError = ::WSAGetLastError();
        if (*systemError == WSA_E_NO_MORE)
            cleanupServiceDiscovery(hSearch);
        return QBluetoothServiceInfo();
     }

    if (resultQuery->lpBlob
            && BluetoothSdpEnumAttributes(resultQuery->lpBlob->pBlobData,
                                          resultQuery->lpBlob->cbSize,
                                          bluetoothSdpCallback,
                                          &result)) {
        return result;
    } else {
        *systemError = GetLastError();
    }

    return result;
}

static QBluetoothServiceInfo findFirstService(LPHANDLE hSearch, const QBluetoothAddress &address, int *systemError)
{
    //### should we try for 2.2 on all platforms ??
    WSAData wsadata;

    // IPv6 requires Winsock v2.0 or better.
    if (WSAStartup(MAKEWORD(2, 0), &wsadata) != 0) {
        *systemError = ::WSAGetLastError();
        return QBluetoothServiceInfo();
    }

    const QString addressAsString = QStringLiteral("(%1)").arg(address.toString());

    QVector<WCHAR> addressAsWChar(addressAsString.size());
    addressAsString.toWCharArray(addressAsWChar.data());

    GUID protocol = L2CAP_PROTOCOL_UUID; //Search for L2CAP and RFCOMM services

    WSAQUERYSET serviceQuery;
    ::ZeroMemory(&serviceQuery, sizeof(serviceQuery));
    serviceQuery.dwSize = sizeof(WSAQUERYSET); //As specified by the windows documentation
    serviceQuery.lpServiceClassId = &protocol; //The protocal of the service what is being queried
    serviceQuery.dwNameSpace = NS_BTH; //As specified by the windows documentation
    serviceQuery.dwNumberOfCsAddrs = 0;  //As specified by the windows documentation
    serviceQuery.lpszContext = addressAsWChar.data(); //The remote address that query will run on

    const int resultCode = WSALookupServiceBegin(&serviceQuery,
                                                 WSAControlFlags,
                                                 hSearch);
    if (resultCode == SOCKET_ERROR) {
        *systemError = ::WSAGetLastError();
        cleanupServiceDiscovery(hSearch);
        return QBluetoothServiceInfo();
    }
    *systemError = NO_ERROR;
    return findNextService(*hSearch, systemError);
}

QBluetoothServiceDiscoveryAgentPrivate::QBluetoothServiceDiscoveryAgentPrivate(
        QBluetoothServiceDiscoveryAgent *qp, const QBluetoothAddress &deviceAdapter)
    :  error(QBluetoothServiceDiscoveryAgent::NoError),
      state(Inactive),
      deviceDiscoveryAgent(0),
      mode(QBluetoothServiceDiscoveryAgent::MinimalDiscovery),
      singleDevice(false),
      systemError(NO_ERROR),
      pendingStop(false),
      pendingFinish(false),
      searchWatcher(Q_NULLPTR),
      hSearch(INVALID_HANDLE_VALUE),
      q_ptr(qp)
{
    Q_UNUSED(deviceAdapter);
    resolveFunctions(bluetoothapis());
    searchWatcher = new QFutureWatcher<QBluetoothServiceInfo>();
}

QBluetoothServiceDiscoveryAgentPrivate::~QBluetoothServiceDiscoveryAgentPrivate()
{
    if (pendingFinish) {
        stop();
        searchWatcher->waitForFinished();
    }
    delete searchWatcher;
}

void QBluetoothServiceDiscoveryAgentPrivate::start(const QBluetoothAddress &address)
{
    Q_Q(QBluetoothServiceDiscoveryAgent);
    if (!pendingFinish) {
        pendingFinish = true;
        pendingStop = false;

        QObject::connect(searchWatcher, SIGNAL(finished()), q, SLOT(_q_nextSdpScan()), Qt::UniqueConnection);
        const QFuture<QBluetoothServiceInfo> future =
                QtConcurrent::run(&findFirstService,
                                  &hSearch,
                                  address,
                                  &systemError);
        searchWatcher->setFuture(future);
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::stop()
{
    pendingStop = true;
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_nextSdpScan()
{
    Q_Q(QBluetoothServiceDiscoveryAgent);

    if (pendingStop) {
        pendingStop = false;
        pendingFinish = false;
        emit q->canceled();
    } else {
        if (systemError == WSA_E_NO_MORE) {
            systemError = NO_ERROR;
        } else if (systemError != NO_ERROR) {
            error = (systemError == ERROR_INVALID_HANDLE) ?
                        QBluetoothServiceDiscoveryAgent::InvalidBluetoothAdapterError
                      : QBluetoothServiceDiscoveryAgent::InputOutputError;
            errorString = qt_error_string(systemError);
            qCWarning(QT_BT_WINDOWS) << errorString;
            emit q->error(this->error);
        } else {
            QBluetoothServiceInfo serviceInfo = searchWatcher->result();
            serviceInfo.setDevice(discoveredDevices.at(0));
            if (serviceInfo.isValid()) {
                if (!isDuplicatedService(serviceInfo)) {
                    discoveredServices.append(serviceInfo);
                    emit q->serviceDiscovered(serviceInfo);
                }
            }
            const QFuture<QBluetoothServiceInfo> future =
                    QtConcurrent::run(&findNextService, hSearch, &systemError);
            searchWatcher->setFuture(future);
            return;
        }
        hSearch = INVALID_HANDLE_VALUE;
        pendingFinish = false;
        _q_serviceDiscoveryFinished();
    }
}

QT_END_NAMESPACE

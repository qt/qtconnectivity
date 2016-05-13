/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "qbluetoothdevicediscoverytimer_osx_p.h"
#include "qbluetoothdevicediscoveryagent.h"
#include "osx/osxbtledeviceinquiry_p.h"
#include "qbluetoothlocaldevice.h"
#include "qbluetoothdeviceinfo.h"
#include "osx/osxbtutility_p.h"
#include "osx/uistrings_p.h"
#include "qbluetoothuuid.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>
#include <QtCore/qlist.h>

#include <CoreBluetooth/CoreBluetooth.h>

QT_BEGIN_NAMESPACE

using OSXBluetooth::ObjCScopedPointer;

class QBluetoothDeviceDiscoveryAgentPrivate
{
    friend class QBluetoothDeviceDiscoveryAgent;
    friend class OSXBluetooth::DDATimerHandler;

public:
    QBluetoothDeviceDiscoveryAgentPrivate(const QBluetoothAddress &address,
                                          QBluetoothDeviceDiscoveryAgent *q);
    virtual ~QBluetoothDeviceDiscoveryAgentPrivate();

    bool isActive() const;

    void start();
    void stop();

private:
    typedef QT_MANGLE_NAMESPACE(OSXBTLEDeviceInquiry) LEDeviceInquiryObjC;

    void LEinquiryError(QBluetoothDeviceDiscoveryAgent::Error error);
    void LEnotSupported();
    void LEdeviceFound(const QBluetoothDeviceInfo &info);
    void LEinquiryFinished();
    void checkLETimeout();

    void setError(QBluetoothDeviceDiscoveryAgent::Error, const QString &text = QString());

    QBluetoothDeviceDiscoveryAgent *q_ptr;

    QBluetoothDeviceDiscoveryAgent::Error lastError;
    QString errorString;

    QBluetoothDeviceDiscoveryAgent::InquiryType inquiryType;

    typedef ObjCScopedPointer<LEDeviceInquiryObjC> LEDeviceInquiry;
    LEDeviceInquiry inquiryLE;

    typedef QList<QBluetoothDeviceInfo> DevicesList;
    DevicesList discoveredDevices;

    bool startPending;
    bool stopPending;

    QScopedPointer<OSXBluetooth::DDATimerHandler> timer;
};

namespace OSXBluetooth {

DDATimerHandler::DDATimerHandler(QBluetoothDeviceDiscoveryAgentPrivate *d)
    : owner(d)
{
    Q_ASSERT_X(owner, Q_FUNC_INFO, "invalid pointer");

    timer.setSingleShot(false);
    connect(&timer, &QTimer::timeout, this, &DDATimerHandler::onTimer);
}

void DDATimerHandler::start(int msec)
{
    Q_ASSERT_X(msec > 0, Q_FUNC_INFO, "invalid time interval");
    if (timer.isActive()) {
        qCWarning(QT_BT_OSX) << Q_FUNC_INFO << "timer is active";
        return;
    }

    timer.start(msec);
}

void DDATimerHandler::stop()
{
    timer.stop();
}

void DDATimerHandler::onTimer()
{
    Q_ASSERT(owner);
    owner->checkLETimeout();
}

}

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(const QBluetoothAddress &adapter,
                                                                             QBluetoothDeviceDiscoveryAgent *q) :
    q_ptr(q),
    lastError(QBluetoothDeviceDiscoveryAgent::NoError),
    inquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry),
    startPending(false),
    stopPending(false)
{
    Q_UNUSED(adapter);

    Q_ASSERT_X(q != Q_NULLPTR, Q_FUNC_INFO, "invalid q_ptr (null)");
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
    if (inquiryLE) {
        // We want the LE scan to stop as soon as possible.
        if (dispatch_queue_t leQueue = OSXBluetooth::qt_LE_queue()) {
            // Local variable to be retained ...
            LEDeviceInquiryObjC *inq = inquiryLE.data();
            dispatch_async(leQueue, ^{
                [inq stop];
            });
        }
    }
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    if (startPending)
        return true;
    if (stopPending)
        return false;

    return inquiryLE;
}

void QBluetoothDeviceDiscoveryAgentPrivate::start()
{
    Q_ASSERT_X(!isActive(), Q_FUNC_INFO, "called on active device discovery agent");

    if (stopPending) {
        startPending = true;
        return;
    }

    using namespace OSXBluetooth;

    inquiryLE.reset([[LEDeviceInquiryObjC alloc] init]);
    dispatch_queue_t leQueue(qt_LE_queue());
    if (!leQueue || !inquiryLE) {
        setError(QBluetoothDeviceDiscoveryAgent::UnknownError,
                 QCoreApplication::translate(DEV_DISCOVERY, DD_NOT_STARTED));
        emit q_ptr->error(lastError);
    }

    discoveredDevices.clear();
    setError(QBluetoothDeviceDiscoveryAgent::NoError);

    // CoreBluetooth does not have a timeout. We start a timer here
    // and check if scan really started and if yes if we have a timeout.
    timer.reset(new OSXBluetooth::DDATimerHandler(this));
    timer->start(2000);

    // Create a local variable - to have a strong referece in a block.
    LEDeviceInquiryObjC *inq = inquiryLE.data();
    dispatch_async(leQueue, ^{
        [inq start];
    });
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    Q_ASSERT_X(isActive(), Q_FUNC_INFO, "called whithout active inquiry");

    startPending = false;
    stopPending = true;

    dispatch_queue_t leQueue(OSXBluetooth::qt_LE_queue());
    Q_ASSERT(leQueue);

    setError(QBluetoothDeviceDiscoveryAgent::NoError);

    // Create a local variable - to have a strong referece in a block.
    LEDeviceInquiryObjC *inq = inquiryLE.data();
    dispatch_async(leQueue, ^{
        [inq stop];
    });
    // We consider LE scan to be stopped immediately and
    // do not care about this LEDeviceInquiry object anymore.
    LEinquiryFinished();
}

void QBluetoothDeviceDiscoveryAgentPrivate::LEinquiryError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    // At the moment the only error reported by osxbtledeviceinquiry
    // can be 'powered off' error, it happens
    // after the LE scan started (so we have LE support and this is
    // a real PoweredOffError).
    Q_ASSERT_X(error == QBluetoothDeviceDiscoveryAgent::PoweredOffError,
               Q_FUNC_INFO, "unexpected error");

    inquiryLE.reset();
    timer->stop();

    startPending = false;
    stopPending = false;
    setError(error);
    emit q_ptr->error(lastError);
}

void QBluetoothDeviceDiscoveryAgentPrivate::LEnotSupported()
{
    inquiryLE.reset();
    timer->stop();

    startPending = false;
    stopPending = false;
    setError(QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError);
    emit q_ptr->error(lastError);
}

void QBluetoothDeviceDiscoveryAgentPrivate::LEdeviceFound(const QBluetoothDeviceInfo &newDeviceInfo)
{
    // Update, append or discard.
    for (int i = 0, e = discoveredDevices.size(); i < e; ++i) {
        if (discoveredDevices[i].deviceUuid() == newDeviceInfo.deviceUuid()) {
            if (discoveredDevices[i] == newDeviceInfo)
                return;

            discoveredDevices.replace(i, newDeviceInfo);
            emit q_ptr->deviceDiscovered(newDeviceInfo);
            return;
        }
    }

    discoveredDevices.append(newDeviceInfo);
    emit q_ptr->deviceDiscovered(newDeviceInfo);
}

void QBluetoothDeviceDiscoveryAgentPrivate::LEinquiryFinished()
{
    inquiryLE.reset();
    timer->stop();

    if (stopPending && !startPending) {
        stopPending = false;
        emit q_ptr->canceled();
    } else if (startPending) {
        startPending = false;
        stopPending = false;
        start();
    } else {
        emit q_ptr->finished();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::checkLETimeout()
{
    Q_ASSERT_X(inquiryLE, Q_FUNC_INFO, "LE device inquiry is nil");

    using namespace OSXBluetooth;

    const LEInquiryState state([inquiryLE inquiryState]);
    if (state == InquiryStarting || state == InquiryActive)
        return; // Wait ...

    if (state == ErrorPoweredOff)
        return LEinquiryError(QBluetoothDeviceDiscoveryAgent::PoweredOffError);

    if (state == ErrorLENotSupported)
        return LEnotSupported();

    if (state == InquiryFinished) {
        // Process found devices if any ...
        const QList<QBluetoothDeviceInfo> leDevices([inquiryLE discoveredDevices]);
        foreach (const QBluetoothDeviceInfo &info, leDevices) {
            // We were cancelled on a previous device discovered signal ...
            if (!inquiryLE)
                break;
            LEdeviceFound(info);
        }

        if (inquiryLE)
            LEinquiryFinished();
        return;
    }

    qCWarning(QT_BT_OSX) << Q_FUNC_INFO << "unexpected inquiry state in LE timeout";
    // Actually, this deserves an assert :)
}

void QBluetoothDeviceDiscoveryAgentPrivate::setError(QBluetoothDeviceDiscoveryAgent::Error error,
                                                     const QString &text)
{
    lastError = error;

    if (text.length() > 0) {
        errorString = text;
    } else {
        switch (lastError) {
        case QBluetoothDeviceDiscoveryAgent::NoError:
            errorString = QString();
            break;
        case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
            errorString = QCoreApplication::translate(DEV_DISCOVERY, DD_POWERED_OFF);
            break;
        case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
            errorString = QCoreApplication::translate(DEV_DISCOVERY, DD_INVALID_ADAPTER);
            break;
        case QBluetoothDeviceDiscoveryAgent::InputOutputError:
            errorString = QCoreApplication::translate(DEV_DISCOVERY, DD_IO);
            break;
        case QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError:
            errorString = QCoreApplication::translate(DEV_DISCOVERY, DD_NOTSUPPORTED);
            break;
        case QBluetoothDeviceDiscoveryAgent::UnknownError:
        default:
            errorString = QCoreApplication::translate(DEV_DISCOVERY, DD_UNKNOWN_ERROR);
        }
    }
}

QBluetoothDeviceDiscoveryAgent::QBluetoothDeviceDiscoveryAgent(QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothDeviceDiscoveryAgentPrivate(QBluetoothAddress(), this))
{
}

QBluetoothDeviceDiscoveryAgent::QBluetoothDeviceDiscoveryAgent(
    const QBluetoothAddress &deviceAdapter, QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothDeviceDiscoveryAgentPrivate(deviceAdapter, this))
{
    if (!deviceAdapter.isNull()) {
        qCWarning(QT_BT_OSX) << Q_FUNC_INFO << "local device address is "
                                "not available, provided address is ignored";
        d_ptr->setError(InvalidBluetoothAdapterError);
    }
}

QBluetoothDeviceDiscoveryAgent::~QBluetoothDeviceDiscoveryAgent()
{
    delete d_ptr;
}

QBluetoothDeviceDiscoveryAgent::InquiryType QBluetoothDeviceDiscoveryAgent::inquiryType() const
{
    return d_ptr->inquiryType;
}

void QBluetoothDeviceDiscoveryAgent::setInquiryType(QBluetoothDeviceDiscoveryAgent::InquiryType type)
{
    d_ptr->inquiryType = type;
}

QList<QBluetoothDeviceInfo> QBluetoothDeviceDiscoveryAgent::discoveredDevices() const
{
    return d_ptr->discoveredDevices;
}

void QBluetoothDeviceDiscoveryAgent::start()
{
    if (d_ptr->lastError != InvalidBluetoothAdapterError) {
        if (!isActive())
            d_ptr->start();
        else
            qCDebug(QT_BT_OSX) << Q_FUNC_INFO << "already started";
    }
}

void QBluetoothDeviceDiscoveryAgent::stop()
{
    if (isActive() && d_ptr->lastError != InvalidBluetoothAdapterError)
        d_ptr->stop();
}

bool QBluetoothDeviceDiscoveryAgent::isActive() const
{
    return d_ptr->isActive();
}

QBluetoothDeviceDiscoveryAgent::Error QBluetoothDeviceDiscoveryAgent::error() const
{
    return d_ptr->lastError;
}

QString QBluetoothDeviceDiscoveryAgent::errorString() const
{
    return d_ptr->errorString;
}

QT_END_NAMESPACE

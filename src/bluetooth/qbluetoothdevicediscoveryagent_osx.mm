/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbluetoothdevicediscoveryagent.h"
#include "osx/osxbtdeviceinquiry_p.h"
#include "qbluetoothlocaldevice.h"
#include "qbluetoothdeviceinfo.h"
#include "osx/osxbtutility_p.h"
#include "qbluetoothhostinfo.h"
#include "qbluetoothuuid.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>
#include <QtCore/qlist.h>

// We have to import, otherwise Apple's header is not protected
// from multiple inclusion.
#import <IOBluetooth/objc/IOBluetoothHostController.h>
#import <IOBluetooth/objc/IOBluetoothDevice.h>

// TODO: check how all these things work with threads.

QT_BEGIN_NAMESPACE

using OSXBluetooth::ObjCScopedPointer;

class QBluetoothDeviceDiscoveryAgentPrivate : public OSXBluetooth::DeviceInquiryDelegate
{
    friend class QBluetoothDeviceDiscoveryAgent;
public:
    QBluetoothDeviceDiscoveryAgentPrivate(const QBluetoothAddress & address,
                                          QBluetoothDeviceDiscoveryAgent *q);
    virtual ~QBluetoothDeviceDiscoveryAgentPrivate(); // Just to make compiler happy.

    bool isValid() const;
    bool isActive() const;

    void start();
    void stop();

private:
    // DeviceInquiryDelegate:
    void inquiryFinished(IOBluetoothDeviceInquiry *inq) Q_DECL_OVERRIDE;
    void error(IOBluetoothDeviceInquiry *inq, IOReturn error) Q_DECL_OVERRIDE;
    void deviceFound(IOBluetoothDeviceInquiry *inq, IOBluetoothDevice *device) Q_DECL_OVERRIDE;

    void setError(IOReturn error, const QString &text = QString());
    void setError(QBluetoothDeviceDiscoveryAgent::Error, const QString &text = QString());

    QBluetoothDeviceDiscoveryAgent *q_ptr;

    QBluetoothAddress adapterAddress;

    bool startPending;
    bool stopPending;

    QBluetoothDeviceDiscoveryAgent::Error lastError;
    QString errorString;

    QBluetoothDeviceDiscoveryAgent::InquiryType inquiryType;

    typedef QT_MANGLE_NAMESPACE(OSXBTDeviceInquiry) DeviceInquiryObjC;
    typedef ObjCScopedPointer<DeviceInquiryObjC> DeviceInquiry;
    DeviceInquiry inquiry;

    typedef ObjCScopedPointer<IOBluetoothHostController> HostController;
    HostController hostController; // Not sure I need it at all.

    typedef QList<QBluetoothDeviceInfo> DevicesList;
    DevicesList discoveredDevices;
};

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(const QBluetoothAddress &adapter,
                                                                             QBluetoothDeviceDiscoveryAgent *q) :
    q_ptr(q),
    adapterAddress(adapter),
    startPending(false),
    stopPending(false),
    lastError(QBluetoothDeviceDiscoveryAgent::NoError),
    inquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry)
{
    Q_ASSERT_X(q != Q_NULLPTR, "QBluetoothDeviceDiscoveryAgentPrivate()",
               "invalid q_ptr (null)");

    HostController controller([[IOBluetoothHostController defaultController] retain]);
    if (!controller) {
        qCCritical(QT_BT_OSX) << "QBluetoothDeviceDiscoveryAgentPrivate() "
                                 "no default host controller";
        setError(QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError);
        return;
    }

    DeviceInquiry newInquiry([[DeviceInquiryObjC alloc]initWithDelegate:this]);
    if (!newInquiry) { // Obj-C's way of "reporting errors":
        qCCritical(QT_BT_OSX) << "QBluetoothDeviceDiscoveryAgentPrivate() "
                                 "failed to initialize an inquiry";
        setError(QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError);
        return;
    }

    hostController.reset(controller.take());
    inquiry.reset(newInquiry.take());
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isValid() const
{
    // isValid() - Qt in general does not use exceptions, but the ctor
    // can fail to initialize some important data-members
    // (and the error is probably not even related to Bluetooth at all)
    // - say, allocation error - this is what meant here by valid/invalid.
    return hostController && inquiry;
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    if (startPending)
        return true;
    if (stopPending)
        return false;

    return [inquiry isActive];
}

void QBluetoothDeviceDiscoveryAgentPrivate::start()
{
    Q_ASSERT_X(!isActive(), "start()", "called on active device discovery agent");
    Q_ASSERT_X(lastError != QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError,
               "start()", "called with invalid bluetooth adapter");

    if (stopPending) {
        startPending = true;
        return;
    }

    discoveredDevices.clear();
    setError(QBluetoothDeviceDiscoveryAgent::NoError);

    const IOReturn res = [inquiry start];
    if (res != kIOReturnSuccess) {
        setError(res, QObject::tr("device discovery agent: failed to start"));
        emit q_ptr->error(lastError);
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    Q_ASSERT_X(isActive(), "stop()", "called whithout active inquiry");
    Q_ASSERT_X(lastError != QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError,
               "stop()", "called with invalid bluetooth adapter");

    const bool prevStart = startPending;
    startPending = false;

    setError(QBluetoothDeviceDiscoveryAgent::NoError);

    if ([inquiry isActive]) {
        stopPending = true;
        const IOReturn res = [inquiry stop];
        if (res != kIOReturnSuccess) {
            qCWarning(QT_BT_OSX) << "QBluetoothDeviceDiscoveryAgentPrivate::stop(), "
                                    "failed to stop";
            startPending = prevStart;
            stopPending = false;
            setError(res, QObject::tr("device discovery agent: failed to stop"));
            emit q_ptr->error(lastError);
        }
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::inquiryFinished(IOBluetoothDeviceInquiry *inq)
{
    Q_UNUSED(inq)

    Q_ASSERT_X(isValid(), "inquiryFinished", "invalid device discovery agent"); //We can never be here.
    Q_ASSERT_X(inq, "inquiryFinished", "invalid device inquiry (nil)");
    Q_ASSERT_X(q_ptr, "inquiryFinished", "invalid q_ptr (null)");

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

void QBluetoothDeviceDiscoveryAgentPrivate::error(IOBluetoothDeviceInquiry *inq, IOReturn error)
{
    Q_UNUSED(inq)

    Q_ASSERT_X(isValid(), "error", "invalid device discovery agent");

    startPending = false;
    stopPending = false;

    setError(error);

    Q_ASSERT_X(q_ptr, "error", "invalid q_ptr (null)");
    emit q_ptr->error(lastError);
}

void QBluetoothDeviceDiscoveryAgentPrivate::deviceFound(IOBluetoothDeviceInquiry *inq, IOBluetoothDevice *device)
{
    Q_UNUSED(inq)

    Q_ASSERT_X(isValid(), "deviceFound()",
               "invalid device discovery agent");
    Q_ASSERT_X(device, "deviceFound()", "invalid IOBluetoothDevice (nil)");

    QT_BT_MAC_AUTORELEASEPOOL;

    // Let's collect some info about this device:
    const QBluetoothAddress deviceAddress(OSXBluetooth::qt_bt_address([device getAddress]));
    if (deviceAddress.isNull()) {
        qCWarning(QT_BT_OSX) << "QBluetoothDeviceDiscoveryAgentPrivate::deviceFound(), "
                                "invalid Bluetooth address";
        return;
    }

    QString deviceName;
    if (device.name)
        deviceName = QString::fromNSString(device.name);

    const qint32 classOfDevice(device.classOfDevice);

    QBluetoothDeviceInfo deviceInfo(deviceAddress, deviceName, classOfDevice);
    deviceInfo.setCoreConfigurations(classOfDevice ? QBluetoothDeviceInfo::BaseRateCoreConfiguration:
                                                     QBluetoothDeviceInfo::LowEnergyCoreConfiguration);
    deviceInfo.setRssi(device.RSSI);

    // TODO: check if I can extract services' uuids from device.services
    // and use them.
    deviceInfo.setServiceUuids(QList<QBluetoothUuid>(),
                               QBluetoothDeviceInfo::DataIncomplete);

    for (int i = 0, e = discoveredDevices.size(); i < e; ++i) {
        if (discoveredDevices[i].address() == deviceInfo.address()) {
            if (discoveredDevices[i] == deviceInfo)
                return;
            discoveredDevices.replace(i, deviceInfo);
            emit q_ptr->deviceDiscovered(deviceInfo);
            return;
        }
    }

    discoveredDevices.append(deviceInfo);
    emit q_ptr->deviceDiscovered(deviceInfo);
}

void QBluetoothDeviceDiscoveryAgentPrivate::setError(IOReturn error, const QString &text)
{
    if (error == kIOReturnSuccess)
        setError(QBluetoothDeviceDiscoveryAgent::NoError, text);
    else if (error == kIOReturnNoPower)
        setError(QBluetoothDeviceDiscoveryAgent::PoweredOffError, text);
    else
        setError(QBluetoothDeviceDiscoveryAgent::UnknownError, text);
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
            errorString = QObject::tr("device discovery agent: adapter is powered off");
            break;
        case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
            errorString = QObject::tr("device discovery agent: invalid bluetooth adapter");
            break;
        case QBluetoothDeviceDiscoveryAgent::InputOutputError:
            errorString = QObject::tr("device discovery agent: input output error");
            break;
        case QBluetoothDeviceDiscoveryAgent::UnknownError:
        default:
            errorString = QObject::tr("device discovery agent: unknown error");
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
        const QList<QBluetoothHostInfo> localDevices = QBluetoothLocalDevice::allDevices();
        foreach (const QBluetoothHostInfo &hostInfo, localDevices) {
            if (hostInfo.address() == deviceAdapter)
                return;
        }
        d_ptr->setError(InvalidBluetoothAdapterError);
    }
}

QBluetoothDeviceDiscoveryAgent::~QBluetoothDeviceDiscoveryAgent()
{
    delete d_ptr;
}

QBluetoothDeviceDiscoveryAgent::InquiryType QBluetoothDeviceDiscoveryAgent::inquiryType() const
{
    Q_D(const QBluetoothDeviceDiscoveryAgent);
    return d->inquiryType;
}

void QBluetoothDeviceDiscoveryAgent::setInquiryType(QBluetoothDeviceDiscoveryAgent::InquiryType type)
{
    Q_D(QBluetoothDeviceDiscoveryAgent);
    d->inquiryType = type;
}

QList<QBluetoothDeviceInfo> QBluetoothDeviceDiscoveryAgent::discoveredDevices() const
{
    return d_ptr->discoveredDevices;
}

void QBluetoothDeviceDiscoveryAgent::start()
{
    if (!isActive() && d_ptr->lastError != InvalidBluetoothAdapterError)
        d_ptr->start();
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

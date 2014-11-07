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
#include "osx/osxbtledeviceinquiry_p.h"
#include "osx/osxbtdeviceinquiry_p.h"
#include "qbluetoothlocaldevice.h"
#include "osx/osxbtsdpinquiry_p.h"
#include "qbluetoothdeviceinfo.h"
#include "osx/osxbtutility_p.h"
#include "qbluetoothhostinfo.h"
#include "qbluetoothuuid.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>
#include <QtCore/qlist.h>

#include "osx/corebluetoothwrapper_p.h"

QT_BEGIN_NAMESPACE

using OSXBluetooth::ObjCScopedPointer;

class QBluetoothDeviceDiscoveryAgentPrivate : public OSXBluetooth::DeviceInquiryDelegate,
                                              public OSXBluetooth::LEDeviceInquiryDelegate
{
    friend class QBluetoothDeviceDiscoveryAgent;
public:
    QBluetoothDeviceDiscoveryAgentPrivate(const QBluetoothAddress & address,
                                          QBluetoothDeviceDiscoveryAgent *q);
    virtual ~QBluetoothDeviceDiscoveryAgentPrivate(); // Just to make compiler happy.

    bool isValid() const;
    bool isActive() const;

    void start();
    void startLE();
    void stop();

private:
    enum AgentState {
        NonActive,
        ClassicScan,
        LEScan
    };

    // DeviceInquiryDelegate:
    void inquiryFinished(IOBluetoothDeviceInquiry *inq) Q_DECL_OVERRIDE;
    void error(IOBluetoothDeviceInquiry *inq, IOReturn error) Q_DECL_OVERRIDE;
    void deviceFound(IOBluetoothDeviceInquiry *inq, IOBluetoothDevice *device) Q_DECL_OVERRIDE;
    // LEDeviceInquiryDelegate:
    void LEdeviceInquiryError(QBluetoothDeviceDiscoveryAgent::Error error) Q_DECL_OVERRIDE;
    void LEnotSupported() Q_DECL_OVERRIDE;
    void LEdeviceFound(CBPeripheral *peripheral, const QBluetoothUuid &deviceUuid,
                       NSDictionary *advertisementData, NSNumber *RSSI) Q_DECL_OVERRIDE;
    void LEdeviceInquiryFinished() Q_DECL_OVERRIDE;

    // Check if it's a really new device/updated info and emit
    // q_ptr->deviceDiscovered.
    void deviceFound(const QBluetoothDeviceInfo &newDeviceInfo);

    void setError(IOReturn error, const QString &text = QString());
    void setError(QBluetoothDeviceDiscoveryAgent::Error, const QString &text = QString());

    QBluetoothDeviceDiscoveryAgent *q_ptr;
    AgentState agentState;

    QBluetoothAddress adapterAddress;

    bool startPending;
    bool stopPending;

    QBluetoothDeviceDiscoveryAgent::Error lastError;
    QString errorString;

    QBluetoothDeviceDiscoveryAgent::InquiryType inquiryType;

    typedef ObjCScopedPointer<DeviceInquiryObjC> DeviceInquiry;
    DeviceInquiry inquiry;

    typedef ObjCScopedPointer<LEDeviceInquiryObjC> LEDeviceInquiry;
    LEDeviceInquiry inquiryLE;

    typedef ObjCScopedPointer<IOBluetoothHostController> HostController;
    HostController hostController;

    typedef QList<QBluetoothDeviceInfo> DevicesList;
    DevicesList discoveredDevices;
};

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(const QBluetoothAddress &adapter,
                                                                             QBluetoothDeviceDiscoveryAgent *q) :
    q_ptr(q),
    agentState(NonActive),
    adapterAddress(adapter),
    startPending(false),
    stopPending(false),
    lastError(QBluetoothDeviceDiscoveryAgent::NoError),
    inquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry)
{
    Q_ASSERT_X(q != Q_NULLPTR, "QBluetoothDeviceDiscoveryAgentPrivate()",
               "invalid q_ptr (null)");

    HostController controller([[IOBluetoothHostController defaultController] retain]);
    if (!controller || [controller powerState] != kBluetoothHCIPowerStateON) {
        qCCritical(QT_BT_OSX) << "QBluetoothDeviceDiscoveryAgentPrivate() "
                                 "no default host controller or adapter is off";
        return;
    }

    DeviceInquiry newInquiry([[DeviceInquiryObjC alloc]initWithDelegate:this]);
    if (!newInquiry) { // Obj-C's way of "reporting errors":
        qCCritical(QT_BT_OSX) << "QBluetoothDeviceDiscoveryAgentPrivate() "
                                 "failed to initialize an inquiry";
        return;
    }

    // OSXBTLEDeviceInquiry can be constructed even if LE is not supported -
    // at this stage it's only a memory allocation of the object itself,
    // if it fails - we have some memory-related problems.
    LEDeviceInquiry newInquiryLE([[LEDeviceInquiryObjC alloc] initWithDelegate:this]);
    if (!newInquiryLE) {
        qCWarning(QT_BT_OSX) << "QBluetoothDeviceDiscoveryAgentPrivate() "
                                "failed to initialize a LE inquiry";
        return;
    }

    qCDebug(QT_BT_OSX) << "host controller is in 'on' state, "
                          "discovery agent created successfully";

    hostController.reset(controller.take());
    inquiry.reset(newInquiry.take());
    inquiryLE.reset(newInquiryLE.take());
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isValid() const
{
    // isValid() - Qt does not use exceptions, but the ctor
    // can fail to initialize some important data-members
    // (and the error is probably not even related to Bluetooth at all)
    // - say, allocation error - this is what meant here by valid/invalid.

    const bool valid = hostController && [hostController powerState] == kBluetoothHCIPowerStateON && inquiry;
    qCDebug(QT_BT_OSX) << "private agent is valid state? "<<valid;

    if (hostController && [hostController powerState] != kBluetoothHCIPowerStateON)
        qCWarning(QT_BT_OSX) << "adapter is powered off (was on)";

    return hostController && [hostController powerState] == kBluetoothHCIPowerStateON && inquiry;
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    if (startPending) {
        qCDebug(QT_BT_OSX) << "start is pending, isActive == true";
        return true;
    }
    if (stopPending) {
        qCDebug(QT_BT_OSX) << "stop is pending, isActive == false";
        return false;
    }

    qCDebug(QT_BT_OSX)<<"isActive? "<< (agentState != NonActive);

    return agentState != NonActive;
}

void QBluetoothDeviceDiscoveryAgentPrivate::start()
{
    Q_ASSERT_X(isValid(), "start()", "called on invalid device discovery agent");
    Q_ASSERT_X(!isActive(), "start()", "called on active device discovery agent");
    Q_ASSERT_X(lastError != QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError,
               "start()", "called with an invalid Bluetooth adapter");

    if (stopPending) {
        qCDebug(QT_BT_OSX) << "START: stop is pending, set start pending and return";
        startPending = true;
        return;
    }

    agentState = ClassicScan;

    discoveredDevices.clear();
    setError(QBluetoothDeviceDiscoveryAgent::NoError);

    const IOReturn res = [inquiry start];
    if (res != kIOReturnSuccess) {
        qCDebug(QT_BT_OSX) << "START: private agent, failed to start";
        setError(res, QObject::tr("device discovery agent: failed to start"));
        agentState = NonActive;
        emit q_ptr->error(lastError);
    } else {
        qCDebug(QT_BT_OSX) << "START: device inquiry started ...";
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::startLE()
{
    Q_ASSERT_X(isValid(), "startLE()", "called on invalid device discovery agent");
    Q_ASSERT_X(lastError != QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError,
               "startLE()", "called with an invalid Bluetooth adapter");

    agentState = LEScan;

    if (![inquiryLE start]) {
        // We can be here only if we have some kind of resource allocation error, so we
        // do not emit finished, we emit error.
        qCDebug(QT_BT_OSX) << "STARTLE: failed to start LE scan ...";

        setError(QBluetoothDeviceDiscoveryAgent::UnknownError,
                 QObject::tr("device discovery agent, LE mode: "
                             "resource allocation error"));
        agentState = NonActive;
        emit q_ptr->error(lastError);
    } else {
        qCDebug(QT_BT_OSX) << "STARTLE: scan started.";
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    Q_ASSERT_X(isValid(), "stop()", "called on invalid device discovery agent");
    Q_ASSERT_X(isActive(), "stop()", "called whithout active inquiry");
    Q_ASSERT_X(lastError != QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError,
               "stop()", "called with invalid bluetooth adapter");

    const bool prevStart = startPending;
    startPending = false;
    stopPending = true;

    setError(QBluetoothDeviceDiscoveryAgent::NoError);

    if (agentState == ClassicScan) {
        const IOReturn res = [inquiry stop];
        if (res != kIOReturnSuccess) {
            qCWarning(QT_BT_OSX) << "QBluetoothDeviceDiscoveryAgentPrivate::stop(), "
                                    "failed to stop";
            startPending = prevStart;
            stopPending = false;
            setError(res, QObject::tr("device discovery agent: failed to stop"));
            emit q_ptr->error(lastError);
        } else {
            qCDebug(QT_BT_OSX) << "stop success on a classic device inquiry";
        }
    } else {
        // Can be asynchronous (depending on a status update of CBCentralManager).
        // The call itself is always 'success'.
        qCDebug(QT_BT_OSX) << "trying to stop LE scan ...";
        [inquiryLE stop];
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::inquiryFinished(IOBluetoothDeviceInquiry *inq)
{
    Q_UNUSED(inq)

    Q_ASSERT_X(isValid(), "inquiryFinished", "invalid device discovery agent"); //We can never be here.
    Q_ASSERT_X(q_ptr, "inquiryFinished", "invalid q_ptr (null)");

    // The subsequent start(LE) function (if any)
    // will (re)set the correct state.
    agentState = NonActive;

    if (stopPending && !startPending) {
        qCDebug(QT_BT_OSX) << "inquiryFinished, stop pending, no pending start, emit canceled";
        stopPending = false;
        emit q_ptr->canceled();
    } else if (startPending) {
        qCDebug(QT_BT_OSX) << "inquiryFinished, NO stop pending, pending start, re-starting";
        startPending = false;
        stopPending = false;
        start();
    } else {
        // We can be here _only_ if a classic scan
        // finished in a normal way (not cancelled).
        // startLE() will take care of old devices
        // not supporting Bluetooth 4.0.
        qCDebug(QT_BT_OSX)<<"CLASSIC inquiryFinished, NO stop pending, starting LE";
        startLE();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::error(IOBluetoothDeviceInquiry *inq, IOReturn error)
{
    Q_UNUSED(inq)

    Q_ASSERT_X(isValid(), "error", "invalid device discovery agent");

    qCDebug(QT_BT_OSX)<<"ERROR: got a native error code: "<<int(error);

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
    Q_ASSERT_X(agentState == ClassicScan, "deviceFound",
               "invalid agent state (expected classic scan)");

    QT_BT_MAC_AUTORELEASEPOOL;

    // Let's collect some info about this device:
    const QBluetoothAddress deviceAddress(OSXBluetooth::qt_address([device getAddress]));
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
    deviceInfo.setCoreConfigurations(QBluetoothDeviceInfo::BaseRateCoreConfiguration);
    deviceInfo.setRssi(device.RSSI);

    const QList<QBluetoothUuid> uuids(OSXBluetooth::extract_services_uuids(device));
    deviceInfo.setServiceUuids(uuids, QBluetoothDeviceInfo::DataIncomplete);

    deviceFound(deviceInfo);
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

    qCDebug(QT_BT_OSX) << "error set: "<<errorString;
}

void QBluetoothDeviceDiscoveryAgentPrivate::LEdeviceInquiryError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    // At the moment the only error reported can be 'powered off' error, it happens
    // after the LE scan started (so we have LE support and this is a real PoweredOffError).
    Q_ASSERT_X(error == QBluetoothDeviceDiscoveryAgent::PoweredOffError,
               "LEdeviceInquiryError", "unexpected error code");

    qCDebug(QT_BT_OSX) << "LEDeviceInquiryError: powered off";

    agentState = NonActive;
    setError(error);
    emit q_ptr->error(lastError);
}

void QBluetoothDeviceDiscoveryAgentPrivate::LEnotSupported()
{
    // Not supported is not an error.
    qCDebug(QT_BT_OSX) << "no Bluetooth LE support";
    // After we call startLE and before receive NotSupported,
    // the user can call stop (setting a pending stop).
    // So the same rule apply:

    qCDebug(QT_BT_OSX) << "LE not supported.";

    LEdeviceInquiryFinished();
}

void QBluetoothDeviceDiscoveryAgentPrivate::LEdeviceFound(CBPeripheral *peripheral, const QBluetoothUuid &deviceUuid,
                                                          NSDictionary *advertisementData,
                                                          NSNumber *RSSI)
{
    Q_ASSERT_X(peripheral, "LEdeviceFound()", "invalid peripheral (nil)");
    Q_ASSERT_X(agentState == LEScan, "LEdeviceFound",
               "invalid agent state, expected LE scan");

    Q_UNUSED(advertisementData)

    QString name;
    if (peripheral.name)
        name = QString::fromNSString(peripheral.name);

    // TODO: fix 'classOfDevice' (0 for now).
    QBluetoothDeviceInfo newDeviceInfo(deviceUuid, name, 0);
    if (RSSI)
        newDeviceInfo.setRssi([RSSI shortValue]);
    // CoreBluetooth scans only for LE devices.
    newDeviceInfo.setCoreConfigurations(QBluetoothDeviceInfo::LowEnergyCoreConfiguration);

    deviceFound(newDeviceInfo);
}

void QBluetoothDeviceDiscoveryAgentPrivate::LEdeviceInquiryFinished()
{
    // The same logic as in inquiryFinished, but does not start LE scan.
    agentState = NonActive;

    if (stopPending && !startPending) {
        qCDebug(QT_BT_OSX) << "LE scan finished, stop pending, NO start pending, emit canceled";
        stopPending = false;
        emit q_ptr->canceled();
    } else if (startPending) {
        qCDebug(QT_BT_OSX) << "LE scan finished, start pending, NO stop pending, re-start";
        startPending = false;
        stopPending = false;
        start(); //Start from a classic scan again.
    } else {
        qCDebug(QT_BT_OSX) << "LE scan finished, emit finished";
        emit q_ptr->finished();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::deviceFound(const QBluetoothDeviceInfo &newDeviceInfo)
{
    // Core Bluetooth does not allow us to access addresses, we have to use uuid instead.
    // This uuid has nothing to do with uuids in Bluetooth in general (it's generated by
    // Apple's framework using some algorithm), but it's a 128-bit uuid after all.
    const bool isLE = newDeviceInfo.coreConfigurations() == QBluetoothDeviceInfo::LowEnergyCoreConfiguration;

    for (int i = 0, e = discoveredDevices.size(); i < e; ++i) {
        if (isLE ? discoveredDevices[i].deviceUuid() == newDeviceInfo.deviceUuid():
                   discoveredDevices[i].address() == newDeviceInfo.address()) {
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
    if (d_ptr->lastError != InvalidBluetoothAdapterError) {
        if (d_ptr->isValid()) {
            qCDebug(QT_BT_OSX) << "DDA::start?";
            if (!isActive()) {
                qCDebug(QT_BT_OSX) << "DDA::start!";
                d_ptr->start();
            }
        } else {
            // We previously failed to initialize d_ptr correctly:
            // either some memory allocation problem or
            // no BT adapter found.
            qCDebug(QT_BT_OSX) << "start failed, invalid d_ptr";
            d_ptr->setError(InvalidBluetoothAdapterError);
            emit error(InvalidBluetoothAdapterError);
        }
    } else
        qCDebug(QT_BT_OSX) << "start failed, invalid adapter";
}

void QBluetoothDeviceDiscoveryAgent::stop()
{
    if (d_ptr->isValid()) {
        qCDebug(QT_BT_OSX) << "DDA::stop, is valid";
        if (isActive() && d_ptr->lastError != InvalidBluetoothAdapterError) {
            qCDebug(QT_BT_OSX) << "DDA::stop, is active and no error...";
            d_ptr->stop();
        }
    } else {
        qCDebug(QT_BT_OSX) << "DDA::stop, d_ptr is not in valid state, can not stop";
    }
}

bool QBluetoothDeviceDiscoveryAgent::isActive() const
{
    qCDebug(QT_BT_OSX) << "DDA::isActive";
    if (d_ptr->isValid()) {
        return d_ptr->isActive();
    } else {
        qCDebug(QT_BT_OSX) << "DDA::isActive, d_ptr is invalid";
    }
    return false;
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

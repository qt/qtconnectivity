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

#include "qbluetoothservicediscoveryagent.h"
#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothlocaldevice.h"
#include "osx/osxbtsdpinquiry_p.h"
#include "qbluetoothhostinfo.h"
#include "osx/osxbtutility_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>
#include <QtCore/qlist.h>

// We have to import obj-C headers, they are not guarded against a multiple inclusion.
#import <IOBluetooth/objc/IOBluetoothSDPServiceRecord.h>
#import <IOBluetooth/objc/IOBluetoothHostController.h>
#import <IOBluetooth/objc/IOBluetoothDevice.h>

QT_BEGIN_NAMESPACE

class QBluetoothServiceDiscoveryAgentPrivate : public QObject, public OSXBluetooth::SDPInquiryDelegate
{
    friend class QBluetoothServiceDiscoveryAgent;
public:
    enum DiscoveryState {
        Inactive,
        DeviceDiscovery,
        ServiceDiscovery,
    };

    QBluetoothServiceDiscoveryAgentPrivate(const QBluetoothAddress &localAddress);

    void startDeviceDiscovery();
    void stopDeviceDiscovery();

    void startServiceDiscovery();
    void stopServiceDiscovery();

    DiscoveryState discoveryState();
    void setDiscoveryMode(QBluetoothServiceDiscoveryAgent::DiscoveryMode m);
    QBluetoothServiceDiscoveryAgent::DiscoveryMode DiscoveryMode();

    void _q_deviceDiscovered(const QBluetoothDeviceInfo &info);
    void _q_deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error);
    void _q_deviceDiscoveryFinished();
    void _q_serviceDiscoveryFinished();


private:
    // SDPInquiryDelegate:
    void SDPInquiryFinished(IOBluetoothDevice *device) Q_DECL_OVERRIDE;
    void SDPInquiryError(IOBluetoothDevice *device, IOReturn errorCode) Q_DECL_OVERRIDE;

    void performMinimalServiceDiscovery(const QBluetoothAddress &deviceAddress);
    void setupDeviceDiscoveryAgent();
    bool isDuplicatedService(const QBluetoothServiceInfo &serviceInfo) const;
    void serviceDiscoveryFinished();

    QBluetoothServiceDiscoveryAgent *q_ptr;

    QBluetoothServiceDiscoveryAgent::Error error;
    QString errorString;

    QList<QBluetoothDeviceInfo> discoveredDevices;
    QList<QBluetoothServiceInfo> discoveredServices;
    QList<QBluetoothUuid> uuidFilter;

    bool singleDevice;
    QBluetoothAddress deviceAddress;
    QBluetoothAddress localAdapterAddress;

    DiscoveryState state;
    QBluetoothServiceDiscoveryAgent::DiscoveryMode discoveryMode;

    QScopedPointer<QBluetoothDeviceDiscoveryAgent> deviceDiscoveryAgent;
    OSXBluetooth::ObjCScopedPointer<ObjCServiceInquiry> serviceInquiry;
};

QBluetoothServiceDiscoveryAgentPrivate::QBluetoothServiceDiscoveryAgentPrivate(const QBluetoothAddress &localAddress) :
    q_ptr(0),
    error(QBluetoothServiceDiscoveryAgent::NoError),
    singleDevice(false),
    localAdapterAddress(localAddress),
    state(Inactive),
    discoveryMode(QBluetoothServiceDiscoveryAgent::MinimalDiscovery)
{
    serviceInquiry.reset([[ObjCServiceInquiry alloc] initWithDelegate:this]);
}

void QBluetoothServiceDiscoveryAgentPrivate::startDeviceDiscovery()
{
    Q_ASSERT_X(q_ptr, "startDeviceDiscovery()", "invalid q_ptr (null)");
    Q_ASSERT_X(state == Inactive, "startDeviceDiscovery()", "invalid state");
    Q_ASSERT_X(error != QBluetoothServiceDiscoveryAgent::InvalidBluetoothAdapterError,
               "startDeviceDiscovery()", "invalid bluetooth adapter");

    Q_ASSERT_X(deviceDiscoveryAgent.isNull(), "startDeviceDiscovery()",
               "discovery agent already exists");

    state = DeviceDiscovery;

    setupDeviceDiscoveryAgent();
    deviceDiscoveryAgent->start();
}

void QBluetoothServiceDiscoveryAgentPrivate::stopDeviceDiscovery()
{
    Q_ASSERT_X(q_ptr, "stopDeviceDiscovery()", "invalid q_ptr (null)");
    Q_ASSERT_X(!deviceDiscoveryAgent.isNull(), "stopDeviceDiscovery()",
               "invalid device discovery agent (null)");
    Q_ASSERT_X(state == DeviceDiscovery, "stopDeviceDiscovery()",
               "invalid state");

    deviceDiscoveryAgent->stop();
    deviceDiscoveryAgent.reset(Q_NULLPTR);
    state = Inactive;

    emit q_ptr->canceled();
}

void QBluetoothServiceDiscoveryAgentPrivate::startServiceDiscovery()
{
    // Any of 'Inactive'/'DeviceDiscovery'/'ServiceDiscovery' states
    // are possible.

    Q_ASSERT_X(q_ptr, "startServiceDiscovery()", "invalid q_ptr (null)");
    Q_ASSERT_X(error != QBluetoothServiceDiscoveryAgent::InvalidBluetoothAdapterError,
               "startServiceDiscovery()", "invalid bluetooth adapter");

    if (discoveredDevices.isEmpty()) {
        state = Inactive;
        emit q_ptr->finished();
        return;
    }

    QT_BT_MAC_AUTORELEASEPOOL;

    state = ServiceDiscovery;
    const QBluetoothAddress &address(discoveredDevices.at(0).address());

    // Autoreleased object.
    IOBluetoothHostController *const hc = [IOBluetoothHostController defaultController];
    if (![hc powerState]) {
        discoveredDevices.clear();
        if (singleDevice) {
            error = QBluetoothServiceDiscoveryAgent::PoweredOffError;
            errorString = QBluetoothServiceDiscoveryAgent::tr("Local device is powered off");
            emit q_ptr->error(error);
        }

        return serviceDiscoveryFinished();
    }

    if (DiscoveryMode() == QBluetoothServiceDiscoveryAgent::MinimalDiscovery) {
        performMinimalServiceDiscovery(address);
    } else {
        uuidFilter.size() ? [serviceInquiry performSDPQueryWithDevice:address filters:uuidFilter]
                          : [serviceInquiry performSDPQueryWithDevice:address];
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::stopServiceDiscovery()
{
    Q_ASSERT_X(state != Inactive, "stopServiceDiscovery()", "invalid state");
    Q_ASSERT_X(q_ptr, "stopServiceDiscovery()", "invalid q_ptr (null)");

    discoveredDevices.clear();
    state = Inactive;

    // "Stops" immediately.
    [serviceInquiry stopSDPQuery];

    emit q_ptr->canceled();
}

QBluetoothServiceDiscoveryAgentPrivate::DiscoveryState
    QBluetoothServiceDiscoveryAgentPrivate::discoveryState()
{
    return state;
}

void QBluetoothServiceDiscoveryAgentPrivate::setDiscoveryMode(
    QBluetoothServiceDiscoveryAgent::DiscoveryMode m)
{
    discoveryMode = m;

}

QBluetoothServiceDiscoveryAgent::DiscoveryMode
    QBluetoothServiceDiscoveryAgentPrivate::DiscoveryMode()
{
    return discoveryMode;
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_deviceDiscovered(const QBluetoothDeviceInfo &info)
{
    // Look for duplicates, and cached entries
    for (int i = 0; i < discoveredDevices.count(); i++) {
        if (discoveredDevices.at(i).address() == info.address()) {
            discoveredDevices.removeAt(i);
            break;
        }
    }

    discoveredDevices.prepend(info);
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error)
{
    Q_ASSERT_X(q_ptr, "_q_deviceDiscoveryError()", "invalid q_ptr (null)");

    error = QBluetoothServiceDiscoveryAgent::UnknownError;
    errorString = tr("Unknown error while scanning for devices");

    deviceDiscoveryAgent->stop();
    deviceDiscoveryAgent.reset(Q_NULLPTR);

    state = QBluetoothServiceDiscoveryAgentPrivate::Inactive;
    emit q_ptr->error(error);
    emit q_ptr->finished();
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_deviceDiscoveryFinished()
{
    Q_ASSERT_X(q_ptr, "_q_deviceDiscoveryFinished()",
               "invalid q_ptr (null)");

    if (deviceDiscoveryAgent->error() != QBluetoothDeviceDiscoveryAgent::NoError) {
        //Forward the device discovery error
        error = static_cast<QBluetoothServiceDiscoveryAgent::Error>(deviceDiscoveryAgent->error());
        errorString = deviceDiscoveryAgent->errorString();
        deviceDiscoveryAgent.reset(Q_NULLPTR);
        state = Inactive;
        emit q_ptr->error(error);
        emit q_ptr->finished();
    } else {
        deviceDiscoveryAgent.reset(Q_NULLPTR);
        startServiceDiscovery();
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_serviceDiscoveryFinished()
{
    // See SDPInquiryFinished.
}

void QBluetoothServiceDiscoveryAgentPrivate::SDPInquiryFinished(IOBluetoothDevice *device)
{
    Q_ASSERT_X(device, "SDPInquiryFinished()", "invalid IOBluetoothDevice (nil)");

    if (state == Inactive)
        return;

    QT_BT_MAC_AUTORELEASEPOOL;

    NSArray *const records = device.services;
    for (IOBluetoothSDPServiceRecord *record in records) {
        QBluetoothServiceInfo serviceInfo;
        Q_ASSERT_X(discoveredDevices.size() >= 1, "SDPInquiryFinished()",
                   "invalid number of devices");

        serviceInfo.setDevice(discoveredDevices.at(0));
        OSXBluetooth::extract_service_record(record, serviceInfo);

        if (!serviceInfo.isValid())
            continue;

        if (!isDuplicatedService(serviceInfo)) {
            discoveredServices.append(serviceInfo);
            emit q_ptr->serviceDiscovered(serviceInfo);
        }
    }

    serviceDiscoveryFinished();
}

void QBluetoothServiceDiscoveryAgentPrivate::SDPInquiryError(IOBluetoothDevice *device, IOReturn errorCode)
{
    Q_UNUSED(device)
    Q_UNUSED(errorCode)

    discoveredDevices.clear();
    // TODO: find a better mapping from IOReturn to QBluetoothServiceDiscoveryAgent::Error.
    if (singleDevice) {
        error = QBluetoothServiceDiscoveryAgent::UnknownError;
        errorString = QObject::tr("service discovery agent: unknown error");
        emit q_ptr->error(error);
    }

    serviceDiscoveryFinished();
}

void QBluetoothServiceDiscoveryAgentPrivate::performMinimalServiceDiscovery(const QBluetoothAddress &deviceAddress)
{
    Q_ASSERT_X(!deviceAddress.isNull(), "performMinimalServiceDiscovery()",
               "invalid device address");

    QT_BT_MAC_AUTORELEASEPOOL;

    const BluetoothDeviceAddress iobtAddress = OSXBluetooth::iobluetooth_address(deviceAddress);
    IOBluetoothDevice *const device = [IOBluetoothDevice deviceWithAddress:&iobtAddress];
    if (!device || !device.services) {
        if (singleDevice) {
            error = QBluetoothServiceDiscoveryAgent::UnknownError;
            errorString = tr("service discovery agent: minimal service discovery failed");
            emit q_ptr->error(error);
        }
    } else {

        NSArray *const records = device.services;
        for (IOBluetoothSDPServiceRecord *record in records) {
            QBluetoothServiceInfo serviceInfo;
            Q_ASSERT_X(discoveredDevices.size() >= 1, "SDPInquiryFinished()",
                       "invalid number of devices");

            serviceInfo.setDevice(discoveredDevices.at(0));
            OSXBluetooth::extract_service_record(record, serviceInfo);

            if (!serviceInfo.isValid())
                continue;

            if (!uuidFilter.isEmpty() && !uuidFilter.contains(serviceInfo.serviceUuid()))
                continue;

            if (!isDuplicatedService(serviceInfo)) {
                discoveredServices.append(serviceInfo);
                emit q_ptr->serviceDiscovered(serviceInfo);
            }
        }
    }

    serviceDiscoveryFinished();
}

void QBluetoothServiceDiscoveryAgentPrivate::setupDeviceDiscoveryAgent()
{
    Q_ASSERT_X(q_ptr, "setupDeviceDiscoveryAgent()",
               "invalid q_ptr (null)");
    Q_ASSERT_X(deviceDiscoveryAgent.isNull() || !deviceDiscoveryAgent->isActive(),
               "setupDeviceDiscoveryAgent()",
               "device discovery agent is active");

    deviceDiscoveryAgent.reset(new QBluetoothDeviceDiscoveryAgent(localAdapterAddress, q_ptr));

    QObject::connect(deviceDiscoveryAgent.data(), SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo &)),
                     q_ptr, SLOT(_q_deviceDiscovered(const QBluetoothDeviceInfo &)));
    QObject::connect(deviceDiscoveryAgent.data(), SIGNAL(finished()),
                     q_ptr, SLOT(_q_deviceDiscoveryFinished()));
    QObject::connect(deviceDiscoveryAgent.data(), SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
                     q_ptr, SLOT(_q_deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error)));
}

bool QBluetoothServiceDiscoveryAgentPrivate::isDuplicatedService(const QBluetoothServiceInfo &serviceInfo) const
{
    //check the service is not already part of our known list
    for (int j = 0; j < discoveredServices.count(); j++) {
        const QBluetoothServiceInfo &info = discoveredServices.at(j);
        if (info.device() == serviceInfo.device()
                && info.serviceClassUuids() == serviceInfo.serviceClassUuids()
                && info.serviceUuid() == serviceInfo.serviceUuid()) {
            return true;
        }
    }

    return false;
}

void QBluetoothServiceDiscoveryAgentPrivate::serviceDiscoveryFinished()
{
    if (!discoveredDevices.isEmpty())
        discoveredDevices.removeFirst();

    if (state == ServiceDiscovery)
        startServiceDiscovery();
}

QBluetoothServiceDiscoveryAgent::QBluetoothServiceDiscoveryAgent(QObject *parent)
: QObject(parent), d_ptr(new QBluetoothServiceDiscoveryAgentPrivate(QBluetoothAddress()))
{
    d_ptr->q_ptr = this;
}

QBluetoothServiceDiscoveryAgent::QBluetoothServiceDiscoveryAgent(const QBluetoothAddress &deviceAdapter, QObject *parent)
: QObject(parent), d_ptr(new QBluetoothServiceDiscoveryAgentPrivate(deviceAdapter))
{
    d_ptr->q_ptr = this;
    if (!deviceAdapter.isNull()) {
        const QList<QBluetoothHostInfo> localDevices = QBluetoothLocalDevice::allDevices();
        foreach (const QBluetoothHostInfo &hostInfo, localDevices) {
            if (hostInfo.address() == deviceAdapter)
                return;
        }
        d_ptr->error = InvalidBluetoothAdapterError;
        d_ptr->errorString = tr("Invalid Bluetooth adapter address");
    }
}

QBluetoothServiceDiscoveryAgent::~QBluetoothServiceDiscoveryAgent()
{
    delete d_ptr;
}

QList<QBluetoothServiceInfo> QBluetoothServiceDiscoveryAgent::discoveredServices() const
{
    return d_ptr->discoveredServices;
}

/*!
    Sets the UUID filter to \a uuids.  Only services matching the UUIDs in \a uuids will be
    returned.

    An empty UUID list is equivalent to a list containing only QBluetoothUuid::PublicBrowseGroup.

    \sa uuidFilter()
*/
void QBluetoothServiceDiscoveryAgent::setUuidFilter(const QList<QBluetoothUuid> &uuids)
{
    d_ptr->uuidFilter = uuids;
}

/*!
    This is an overloaded member function, provided for convenience.

    Sets the UUID filter to a list containing the single element \a uuid.

    \sa uuidFilter()
*/
void QBluetoothServiceDiscoveryAgent::setUuidFilter(const QBluetoothUuid &uuid)
{
    d_ptr->uuidFilter.clear();
    d_ptr->uuidFilter.append(uuid);
}

/*!
    Returns the UUID filter.

    \sa setUuidFilter()
*/
QList<QBluetoothUuid> QBluetoothServiceDiscoveryAgent::uuidFilter() const
{
    return d_ptr->uuidFilter;
}

/*!
    Sets the remote device address to \a address. If \a address is default constructed,
    services will be discovered on all contactable Bluetooth devices. A new remote
    address can only be set while there is no service discovery in progress; otherwise
    this function returns false.

    On some platforms such as Blackberry the service discovery might lead to pairing requests.
    Therefore it is not recommended to do service discoveries on all devices.

    \sa remoteAddress()
*/
bool QBluetoothServiceDiscoveryAgent::setRemoteAddress(const QBluetoothAddress &address)
{
    if (isActive())
        return false;

    if (!address.isNull())
        d_ptr->singleDevice = true;

    d_ptr->deviceAddress = address;
    return true;
}

QBluetoothAddress QBluetoothServiceDiscoveryAgent::remoteAddress() const
{
    if (d_ptr->singleDevice)
        return d_ptr->deviceAddress;

    return QBluetoothAddress();
}

void QBluetoothServiceDiscoveryAgent::start(DiscoveryMode mode)
{
    if (d_ptr->discoveryState() == QBluetoothServiceDiscoveryAgentPrivate::Inactive
        && d_ptr->error != InvalidBluetoothAdapterError)
    {
        d_ptr->setDiscoveryMode(mode);
        if (d_ptr->deviceAddress.isNull()) {
            d_ptr->startDeviceDiscovery();
        } else {
            d_ptr->discoveredDevices.append(QBluetoothDeviceInfo(d_ptr->deviceAddress, QString(), 0));
            d_ptr->startServiceDiscovery();
        }
    }
}

void QBluetoothServiceDiscoveryAgent::stop()
{
    if (d_ptr->error == InvalidBluetoothAdapterError || !isActive())
        return;

    switch (d_ptr->discoveryState()) {
    case QBluetoothServiceDiscoveryAgentPrivate::DeviceDiscovery:
        d_ptr->stopDeviceDiscovery();
        break;
    case QBluetoothServiceDiscoveryAgentPrivate::ServiceDiscovery:
        d_ptr->stopServiceDiscovery();
    default:;
    }

    d_ptr->discoveredDevices.clear();
}

void QBluetoothServiceDiscoveryAgent::clear()
{
    // Don't clear the list while the search is ongoing
    if (isActive())
        return;

    d_ptr->discoveredDevices.clear();
    d_ptr->discoveredServices.clear();
    d_ptr->uuidFilter.clear();
}

bool QBluetoothServiceDiscoveryAgent::isActive() const
{
    return d_ptr->state != QBluetoothServiceDiscoveryAgentPrivate::Inactive;
}

QBluetoothServiceDiscoveryAgent::Error QBluetoothServiceDiscoveryAgent::error() const
{
    return d_ptr->error;
}

QString QBluetoothServiceDiscoveryAgent::errorString() const
{
    return d_ptr->errorString;
}

#include "moc_qbluetoothservicediscoveryagent.cpp"

QT_END_NAMESPACE

/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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
#include "qbluetoothservicediscoveryagent_p.h"

#include "qbluetoothdevicediscoveryagent.h"

QT_BEGIN_NAMESPACE

/*!
    \class QBluetoothServiceDiscoveryAgent
    \inmodule QtBluetooth
    \brief The QBluetoothServiceDiscoveryAgent class enables you to query for
    Bluetooth services.

    To query the services provided by all contactable Bluetooth devices:
    \list
    \li create an instance of QBluetoothServiceDiscoveryAgent,
    \li connect to either the serviceDiscovered() or finished() signals,
    \li and call start().
    \endlist

    \snippet doc_src_qtbluetooth.cpp discovery

    By default a minimal service discovery is performed. In this mode, the QBluetotohServiceInfo
    objects returned are guaranteed to contain only device and service UUID information. Depending
    on platform and device capabilities, other service information may also be available. For most
    use cases this is adequate as QBluetoothSocket::connectToService() will perform additional
    discovery if required.  If full service information is required, pass \l FullDiscovery as the
    discoveryMode parameter to start().
*/

/*!
    \enum QBluetoothServiceDiscoveryAgent::Error

    This enum describes errors that can occur during service discovery.

    \value NoError          No error has occurred.
    \value PoweredOffError  The Bluetooth adaptor is powered off, power it on before doing discovery.
    \value InputOutputError    Writing or reading from the device resulted in an error.
    \value UnknownError     An unknown error has occurred.
*/

/*!
    \enum QBluetoothServiceDiscoveryAgent::DiscoveryMode

    This enum describes the service discovery mode.

    \value MinimalDiscovery     Performs a minimal service discovery. The QBluetoothServiceInfo
    objects returned may be incomplete and are only guaranteed to contain device and service UUID information.
    \value FullDiscovery        Performs a full service discovery.
*/

/*!
    \fn QBluetoothServiceDiscoveryAgent::serviceDiscovered(const QBluetoothServiceInfo &info)

    This signal is emitted when the Bluetooth service described by \a info is discovered.
*/

/*!
    \fn QBluetoothServiceDiscoveryAgent::finished()

    This signal is emitted when Bluetooth service discovery completes. This signal will even
    be emitted when an error occurred during the service discovery.
*/

/*!
    \fn void QBluetoothServiceDiscoveryAgent::error(QBluetoothServiceDiscoveryAgent::Error error)

    This signal is emitted when an error occurs. The \a error parameter describes the error that
    occurred.
*/

/*!
    Constructs a new QBluetoothServiceDiscoveryAgent with \a parent. Services will be discovered on all
    contactable devices.
*/
QBluetoothServiceDiscoveryAgent::QBluetoothServiceDiscoveryAgent(QObject *parent)
: QObject(parent), d_ptr(new QBluetoothServiceDiscoveryAgentPrivate(QBluetoothAddress()))
{
     d_ptr->q_ptr = this;
}

/*!
    Constructs a new QBluetoothServiceDiscoveryAgent for \a deviceAdapter and with \a parent.

    If \a deviceAdapter is null, the default adapter will be used.
*/
QBluetoothServiceDiscoveryAgent::QBluetoothServiceDiscoveryAgent(const QBluetoothAddress &deviceAdapter, QObject *parent)
: QObject(parent), d_ptr(new QBluetoothServiceDiscoveryAgentPrivate(deviceAdapter))
{
    d_ptr->q_ptr = this;
}

/*!

  Destructor for QBluetoothServiceDiscoveryAgent

*/

QBluetoothServiceDiscoveryAgent::~QBluetoothServiceDiscoveryAgent()
{
    delete d_ptr;
}

/*!
    Returns the list of all discovered services.
*/
QList<QBluetoothServiceInfo> QBluetoothServiceDiscoveryAgent::discoveredServices() const
{
    Q_D(const QBluetoothServiceDiscoveryAgent);

    return d->discoveredServices;
}
/*!
    Sets the UUID filter to \a uuids.  Only services matching the UUIDs in \a uuids will be
    returned.

    An empty UUID list is equivalent to a list containing only QBluetoothUuid::PublicBrowseGroup.

    \sa uuidFilter()
*/
void QBluetoothServiceDiscoveryAgent::setUuidFilter(const QList<QBluetoothUuid> &uuids)
{
    Q_D(QBluetoothServiceDiscoveryAgent);

    d->uuidFilter = uuids;
}

/*!
    This is an overloaded member function, provided for convenience.

    Sets the UUID filter to a list containing the single element \a uuid.

    \sa uuidFilter()
*/
void QBluetoothServiceDiscoveryAgent::setUuidFilter(const QBluetoothUuid &uuid)
{
    Q_D(QBluetoothServiceDiscoveryAgent);

    d->uuidFilter.clear();
    d->uuidFilter.append(uuid);
}

/*!
    Returns the UUID filter.

    \sa setUuidFilter()
*/
QList<QBluetoothUuid> QBluetoothServiceDiscoveryAgent::uuidFilter() const
{
    Q_D(const QBluetoothServiceDiscoveryAgent);

    return d->uuidFilter;
}

/*!
    Sets remote device address to \a address. If \a address is null, services will be discovered
    on all contactable Bluetooth devices. A new remote address can only be set while there is
    no service discovery in progress; otherwise this function returns false.

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

/*!
    Returns the remote device address. If setRemoteAddress is not called, the function
    will return default QBluetoothAddress.

*/
QBluetoothAddress QBluetoothServiceDiscoveryAgent::remoteAddress() const
{
    if (d_ptr->singleDevice == true)
        return d_ptr->deviceAddress;
    else
        return QBluetoothAddress();
}

/*!
    Starts service discovery. \a mode specifies the type of service discovery to perform.

    On BlackBerry devices, device discovery may lead to pairing requests.

    \sa DiscoveryMode
*/
void QBluetoothServiceDiscoveryAgent::start(DiscoveryMode mode)
{
    Q_D(QBluetoothServiceDiscoveryAgent);

    if (d->discoveryState() == QBluetoothServiceDiscoveryAgentPrivate::Inactive) {
        d->setDiscoveryMode(mode);
        if (d->deviceAddress.isNull()) {
            d->startDeviceDiscovery();
        } else {
            d->discoveredDevices << QBluetoothDeviceInfo(d->deviceAddress, QString(), 0);
            d->startServiceDiscovery();
        }
    }
}

/*!
    Stops service discovery.
*/
void QBluetoothServiceDiscoveryAgent::stop()
{
    Q_D(QBluetoothServiceDiscoveryAgent);

    switch (d->discoveryState()) {
    case QBluetoothServiceDiscoveryAgentPrivate::DeviceDiscovery:
        d->stopDeviceDiscovery();
        break;
    case QBluetoothServiceDiscoveryAgentPrivate::ServiceDiscovery:
        d->stopServiceDiscovery();
    default:
        ;
    }

    d->discoveredDevices.clear();
}

/*!
    Clears the results of a previous service discovery.
*/
void QBluetoothServiceDiscoveryAgent::clear()
{
    Q_D(QBluetoothServiceDiscoveryAgent);

    d->discoveredDevices.clear();
    d->discoveredServices.clear();
    d->uuidFilter.clear();
}

/*!
    Returns true if service discovery is currently active, otherwise returns false.
*/
bool QBluetoothServiceDiscoveryAgent::isActive() const
{
    Q_D(const QBluetoothServiceDiscoveryAgent);

    return d->state != QBluetoothServiceDiscoveryAgentPrivate::Inactive;
}

/*!
    Returns the type of error that last occurred. If service discovery is done
    on a signle address it will return errors that occured while trying to discover
    services on that device. If the alternate constructor is used and devices are
    discovered by a scan, errors during service discovery on individual
    devices are not saved and no signals are emitted. In this case, errors are
    fairly normal as some devices may not respond to discovery or
    may no longer be in range.  Such errors are surpressed.  If no services
    are returned, it can be assumed no services could be discovered.

*/
QBluetoothServiceDiscoveryAgent::Error QBluetoothServiceDiscoveryAgent::error() const
{
    Q_D(const QBluetoothServiceDiscoveryAgent);

    return d->error;
}

/*!
    Returns a human-readable description of the last error that occurred during
    service discovery on a single device.
*/
QString QBluetoothServiceDiscoveryAgent::errorString() const
{
    Q_D(const QBluetoothServiceDiscoveryAgent);
    return d->errorString;
}


/*!
    \fn QBluetoothServiceDiscoveryAgent::canceled()
    Signals the cancellation of the service discovery.
 */


/*!
    Starts device discovery.
*/
void QBluetoothServiceDiscoveryAgentPrivate::startDeviceDiscovery()
{
    Q_Q(QBluetoothServiceDiscoveryAgent);

    if (!deviceDiscoveryAgent) {
#ifdef QT_BLUEZ_BLUETOOTH
        deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(m_deviceAdapterAddress, q);
#else
        deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(q);
#endif
        QObject::connect(deviceDiscoveryAgent, SIGNAL(finished()),
                         q, SLOT(_q_deviceDiscoveryFinished()));
        QObject::connect(deviceDiscoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
                         q, SLOT(_q_deviceDiscovered(QBluetoothDeviceInfo)));
        QObject::connect(deviceDiscoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
                         q, SLOT(_q_deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error)));

    }

    setDiscoveryState(DeviceDiscovery);

    deviceDiscoveryAgent->start();
}

/*!
    Stops device discovery.
*/
void QBluetoothServiceDiscoveryAgentPrivate::stopDeviceDiscovery()
{
    deviceDiscoveryAgent->stop();
    delete deviceDiscoveryAgent;
    deviceDiscoveryAgent = 0;

    setDiscoveryState(Inactive);

    Q_Q(QBluetoothServiceDiscoveryAgent);
    emit q->canceled();
}

/*!
    Called when device discovery finishes.
*/
void QBluetoothServiceDiscoveryAgentPrivate::_q_deviceDiscoveryFinished()
{
    if (deviceDiscoveryAgent->error() != QBluetoothDeviceDiscoveryAgent::NoError) {
        //Forward the device discovery error
        error = static_cast<QBluetoothServiceDiscoveryAgent::Error>(deviceDiscoveryAgent->error());
        errorString = deviceDiscoveryAgent->errorString();
        setDiscoveryState(Inactive);
        Q_Q(QBluetoothServiceDiscoveryAgent);
        emit q->error(error);
        emit q->finished();
        return;
    }

//    discoveredDevices = deviceDiscoveryAgent->discoveredDevices();

    delete deviceDiscoveryAgent;
    deviceDiscoveryAgent = 0;

    startServiceDiscovery();
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_deviceDiscovered(const QBluetoothDeviceInfo &info)
{
    if(mode == QBluetoothServiceDiscoveryAgent::FullDiscovery) {
        // look for duplicates, and cached entries
        for(int i = 0; i < discoveredDevices.count(); i++){
            if(discoveredDevices.at(i).address() == info.address()){
                discoveredDevices.removeAt(i);
            }
        }
        discoveredDevices.prepend(info);
    }
    else {
        for(int i = 0; i < discoveredDevices.count(); i++){
            if(discoveredDevices.at(i).address() == info.address()){
                discoveredDevices.removeAt(i);
            }
        }
        discoveredDevices.prepend(info);
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error newError)
{
    error = static_cast<QBluetoothServiceDiscoveryAgent::Error>(newError);
    errorString = deviceDiscoveryAgent->errorString();

    deviceDiscoveryAgent->stop();
    delete deviceDiscoveryAgent;
    deviceDiscoveryAgent = 0;

    setDiscoveryState(Inactive);
    Q_Q(QBluetoothServiceDiscoveryAgent);
    emit q->error(error);
    emit q->finished();
}

/*!
    Starts service discovery for the next device.
*/
void QBluetoothServiceDiscoveryAgentPrivate::startServiceDiscovery()
{
    Q_Q(QBluetoothServiceDiscoveryAgent);

    if (discoveredDevices.isEmpty()) {
        setDiscoveryState(Inactive);
        emit q->finished();
        return;
    }

    setDiscoveryState(ServiceDiscovery);
    start(discoveredDevices.at(0).address());
}

/*!
    Stops service discovery.
*/
void QBluetoothServiceDiscoveryAgentPrivate::stopServiceDiscovery()
{
    stop();

    setDiscoveryState(Inactive);
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_serviceDiscoveryFinished()
{
    if(!discoveredDevices.isEmpty()) {
        discoveredDevices.removeFirst();
    }

    startServiceDiscovery();
}

#include "moc_qbluetoothservicediscoveryagent.cpp"

QT_END_NAMESPACE

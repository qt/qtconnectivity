/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothdevicediscoveryagent_p.h"

QTBLUETOOTH_BEGIN_NAMESPACE

/*!
    \class QBluetoothDeviceDiscoveryAgent
    \brief The QBluetoothDeviceDiscoveryAgent class provides an API for discovering nearby
           Bluetooth devices.
    \since 5.0

    \ingroup connectivity-bluetooth
    \inmodule QtConnectivity

    To discovery nearby Bluetooth devices create an instance of QBluetoothDeviceDiscoveryAgent,
    connect to either the deviceDiscovered() or finished() signals and call start().

    \snippet snippets/connectivity/devicediscovery.cpp Device discovery

    To retrieve results asynchronously connect to the deviceDiscovered() signal. To get a list of
    all discovered devices call discoveredDevices() after the finished() signal is emitted.
*/

/*!
    \enum QBluetoothDeviceDiscoveryAgent::Error

    Indicates all possible error conditions found during Bluetooth device discovery.

    \value NoError          No error has occurred.
    \value PoweredOff       Bluetooth adaptor is powered off, power it on before doing discovery.
    \value IOFailure        Writing or reading from device resulted in an error.
    \value UnknownError     An unknown error has occurred.
*/

/*!
    \enum QBluetoothDeviceDiscoveryAgent::InquiryType

    This enum describes the inquiry type used when discovering Bluetooth devices.

    \value GeneralUnlimitedInquiry  A general unlimited inquiry. Discovers all visible Bluetooth
                                    devices in the local vicinity.
    \value LimitedInquiry           A limited inquiry. Only discovers devices that are in limited
                                    inquiry mode. Not all platforms support limited inquiry. If
                                    limited inquiry is requested on a platform that does not
                                    support it general unlimited inquiry we be used instead. Setting
                                    LimitedInquiry is useful for 2 games that wish to find each other
                                    quickly.  The phone scans for devices in LimitedInquiry and
                                    Service Discovery is only done on one or two devices speeding up the
                                    service scan.  After the game has connected the device returns to
                                    GeneralUnilimitedInquiry
*/

/*!
    \fn void QBluetoothDeviceDiscoveryAgent::deviceDiscovered(const QBluetoothDeviceInfo &info)

    This signal is emitted when the Bluetooth device described by \a info is discovered.
*/

/*!
    \fn void QBluetoothDeviceDiscoveryAgent::finished()

    This signal is emitted when Bluetooth device discovery completes.
*/

/*!
    \fn void QBluetoothDeviceDiscoveryAgent::error(QBluetoothDeviceDiscoveryAgent::Error error)

    This signal is emitted when an \a error occurs during Bluetooth device discovery.

    \sa error(), errorString()
*/

/*!
    \fn void QBluetoothDeviceDiscoveryAgent::canceled()

    This signal is emitted when device discovery is aborted by a call to stop().
*/

/*!
    \fn bool QBluetoothDeviceDiscoveryAgent::isActive() const

    Returns true if the agent is currently discovering Bluetooth devices, other returns false.
*/

/*!
    Constructs a new Bluetooth device discovery agent with parent \a parent.
*/
QBluetoothDeviceDiscoveryAgent::QBluetoothDeviceDiscoveryAgent(QObject *parent)
: QObject(parent), d_ptr(new QBluetoothDeviceDiscoveryAgentPrivate)
{
    d_ptr->q_ptr = this;
}

/*!
  Destructor for ~QBluetoothDeviceDiscoveryAgent()
*/
QBluetoothDeviceDiscoveryAgent::~QBluetoothDeviceDiscoveryAgent()
{
    delete d_ptr;
}

/*!
    \property QBluetoothDeviceDiscoveryAgent::inquiryType
    \brief type of inquiry scan to use when discovering devices

    This property affects the type of inquiry scan which is performed when discovering devices.

    By default, this property is set to GeneralUnlimitedInquiry.

    Not all platforms support LimitedInquiry.

    \sa InquiryType
*/
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

/*!
    Returns a list of all discovered Bluetooth devices.
*/
QList<QBluetoothDeviceInfo> QBluetoothDeviceDiscoveryAgent::discoveredDevices() const
{
    Q_D(const QBluetoothDeviceDiscoveryAgent);
    return d->discoveredDevices;
}

/*!
    Starts Bluetooth device discovery, if it is not already started.

    The deviceDiscovered() signal is emitted as each device is discovered. The finished() signal
    is emitted once device discovery is complete.
*/
void QBluetoothDeviceDiscoveryAgent::start()
{
    Q_D(QBluetoothDeviceDiscoveryAgent);
    if (!isActive())
        d->start();
}

/*!
    Stops Bluetooth device discovery.  The cancel() signal is emitted once the
    device discovery is canceled.  start() maybe called before the cancel signal is
    received.  Once start() has been called the cancel signal from the prior
    discovery will be silently discarded.
*/
void QBluetoothDeviceDiscoveryAgent::stop()
{
    Q_D(QBluetoothDeviceDiscoveryAgent);
    if (isActive())
        d->stop();
}

bool QBluetoothDeviceDiscoveryAgent::isActive() const
{
    Q_D(const QBluetoothDeviceDiscoveryAgent);
    return d->isActive();
}


/*!
    Returns the last error which has occurred.
*/
QBluetoothDeviceDiscoveryAgent::Error QBluetoothDeviceDiscoveryAgent::error() const
{
    Q_D(const QBluetoothDeviceDiscoveryAgent);

    return d->lastError;
}

/*!
    Returns a human-readable description of the last error that occurred.
*/
QString QBluetoothDeviceDiscoveryAgent::errorString() const
{
    Q_D(const QBluetoothDeviceDiscoveryAgent);
    return d->errorString;
}

#include "moc_qbluetoothdevicediscoveryagent.cpp"

QTBLUETOOTH_END_NAMESPACE


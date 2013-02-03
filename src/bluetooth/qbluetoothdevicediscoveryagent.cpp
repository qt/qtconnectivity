/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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
#include "qbluetoothdevicediscoveryagent_p.h"

QT_BEGIN_NAMESPACE_BLUETOOTH

/*!
    \class QBluetoothDeviceDiscoveryAgent
    \inmodule QtBluetooth
    \brief The QBluetoothDeviceDiscoveryAgent class discovers the Bluetooth
    devices nearby.

    To discover the nearby Bluetooth devices:
    \list
    \li create an instance of QBluetoothDeviceDiscoveryAgent,
    \li connect to either the deviceDiscovered() or finished() signals,
    \li and call start().
    \endlist

    \snippet doc_src_qtbluetooth.cpp discovery

    To retrieve results asynchronously, connect to the deviceDiscovered() signal. To get a list of
    all discovered devices, call discoveredDevices() after the finished() signal.
*/

/*!
    \enum QBluetoothDeviceDiscoveryAgent::Error

    Indicates all possible error conditions found during Bluetooth device discovery.

    \value NoError          No error has occurred.
    \value PoweredOff       The Bluetooth adaptor is powered off, power it on before doing discovery.
    \value IOFailure        Writing or reading from the device resulted in an error.
    \value UnknownError     An unknown error has occurred.
*/

/*!
    \enum QBluetoothDeviceDiscoveryAgent::InquiryType

    This enum describes the inquiry type used while discovering Bluetooth devices.

    \value GeneralUnlimitedInquiry  A general unlimited inquiry. Discovers all visible Bluetooth
                                    devices in the local vicinity.
    \value LimitedInquiry           A limited inquiry discovers devices that are in limited
                                    inquiry mode.

    LimitedInquiry is not supported on all platforms. If it is requested on a platform that does not
    support it, GeneralUnlimitedInquiry will be used instead. Setting LimitedInquiry is useful
    for multi-player Bluetooth-based games that needs faster communication between the devices.
    The phone scans for devices in LimitedInquiry and Service Discovery is done on one or two devices
    to speed up the service scan. After the game has connected to the device it intends to,the device
    returns to GeneralUnilimitedInquiry.
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

    Returns true if the agent is currently discovering Bluetooth devices, otherwise returns false.
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
    \brief type of inquiry scan to be used while discovering devices

    This property affects the type of inquiry scan which is performed while discovering devices.

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
    discovery will be discarded.
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
    Returns the last error.
*/
QBluetoothDeviceDiscoveryAgent::Error QBluetoothDeviceDiscoveryAgent::error() const
{
    Q_D(const QBluetoothDeviceDiscoveryAgent);

    return d->lastError;
}

/*!
    Returns a human-readable description of the last error.
*/
QString QBluetoothDeviceDiscoveryAgent::errorString() const
{
    Q_D(const QBluetoothDeviceDiscoveryAgent);
    return d->errorString;
}

#include "moc_qbluetoothdevicediscoveryagent.cpp"

QT_END_NAMESPACE_BLUETOOTH


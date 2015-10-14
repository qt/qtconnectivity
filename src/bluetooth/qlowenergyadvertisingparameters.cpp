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

#include "qlowenergyadvertisingparameters.h"

QT_BEGIN_NAMESPACE

class QLowEnergyAdvertisingParametersPrivate : public QSharedData
{
public:
    QLowEnergyAdvertisingParametersPrivate()
        : filterPolicy(QLowEnergyAdvertisingParameters::IgnoreWhiteList)
        , mode(QLowEnergyAdvertisingParameters::AdvInd)
        , minInterval(1280)
        , maxInterval(1280)
    {
    }

    QList<QLowEnergyAdvertisingParameters::AddressInfo> whiteList;
    QLowEnergyAdvertisingParameters::FilterPolicy filterPolicy;
    QLowEnergyAdvertisingParameters::Mode mode;
    int minInterval;
    int maxInterval;
};

/*!
    \since 5.7
    \class QLowEnergyAdvertisingParameters
    \brief The QLowEnergyAdvertisingParameters class represents the parameters used for
           Bluetooth Low Energy advertising.
    \inmodule QtBluetooth
    \ingroup shared

    When running the advertising procedure, a number of parameters can be configured, such as
    how fast to advertise or which clients, if any, can connect to the advertising device.
    These parameters are set via this class, and their values will be used when advertising
    is started by calling \l QLowEnergyController::startAdvertising().

    \sa QLowEnergyAdvertisingData
    \sa QLowEnergyController::startAdvertising()
*/

/*!
    \enum QLowEnergyAdvertisingParameters::Mode

    Specifies in which way to advertise.
    \value AdvInd
        For non-directed, connectable advertising. Advertising is not directed to
        one specific device and a device seeing the advertisement can connect to the
        advertising device or send scan requests.
    \value AdvScanInd
        For non-directed, scannable advertising. Advertising is not directed to
        one specific device and a device seeing the advertisement can send a scan
        request to the advertising device, but cannot connect to it.
    \value AdvNonConnInd
        For non-directed, non-connectable advertising. Advertising is not directed to
        one specific device. A device seeing the advertisement cannot connect to the
        advertising device, nor can it send a scan request. This mode thus implies
        pure broadcasting.
*/

/*!
    \enum QLowEnergyAdvertisingParameters::FilterPolicy

    Specifies the semantics of the white list.
    \value IgnoreWhiteList
        The value of the white list is ignored, that is, no filtering takes place for
        either scan or connection requests when using undirected advertising.
    \value UseWhiteListForScanning
        The white list is used when handling scan requests, but is ignored for connection
        requests.
    \value UseWhiteListForConnecting
        The white list is used when handling connection requests, but is ignored for scan
        requests.
    \value UseWhiteListForScanningAndConnecting
        The white list is used for both connection and scan requests.

    \sa QLowEnergyAdvertisingParameters::whiteList()
*/

/*!
    \struct QLowEnergyAdvertisingParameters::AddressInfo

    Objects of this type form the elements of a white list.
    \sa QLowEnergyAdvertisingParameters::whiteList()
*/

/*!
    \variable QLowEnergyAdvertisingParameters::AddressInfo::address
        The Bluetooth address of a remote address.
*/

/*!
    \variable QLowEnergyAdvertisingParameters::AddressInfo::type
        The type of the address (public or private).
*/


/*!
   Constructs a new object of this class. All values are initialized to their defaults
   according to the Bluetooth Low Energy specification.
 */
QLowEnergyAdvertisingParameters::QLowEnergyAdvertisingParameters()
    : d(new QLowEnergyAdvertisingParametersPrivate)
{
}

/*! Constructs a new object of this class that is a copy of \a other. */
QLowEnergyAdvertisingParameters::QLowEnergyAdvertisingParameters(const QLowEnergyAdvertisingParameters &other)
    : d(other.d)
{
}

/*! Destroys this object. */
QLowEnergyAdvertisingParameters::~QLowEnergyAdvertisingParameters()
{
}

/*! Makes this object a copy of \a other and returns the new value of this object. */
QLowEnergyAdvertisingParameters &QLowEnergyAdvertisingParameters::operator=(const QLowEnergyAdvertisingParameters &other)
{
    d = other.d;
    return *this;
}

/*! Sets the advertising mode to \a mode. */
void QLowEnergyAdvertisingParameters::setMode(QLowEnergyAdvertisingParameters::Mode mode)
{
    d->mode = mode;
}

/*!
   Returns the advertising mode. The default is \l QLowEnergyAdvertisingParameters::AdvInd.
 */
QLowEnergyAdvertisingParameters::Mode QLowEnergyAdvertisingParameters::mode() const
{
    return d->mode;
}

/*!
   Sets the white list that is potentially used for filtering scan and connection requests.
   The \a whiteList parameter is the list of addresses to use for filtering, and \a policy
   specifies how exactly to use \a whiteList.
 */
void QLowEnergyAdvertisingParameters::setWhiteList(const QList<AddressInfo> &whiteList,
                                                   FilterPolicy policy)
{
    d->whiteList = whiteList;
    d->filterPolicy = policy;
}

/*!
   Returns the white list used for filtering scan and connection requests.
   By default, this list is empty.
 */
QList<QLowEnergyAdvertisingParameters::AddressInfo> QLowEnergyAdvertisingParameters::whiteList() const
{
    return d->whiteList;
}

/*!
   Returns the filter policy that determines how the white list is used. The default
   is \l QLowEnergyAdvertisingParameters::IgnoreWhiteList.
 */
QLowEnergyAdvertisingParameters::FilterPolicy QLowEnergyAdvertisingParameters::filterPolicy() const
{
    return d->filterPolicy;
}

/*!
   Sets the advertising interval. This is a range that gives the controller an upper and a lower
   bound for how often to send the advertising data. Both \a minimum and \a maximum are given
   in milliseconds.
   If \a maximum is smaller than \a minimum, it will be set to the value of \a minimum.
   \note There are limits for the minimum and maximum interval; the exact values depend on
         the mode. If they are exceeded, the lowest or highest possible value will be used,
         respectively.
 */
void QLowEnergyAdvertisingParameters::setInterval(quint16 minimum, quint16 maximum)
{
    d->minInterval = minimum;
    d->maxInterval = qMax(minimum, maximum);
}

/*!
   Returns the minimum advertising interval in milliseconds. The default is 1280.
 */
int QLowEnergyAdvertisingParameters::minimumInterval() const
{
    return d->minInterval;
}

/*!
   Returns the maximum advertising interval in milliseconds. The default is 1280.
 */
int QLowEnergyAdvertisingParameters::maximumInterval() const
{
    return d->maxInterval;
}

/*!
   \fn void QLowEnergyAdvertisingParameters::swap(QLowEnergyAdvertisingParameters &other)
   Swaps this object with \a other.
 */

/*!
   Returns \a true if \a p1 and \a p2 are equal with respect to their public state,
   otherwise returns false.
 */
bool operator==(const QLowEnergyAdvertisingParameters &p1,
                const QLowEnergyAdvertisingParameters &p2)
{
    if (p1.d == p2.d)
        return true;
    return p1.filterPolicy() == p2.filterPolicy()
            && p1.minimumInterval() == p2.minimumInterval()
            && p1.maximumInterval() == p2.maximumInterval()
            && p1.mode() == p2.mode()
            && p1.whiteList() == p2.whiteList();
}

/*!
   \fn bool operator!=(const QLowEnergyAdvertisingParameters &p1,
                       const QLowEnergyAdvertisingParameters &p2)
   Returns \a true if \a p1 and \a p2 are not equal with respect to their public state,
   otherwise returns false.
 */

QT_END_NAMESPACE

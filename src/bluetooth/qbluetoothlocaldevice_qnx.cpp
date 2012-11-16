/***************************************************************************
**
** Copyright (C) 2012 Research In Motion
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

#include "qbluetoothlocaldevice.h"
#include "qbluetoothaddress.h"
#include "qbluetoothlocaldevice_p.h"
#include "qnx/ppshelpers_p.h"

QTBLUETOOTH_BEGIN_NAMESPACE

QBluetoothLocalDevice::QBluetoothLocalDevice(QObject *parent)
:   QObject(parent)
{
    this->d_ptr = new QBluetoothLocalDevicePrivate();
}

QBluetoothLocalDevice::QBluetoothLocalDevice(const QBluetoothAddress &address, QObject *parent)
: QObject(parent)
{
    Q_UNUSED(address)
    this->d_ptr = new QBluetoothLocalDevicePrivate();
}

QString QBluetoothLocalDevice::name() const
{
    return this->d_ptr->name();
}

QBluetoothAddress QBluetoothLocalDevice::address() const
{
    return this->d_ptr->address();
}

void QBluetoothLocalDevice::powerOn()
{
    this->d_ptr->powerOn();
}

void QBluetoothLocalDevice::setHostMode(QBluetoothLocalDevice::HostMode mode)
{
    this->d_ptr->setHostMode(mode);
}

QBluetoothLocalDevice::HostMode QBluetoothLocalDevice::hostMode() const
{
    return this->d_ptr->hostMode();
}

QList<QBluetoothHostInfo> QBluetoothLocalDevice::allDevices()
{
    //We only have one device
    QList<QBluetoothHostInfo> localDevices;
    QBluetoothHostInfo hostInfo;
    hostInfo.setName(QBluetoothLocalDevicePrivate::name());
    hostInfo.setAddress(QBluetoothLocalDevicePrivate::address());
    localDevices.append(hostInfo);
    return localDevices;
}

void QBluetoothLocalDevice::requestPairing(const QBluetoothAddress &address, Pairing pairing)
{
    Q_UNUSED(pairing);
    ppsSendControlMessage("initiate_pairing", QStringLiteral("{\"addr\":\"%1\"}").arg(address.toString()), this);
}

QBluetoothLocalDevice::Pairing QBluetoothLocalDevice::pairingStatus(const QBluetoothAddress &address) const
{
    QVariant status = ppsRemoteDeviceStatus(address.toString().toLocal8Bit(), "paired");
    if (status.toBool())
        return Paired;
    return Unpaired;
}

void QBluetoothLocalDevice::pairingConfirmation(bool confirmation)
{
    Q_UNUSED(confirmation);
}

QBluetoothLocalDevicePrivate::QBluetoothLocalDevicePrivate()
{
    ppsRegisterControl();
}

QBluetoothLocalDevicePrivate::~QBluetoothLocalDevicePrivate()
{
    ppsUnregisterControl();
}

QBluetoothAddress QBluetoothLocalDevicePrivate::address()
{
    return QBluetoothAddress(ppsReadSetting("btaddr").toString());
}

QString QBluetoothLocalDevicePrivate::name()
{
    return ppsReadSetting("name").toString();
}

void QBluetoothLocalDevicePrivate::powerOn()
{
    ppsSendControlMessage("radio_init", this);
}

void QBluetoothLocalDevicePrivate::powerOff()
{
    ppsSendControlMessage("radio_shutdown", this);
}

void QBluetoothLocalDevicePrivate::setHostMode(QBluetoothLocalDevice::HostMode mode)
{
    //if (m_currentMode==mode){
    //    return;
    //}
    //If the device is in PowerOff state and the profile is changed then the power has to be turned on
    //if (m_currentMode == QBluetoothLocalDevice::HostPoweredOff) {
    //    powerOn();

    //}

    if (mode == QBluetoothLocalDevice::HostPoweredOff) {
        powerOff();
    }
    else if (mode == QBluetoothLocalDevice::HostDiscoverable) { //General discoverable and connectable.
        setAccess(1);
    }
    else if (mode == QBluetoothLocalDevice::HostConnectable) { //Connectable but not discoverable.
        setAccess(3);
    }
    else if (mode == QBluetoothLocalDevice::HostDiscoverableLimitedInquiry) { //Limited discoverable and connectable.
        setAccess(2);
    }
}
QBluetoothLocalDevice::HostMode QBluetoothLocalDevicePrivate::hostMode() const
{
    int hostMode = ppsReadSetting("accessibility").toInt();

    if (hostMode == 1) //General discoverable and connectable.
        return QBluetoothLocalDevice::HostDiscoverable;
    else if (hostMode == 3)  //Connectable but not discoverable.
        return QBluetoothLocalDevice::HostConnectable;
    else if (hostMode == 2)  //Limited discoverable and connectable.
        return QBluetoothLocalDevice::HostDiscoverableLimitedInquiry;
    else
        return QBluetoothLocalDevice::HostPoweredOff;
}

void QBluetoothLocalDevicePrivate::setAccess(int access)
{
    ppsSendControlMessage("set_access", QStringLiteral("\"access\":n:%1").arg(access), this);
}

void QBluetoothLocalDevicePrivate::controlReply(ppsResult result)
{
    if (!result.errorMsg.isEmpty()) {
        qWarning() << Q_FUNC_INFO << result.errorMsg;
        q_ptr->error(QBluetoothLocalDevice::UnknownError);
    }
}

QTBLUETOOTH_END_NAMESPACE

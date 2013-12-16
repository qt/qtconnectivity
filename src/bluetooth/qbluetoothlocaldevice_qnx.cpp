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

QT_BEGIN_NAMESPACE

QBluetoothLocalDevice::QBluetoothLocalDevice(QObject *parent)
:   QObject(parent)
{
    this->d_ptr = new QBluetoothLocalDevicePrivate(this);
    this->d_ptr->isValidDevice = true; //assume single local device on QNX
}

QBluetoothLocalDevice::QBluetoothLocalDevice(const QBluetoothAddress &address, QObject *parent)
: QObject(parent)
{
    this->d_ptr = new QBluetoothLocalDevicePrivate(this);

    //works since we assume a single local device on QNX
    this->d_ptr->isValidDevice = (QBluetoothLocalDevicePrivate::address() == address ||
                                      address == QBluetoothAddress());
}

QString QBluetoothLocalDevice::name() const
{
    if (this->d_ptr->isValid())
        return this->d_ptr->name();
    return QString();
}

QBluetoothAddress QBluetoothLocalDevice::address() const
{
    if (this->d_ptr->isValid())
        return this->d_ptr->address();
    return QBluetoothAddress();
}

void QBluetoothLocalDevice::powerOn()
{
    if (hostMode() == QBluetoothLocalDevice::HostPoweredOff)
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
    if (address.isNull()) {
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothLocalDevice::Error, QBluetoothLocalDevice::PairingError));
        return;
    }

    const Pairing current_pairing = pairingStatus(address);
    if (current_pairing == pairing) {
        QMetaObject::invokeMethod(this, "pairingFinished", Qt::QueuedConnection, Q_ARG(QBluetoothAddress, address),
                                  Q_ARG(QBluetoothLocalDevice::Pairing, pairing));
        return;
    }
    d_ptr->requestPairing(address, pairing);
}

QBluetoothLocalDevice::Pairing QBluetoothLocalDevice::pairingStatus(const QBluetoothAddress &address) const
{
    if (!isValid())
        return Unpaired;

    QVariant status = ppsRemoteDeviceStatus(address.toString().toLocal8Bit(), "paired");
    if (status.toBool())
        return Paired;
    return Unpaired;
}

void QBluetoothLocalDevice::pairingConfirmation(bool confirmation)
{
    Q_UNUSED(confirmation);
}

QBluetoothLocalDevicePrivate::QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *q)
    : q_ptr(q)
{
    ppsRegisterControl();
    ppsRegisterForEvent(QStringLiteral("access_changed"), this);
    ppsRegisterForEvent(QStringLiteral("pairing_complete"), this);
    ppsRegisterForEvent(QStringLiteral("device_deleted"), this);
    ppsRegisterForEvent(QStringLiteral("radio_shutdown"), this);
}

QBluetoothLocalDevicePrivate::~QBluetoothLocalDevicePrivate()
{
    ppsUnregisterControl(this);
    ppsUnregisterForEvent(QStringLiteral("access_changed"), this);
    ppsUnregisterForEvent(QStringLiteral("pairing_complete"), this);
    ppsUnregisterForEvent(QStringLiteral("device_deleted"), this);
    ppsUnregisterForEvent(QStringLiteral("radio_shutdown"), this);
}

bool QBluetoothLocalDevicePrivate::isValid() const
{
    return isValidDevice;
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
    if (isValid())
        ppsSendControlMessage("radio_init", this);
}

void QBluetoothLocalDevicePrivate::powerOff()
{
    if (isValid())
        ppsSendControlMessage("radio_shutdown", this);
}

void QBluetoothLocalDevicePrivate::setHostMode(QBluetoothLocalDevice::HostMode mode)
{
    if (!isValid())
        return;

    QBluetoothLocalDevice::HostMode currentHostMode = hostMode();
    if (currentHostMode == mode){
        return;
    }
    //If the device is in PowerOff state and the profile is changed then the power has to be turned on
    if (currentHostMode == QBluetoothLocalDevice::HostPoweredOff) {
        qCDebug(QT_BT_QNX) << "Powering on";
        powerOn();
    }

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

void QBluetoothLocalDevicePrivate::requestPairing(const QBluetoothAddress &address,
                                                  QBluetoothLocalDevice::Pairing pairing)
{
    if (pairing == QBluetoothLocalDevice::Paired || pairing == QBluetoothLocalDevice::AuthorizedPaired) {
        ppsSendControlMessage("initiate_pairing", QStringLiteral("{\"addr\":\"%1\"}").arg(address.toString()),
                          this);
    } else {
        ppsSendControlMessage("remove_device", QStringLiteral("{\"addr\":\"%1\"}").arg(address.toString()),
                          this);
    }
}

QBluetoothLocalDevice::HostMode QBluetoothLocalDevicePrivate::hostMode() const
{
    if (!isValid())
        return QBluetoothLocalDevice::HostPoweredOff;

    if (!ppsReadSetting("enabled").toBool())
        return QBluetoothLocalDevice::HostPoweredOff;

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

extern int __newHostMode;

void QBluetoothLocalDevicePrivate::setAccess(int access)
{
    if (!ppsReadSetting("enabled").toBool()) { //We cannot set the host mode until BT is fully powered up
        __newHostMode = access;
    } else {
        ppsSendControlMessage("set_access", QStringLiteral("{\"access\":%1}").arg(access), 0);

    }
}

void QBluetoothLocalDevicePrivate::controlReply(ppsResult result)
{
    qCDebug(QT_BT_QNX) << Q_FUNC_INFO << result.msg << result.dat;
    if (!result.errorMsg.isEmpty()) {
        qCWarning(QT_BT_QNX) << Q_FUNC_INFO << result.errorMsg;
        if (result.msg == QStringLiteral("initiate_pairing"))
            q_ptr->error(QBluetoothLocalDevice::PairingError);
        else
            q_ptr->error(QBluetoothLocalDevice::UnknownError);
    }
}

void QBluetoothLocalDevicePrivate::controlEvent(ppsResult result)
{
    qCDebug(QT_BT_QNX) << Q_FUNC_INFO << "Control Event" << result.msg;
    if (result.msg == QStringLiteral("access_changed")) {
        if (__newHostMode == -1 && result.dat.size() > 1 &&
                result.dat.first() == QStringLiteral("level")) {
            QBluetoothLocalDevice::HostMode newHostMode = hostMode();
            qCDebug(QT_BT_QNX) << "New Host mode" << newHostMode;
            Q_EMIT q_ptr->hostModeStateChanged(newHostMode);
        }
    } else if (result.msg == QStringLiteral("pairing_complete")) {
        qCDebug(QT_BT_QNX) << "pairing completed";
        if (result.dat.contains(QStringLiteral("addr"))) {
            const QBluetoothAddress address = QBluetoothAddress(
                        result.dat.at(result.dat.indexOf(QStringLiteral("addr")) + 1));

            QBluetoothLocalDevice::Pairing pairingStatus = QBluetoothLocalDevice::Paired;

            if (result.dat.contains(QStringLiteral("trusted")) &&
                    result.dat.at(result.dat.indexOf(QStringLiteral("trusted")) + 1) == QStringLiteral("true")) {
                pairingStatus = QBluetoothLocalDevice::AuthorizedPaired;
            }
            qCDebug(QT_BT_QNX) << "pairing completed" << address.toString();
            Q_EMIT q_ptr->pairingFinished(address, pairingStatus);
        }
    } else if (result.msg == QStringLiteral("device_deleted")) {
        qCDebug(QT_BT_QNX) << "device deleted";
        if (result.dat.contains(QStringLiteral("addr"))) {
            const QBluetoothAddress address = QBluetoothAddress(
                        result.dat.at(result.dat.indexOf(QStringLiteral("addr")) + 1));
            Q_EMIT q_ptr->pairingFinished(address, QBluetoothLocalDevice::Unpaired);
        }
    } else if (result.msg == QStringLiteral("radio_shutdown")) {
        qCDebug(QT_BT_QNX) << "radio shutdown";
        Q_EMIT q_ptr->hostModeStateChanged(QBluetoothLocalDevice::HostPoweredOff);
    }
}

QT_END_NAMESPACE

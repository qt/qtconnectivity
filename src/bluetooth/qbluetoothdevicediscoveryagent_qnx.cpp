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

#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothdevicediscoveryagent_p.h"
#include "qbluetoothaddress.h"
#include "qbluetoothuuid.h"

#include <QtCore/private/qcore_unix_p.h>


QT_BEGIN_NAMESPACE_BLUETOOTH

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate():
    QObject(0), lastError(QBluetoothDeviceDiscoveryAgent::NoError), pendingCancel(false), pendingStart(false), m_rdfd(-1)
{
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
    if (pendingStart)
        stop();

    ppsUnreguisterForEvent(QStringLiteral("device_added"), this);
    ppsUnreguisterForEvent(QStringLiteral("device_search"), this);
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    return pendingStart;
}

void QBluetoothDeviceDiscoveryAgentPrivate::start()
{
    qBBBluetoothDebug() << "Starting device discovery";
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    if (pendingStart)
        return;
    pendingStart = true;
    discoveredDevices.clear();

    if (m_rdfd != -1 || (m_rdfd = qt_safe_open("/pps/services/bluetooth/remote_devices/.all", O_RDONLY)) == -1) {
        qWarning() << Q_FUNC_INFO << "rdfd - failed to open /pps/services/bluetooth/remote_devices/.all";
        lastError = QBluetoothDeviceDiscoveryAgent::IOFailure;
        emit q->error(lastError);
        stop();
        return;
    } else {
        m_rdNotifier = new QSocketNotifier(m_rdfd, QSocketNotifier::Read, this);
        if (m_rdNotifier) {
            connect(m_rdNotifier, SIGNAL(activated(int)), this, SLOT(remoteDevicesChanged(int)));
        } else {
            qWarning() << Q_FUNC_INFO << "failed to connect to m_rdNotifier";
            lastError = QBluetoothDeviceDiscoveryAgent::IOFailure;
            emit q->error(lastError);
            stop();
            return;
        }
    }

    //If there is no new results after 7 seconds, the device inquire will be stopped
    m_finishedTimer.start(7000);
    connect(&m_finishedTimer, SIGNAL(timeout()), this, SLOT(finished()));

    if (!m_controlRegistered) {
        ppsRegisterControl();
        m_controlRegistered = true;
    }

    ppsSendControlMessage("device_search", this);
    ppsRegisterForEvent(QStringLiteral("device_added"), this);

    ppsRegisterForEvent(QStringLiteral("device_search"), this);
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    qBBBluetoothDebug() << "Stopping device search";
    ppsSendControlMessage("cancel_device_search",this);
    emit q->canceled();
    abort();
}

void QBluetoothDeviceDiscoveryAgentPrivate::remoteDevicesChanged(int fd)
{
    pps_decoder_t ppsDecoder;
    pps_decoder_initialize(&ppsDecoder, NULL);

    QBluetoothAddress deviceAddr;
    QString deviceName;

    if (!ppsReadRemoteDevice(fd, &ppsDecoder, &deviceAddr, &deviceName)) {
        return;
    }

    bool paired = false;
    int cod = 0;
    int dev_type = 0;
    int rssi = 0;

    pps_decoder_get_bool(&ppsDecoder, "paired", &paired);
    pps_decoder_get_int(&ppsDecoder, "cod", &cod);
    pps_decoder_get_int(&ppsDecoder, "dev_type", &dev_type);
    pps_decoder_get_int(&ppsDecoder, "rssi", &rssi);
    pps_decoder_cleanup(&ppsDecoder);

    QBluetoothDeviceInfo deviceInfo(deviceAddr, deviceName, cod);
    deviceInfo.setRssi(rssi);

    //Prevent a device from beeing listed twice
    for (int i=0; i < discoveredDevices.size(); i++) {
        if (discoveredDevices.at(i).address() == deviceInfo.address()) {
            if (discoveredDevices.at(i) == deviceInfo) {
                return;
            } else {
                discoveredDevices.removeAt(i);
                break;
            }
        }
    }
    //Starts the timer again
    m_finishedTimer.start(7000);

    if (!deviceAddr.isNull()) {
        discoveredDevices.append(deviceInfo);
        qBBBluetoothDebug() << "Device discovered: " << deviceName << deviceAddr.toString();
        emit q_ptr->deviceDiscovered(discoveredDevices.last());
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::controlReply(ppsResult result)
{
    if (result.msg == QStringLiteral("device_search")) {
        if (result.dat.size() > 0 && result.dat.first() == QStringLiteral("EOK")) {
            //Do nothing. We can not be certain, that the device search is over yet
        }
        else {
            qWarning() << "A PPS Bluetooth error occured:" << result.errorMsg;
            q_ptr->error(QBluetoothDeviceDiscoveryAgent::UnknownError);
            lastError = QBluetoothDeviceDiscoveryAgent::UnknownError;
            errorString = result.errorMsg;
            stop();
        }
    } else if (result.msg == QStringLiteral("cancel_device_search")) {
        qBBBluetoothDebug() << "Cancel device search";
        if (result.error == 16) {
            qWarning() << Q_FUNC_INFO << "Resource is busy. Is Bluetooth enabled?";
            lastError = QBluetoothDeviceDiscoveryAgent::PoweredOff;
            errorString = result.errorMsg;
        }
        //stop();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::controlEvent(ppsResult result)
{
    if (result.msg == QStringLiteral("device_added")) {
        qBBBluetoothDebug() << "Device was added" << result.dat.first();
    }
}


void QBluetoothDeviceDiscoveryAgentPrivate::finished()
{
    qBBBluetoothDebug() << "Device discovery finished";
    m_finishedTimer.stop();
    stop();
    q_ptr->finished();
}

void QBluetoothDeviceDiscoveryAgentPrivate::abort()
{
    if (m_controlRegistered) {
        ppsUnregisterControl(this);
        m_controlRegistered = false;
    }

    pendingStart = false;
    if (m_rdNotifier) {
        delete m_rdNotifier;
        m_rdNotifier = 0;
    }
    if (m_rdfd != -1) {
        qt_safe_close(m_rdfd);
        m_rdfd = -1;
    }
}

QT_END_NAMESPACE_BLUETOOTH


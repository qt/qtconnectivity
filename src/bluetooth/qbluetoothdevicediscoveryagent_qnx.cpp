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


QT_BEGIN_NAMESPACE

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(const QBluetoothAddress &deviceAdapter):
    QObject(0), lastError(QBluetoothDeviceDiscoveryAgent::NoError),
    m_rdfd(-1), m_active(false), m_nextOp(None), m_currentOp(None)
{
    Q_UNUSED(deviceAdapter);
    inquiryType = QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry;
    ppsRegisterControl();
    ppsRegisterForEvent(QStringLiteral("device_added"), this);
    ppsRegisterForEvent(QStringLiteral("device_search"), this);
    connect(&m_finishedTimer, SIGNAL(timeout()), this, SLOT(finished()));
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
    if (m_active)
        stop();
    ppsUnregisterForEvent(QStringLiteral("device_added"), this);
    ppsUnregisterForEvent(QStringLiteral("device_search"), this);
    ppsUnregisterControl(this);
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    return m_active;
}

void QBluetoothDeviceDiscoveryAgentPrivate::start()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    m_active = true;
    isFinished = false;
    discoveredDevices.clear();

    if (m_currentOp == Cancel) {
        m_nextOp = Start;
        return;
    }
    if (m_nextOp == Cancel) {
        m_nextOp = None;
    }
    m_currentOp = Start;

    if (m_rdfd != -1) {
        qBBBluetoothDebug() << "RDev FD still open";
    } else if ((m_rdfd = qt_safe_open("/pps/services/bluetooth/remote_devices/.all", O_RDONLY)) == -1) {
        qWarning() << Q_FUNC_INFO << "rdfd - failed to open /pps/services/bluetooth/remote_devices/.all"
                      << m_rdfd;
        lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
        emit q->error(lastError);
        stop();
        return;
    } else {
        m_rdNotifier = new QSocketNotifier(m_rdfd, QSocketNotifier::Read, this);
        if (!m_rdNotifier) {
            qWarning() << Q_FUNC_INFO << "failed to connect to m_rdNotifier";
            lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
            emit q->error(lastError);
            stop();
            return;
        }
    }

    if (ppsSendControlMessage("device_search", this)) {
        //If there is no new results after 7 seconds, the device inquire will be stopped
        m_finishedTimer.start(10000);
        connect(m_rdNotifier, SIGNAL(activated(int)), this, SLOT(remoteDevicesChanged(int)));
    } else {
        qWarning() << "Could not write to control FD";
        m_active = false;
        q->error(QBluetoothDeviceDiscoveryAgent::InputOutputError);
        return;
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    m_active = false;
    m_finishedTimer.stop();
    if (m_currentOp == Start) {
        m_nextOp = Cancel;
        return;
    }
    m_currentOp = Cancel;

    qBBBluetoothDebug() << "Stopping device search";
    ppsSendControlMessage("cancel_device_search",this);

    if (m_rdNotifier) {
        delete m_rdNotifier;
        m_rdNotifier = 0;
    }
    if (m_rdfd != -1) {
        qt_safe_close(m_rdfd);
        m_rdfd = -1;
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::remoteDevicesChanged(int fd)
{
    if (!m_active)
        return;
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

    bool updated = false;
    //Prevent a device from beeing listed twice
    for (int i=0; i < discoveredDevices.size(); i++) {
        if (discoveredDevices.at(i).address() == deviceInfo.address()) {
            updated = true;
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
        qBBBluetoothDebug() << "Device discovered: " << deviceName << deviceAddr.toString();
        discoveredDevices.append(deviceInfo);
        if (!updated)//We are not allowed to emit a signal with the updated version
            emit q_ptr->deviceDiscovered(discoveredDevices.last());
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::controlReply(ppsResult result)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    if (result.msg == QStringLiteral("device_search") && m_currentOp == Start) {
        if (result.dat.size() > 0 && result.dat.first() == QStringLiteral("EOK")) {
            //Do nothing. We can not be certain, that the device search is over yet
        } else if (result.error == 16) {
            qBBBluetoothDebug() << "Could not start device inquire bc resource is busy";
            if (m_nextOp == None) { //We try again
                ppsSendControlMessage("cancel_device_search",this);
                QTimer::singleShot(5000, this, SLOT(startDeviceSearch()));
                m_finishedTimer.start(20000);
            }
            return;
        } else {
            qWarning("A PPS Bluetooth error occurred:");
            lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
            errorString = result.errorMsg;
            Q_EMIT q_ptr->error(QBluetoothDeviceDiscoveryAgent::InputOutputError);
            stop();
        }
        processNextOp();
    } else if (result.msg == QStringLiteral("cancel_device_search") && m_currentOp == Cancel && !isFinished) {
        qBBBluetoothDebug() << "Cancel device search";
//        if (!result.errorMsg.isEmpty()) {
//            lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
//            errorString = result.errorMsg;
//            q_ptr->error(QBluetoothDeviceDiscoveryAgent::InputOutputError);
//        }
        emit q->canceled();
        processNextOp();
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
    if (m_active) {
        qBBBluetoothDebug() << "Device discovery finished";
        isFinished = true;
        stop();
        q_ptr->finished();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::startDeviceSearch()
{
    if (m_currentOp == Start) {
        ppsSendControlMessage("device_search", this); //Try again
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::processNextOp()
{
    if (m_currentOp == m_nextOp) {
        m_currentOp = None;
        m_nextOp = None;
    }
    m_currentOp = m_nextOp;
    m_nextOp = None;

    if (m_currentOp == Start) {
        start();
    } else if (m_currentOp == Cancel) {
        stop();
    }
}

QT_END_NAMESPACE


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

#include "qbluetoothservicediscoveryagent.h"
#include "qbluetoothservicediscoveryagent_p.h"

#include "qbluetoothdeviceinfo.h"
#include "qbluetoothdevicediscoveryagent.h"

#include <QStringList>
#include "qbluetoothuuid.h"

#include <sys/pps.h>

#include <QtCore/private/qcore_unix_p.h>

QTBLUETOOTH_BEGIN_NAMESPACE

QBluetoothServiceDiscoveryAgentPrivate::QBluetoothServiceDiscoveryAgentPrivate(const QBluetoothAddress &address)
    : rdNotifier(0), error(QBluetoothServiceDiscoveryAgent::NoError), state(Inactive), deviceAddress(address),
      deviceDiscoveryAgent(0), mode(QBluetoothServiceDiscoveryAgent::MinimalDiscovery)
{
}

QBluetoothServiceDiscoveryAgentPrivate::~QBluetoothServiceDiscoveryAgentPrivate()
{
}

void QBluetoothServiceDiscoveryAgentPrivate::start(const QBluetoothAddress &address)
{
    Q_Q(QBluetoothServiceDiscoveryAgent);
    qBBBluetoothDebug() << "Starting Service discovery for" << address.toString();
    if ((m_rdfd = qt_safe_open((QByteArray("/pps/services/bluetooth/remote_devices/%1").append(address.toString().toUtf8().constData())).constData(), O_RDONLY)) == -1) {
        qWarning() << "Failed to open " << "/pps/services/bluetooth/remote_devices/" << address.toString();
    } else {
        if (rdNotifier)
            delete rdNotifier;
        rdNotifier = new QSocketNotifier(m_rdfd, QSocketNotifier::Read, this);
        if (rdNotifier) {
            connect(rdNotifier, SIGNAL(activated(int)), this, SLOT(remoteDevicesChanged(int)));
        } else {
            qWarning() << "Service Discovery: Failed to connect to rdNotifier";
            error = QBluetoothServiceDiscoveryAgent::DeviceDiscoveryError;
            errorString = QStringLiteral("Failed to connect to rdNotifier");
            q->error(error);
            return;
        }
    }

    ppsRegisterControl();
    ppsSendControlMessage("service_query", QStringLiteral("{\"addr\":\"%1\"}").arg(address.toString()));
    ppsRegisterForEvent(QStringLiteral("service_query"), this);
}

void QBluetoothServiceDiscoveryAgentPrivate::stop()
{
    if (rdNotifier)
        delete rdNotifier;
    rdNotifier = 0;
    ppsUnregisterControl();
}

void QBluetoothServiceDiscoveryAgentPrivate::remoteDevicesChanged(int fd)
{
    pps_decoder_t ppsDecoder;
    pps_decoder_initialize(&ppsDecoder, 0);

    QBluetoothAddress deviceAddr;
    QString deviceName;

    if (!ppsReadRemoteDevice(fd, &ppsDecoder, &deviceAddr, &deviceName)) {
        pps_decoder_cleanup(&ppsDecoder);
        return;
    }

    pps_decoder_push(&ppsDecoder, "available_services");

    const char *next_service = 0;
    for (int service_count=0; pps_decoder_get_string(&ppsDecoder, 0, &next_service ) == PPS_DECODER_OK; service_count++) {
        if (next_service == 0)
            break;

        qBBBluetoothDebug() << Q_FUNC_INFO << "Service" << next_service;

        QBluetoothServiceInfo serviceInfo;
        serviceInfo.setDevice(discoveredDevices.at(0));

        QBluetoothServiceInfo::Sequence  protocolDescriptorList;
        protocolDescriptorList << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap));

        bool ok;
        QBluetoothUuid suuid(QByteArray(next_service).toUInt(&ok,16));
        if (!ok) {
            QList<QByteArray> serviceName = QByteArray(next_service).split(':');
            if (serviceName.size() == 2) {
                serviceInfo.setServiceUuid(QBluetoothUuid(serviceName.last().toUInt()));
                suuid = QBluetoothUuid((quint16)(serviceName.first().toUInt(&ok,16)));
                if (suuid == QBluetoothUuid::SerialPort)
                    protocolDescriptorList << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm));
            }
        } else {
            //We do not have anything better, so we set the service class UUID as service UUID
            serviceInfo.setServiceUuid(suuid);
        }

        serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);

        QList<QBluetoothUuid> serviceClassId;
        serviceClassId << suuid;
        serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, QVariant::fromValue(serviceClassId));

        serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList,
                                     QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));
        serviceInfo.setDevice(discoveredDevices.at(0));

        bool entryExists = false;
        //Did we already discover this service?
        foreach (QBluetoothServiceInfo sInfo, q_ptr->discoveredServices()) {
            if (sInfo.device() == serviceInfo.device()
                    && sInfo.serviceUuid() == serviceInfo.serviceUuid()
                    && sInfo.serviceClassUuids() == serviceInfo.serviceClassUuids()) {
                entryExists = true;
                //qBBBluetoothDebug() << "Entry exists" << serviceInfo.serviceClassUuids().first() << sInfo.serviceClassUuids().first();
                break;
            }
        }

        if (!entryExists) {
            qBBBluetoothDebug() << "Adding service" << next_service << " " << serviceInfo.socketProtocol();
            discoveredServices << serviceInfo;
            q_ptr->serviceDiscovered(serviceInfo);
        }
    }

    pps_decoder_cleanup(&ppsDecoder);
}

void QBluetoothServiceDiscoveryAgentPrivate::controlReply(ppsResult result)
{
    Q_Q(QBluetoothServiceDiscoveryAgent);
    if (!result.errorMsg.isEmpty()) {
        qWarning() << Q_FUNC_INFO << result.errorMsg;
        errorString = result.errorMsg;
        error = QBluetoothServiceDiscoveryAgent::DeviceDiscoveryError;
        q->error(error);
    } else {
        _q_serviceDiscoveryFinished();
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::controlEvent(ppsResult result)
{
    Q_Q(QBluetoothServiceDiscoveryAgent);
    if (!result.errorMsg.isEmpty()) {
        qWarning() << Q_FUNC_INFO << result.errorMsg;
        errorString = result.errorMsg;
        error = QBluetoothServiceDiscoveryAgent::DeviceDiscoveryError;
        q->error(error);
    } else {
        _q_serviceDiscoveryFinished();
    }
}

QTBLUETOOTH_END_NAMESPACE

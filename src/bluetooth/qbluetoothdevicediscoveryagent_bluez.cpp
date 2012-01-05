/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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
#include "qbluetoothaddress.h"
#include "qbluetoothuuid.h"

#include "bluez/manager_p.h"
#include "bluez/adapter_p.h"
#include "bluez/device_p.h"

//#define QT_DEVICEDISCOVERY_DEBUG

QTBLUETOOTH_BEGIN_NAMESPACE

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate()
    :   lastError(QBluetoothDeviceDiscoveryAgent::NoError), pendingCancel(false), pendingStart(false), adapter(0)
{
    manager = new OrgBluezManagerInterface(QLatin1String("org.bluez"), QLatin1String("/"),
                                           QDBusConnection::systemBus());
    inquiryType = QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry;
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
    delete manager;
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    if(pendingStart)
        return true;
    if(pendingCancel)
        return false;
    return adapter != 0;
}

void QBluetoothDeviceDiscoveryAgentPrivate::start()
{

    if(pendingCancel == true) {
        pendingStart = true;
        return;
    }

    discoveredDevices.clear();

    QDBusPendingReply<QDBusObjectPath> reply = manager->DefaultAdapter();
    reply.waitForFinished();
    if (reply.isError()) {
        errorString = reply.error().message();
#ifdef QT_DEVICEDISCOVERY_DEBUG
        qDebug() << Q_FUNC_INFO << "ERROR: " << errorString;
#endif
        lastError = QBluetoothDeviceDiscoveryAgent::IOFailure;
        Q_Q(QBluetoothDeviceDiscoveryAgent);
        emit q->error(lastError);
        return;
    }

    adapter = new OrgBluezAdapterInterface(QLatin1String("org.bluez"), reply.value().path(),
                                           QDBusConnection::systemBus());

    Q_Q(QBluetoothDeviceDiscoveryAgent);
    QObject::connect(adapter, SIGNAL(DeviceFound(QString,QVariantMap)),
                     q, SLOT(_q_deviceFound(QString,QVariantMap)));
    QObject::connect(adapter, SIGNAL(PropertyChanged(QString,QDBusVariant)),
                     q, SLOT(_q_propertyChanged(QString,QDBusVariant)));

    QDBusPendingReply<QVariantMap> propertiesReply = adapter->GetProperties();
    propertiesReply.waitForFinished();
    if(propertiesReply.isError()) {
#ifdef QT_DEVICEDISCOVERY_DEBUG
        qDebug() << Q_FUNC_INFO << "ERROR: " << errorString;
#endif
        errorString = propertiesReply.error().message();
        lastError = QBluetoothDeviceDiscoveryAgent::IOFailure;
        Q_Q(QBluetoothDeviceDiscoveryAgent);
        emit q->error(lastError);
        return;
    }

    QDBusPendingReply<> discoveryReply = adapter->StartDiscovery();
    if (discoveryReply.isError()) {
        delete adapter;
        adapter = 0;
        errorString = discoveryReply.error().message();
        lastError = QBluetoothDeviceDiscoveryAgent::IOFailure;
        Q_Q(QBluetoothDeviceDiscoveryAgent);
        emit q->error(lastError);
#ifdef QT_DEVICEDISCOVERY_DEBUG
        qDebug() << Q_FUNC_INFO << "ERROR: " << errorString;
#endif
        return;
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    if (adapter) {
#ifdef QT_DEVICEDISCOVERY_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif        
        pendingCancel = true;
        pendingStart = false;
        QDBusPendingReply<> reply = adapter->StopDiscovery();
        reply.waitForFinished();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::_q_deviceFound(const QString &address,
                                                           const QVariantMap &dict)
{
    const QBluetoothAddress btAddress(address);
    const QString btName = dict.value(QLatin1String("Name")).toString();
    quint32 btClass = dict.value(QLatin1String("Class")).toUInt();    

#ifdef QT_DEVICEDISCOVERY_DEBUG
    qDebug() << "Discovered: " << address << btName
             << "Num UUIDs" << dict.value(QLatin1String("UUIDs")).toStringList().count()
             << "total device" << discoveredDevices.count() << "cached"
             << dict.value(QLatin1String("Cached")).toBool()
             << "RSSI" << dict.value(QLatin1String("RSSI")).toInt();
#endif

    QBluetoothDeviceInfo device(btAddress, btName, btClass);
    if(dict.value(QLatin1String("RSSI")).isValid())
        device.setRssi(dict.value(QLatin1String("RSSI")).toInt());
    QList<QBluetoothUuid> uuids;
    foreach (const QString &u, dict.value(QLatin1String("UUIDs")).toStringList()) {
        uuids.append(QBluetoothUuid(u));
    }
    device.setServiceUuids(uuids, QBluetoothDeviceInfo::DataIncomplete);
    device.setCached(dict.value(QLatin1String("Cached")).toBool());
    for(int i = 0; i < discoveredDevices.size(); i++){
        if(discoveredDevices[i].address() == device.address()) {
            if(discoveredDevices[i] == device) {
#ifdef QT_DEVICEDISCOVERY_DEBUG
                  qDebug() << "Duplicate: " << address;
#endif
                return;
            }
            discoveredDevices.replace(i, device);
            Q_Q(QBluetoothDeviceDiscoveryAgent);
#ifdef QT_DEVICEDISCOVERY_DEBUG
            qDebug() << "Updated: " << address;
#endif

            emit q->deviceDiscovered(device);
            return; // this works if the list doesn't contain duplicates. Don't let it.
        }
    }
#ifdef QT_DEVICEDISCOVERY_DEBUG
    qDebug() << "Emit: " << address;
#endif
    discoveredDevices.append(device);
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    emit q->deviceDiscovered(device);
}

void QBluetoothDeviceDiscoveryAgentPrivate::_q_propertyChanged(const QString &name,
                                                               const QDBusVariant &value)
{    
#ifdef QT_DEVICEDISCOVERY_DEBUG
    qDebug() << Q_FUNC_INFO << name << value.variant();
#endif

    if (name == QLatin1String("Discovering") && !value.variant().toBool()) {
        Q_Q(QBluetoothDeviceDiscoveryAgent);
        adapter->deleteLater();
        adapter = 0;
        if(pendingCancel && !pendingStart){
            emit q->canceled();
            pendingCancel = false;
        }
        else if(pendingStart){
            pendingStart = false;
            pendingCancel = false;
            start();
        }
        else {
            emit q->finished();
        }
    }
}

QTBLUETOOTH_END_NAMESPACE

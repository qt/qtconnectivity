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

#include <QtCore/QLoggingCategory>
#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothdevicediscoveryagent_p.h"
#include "qbluetoothaddress.h"
#include "qbluetoothuuid.h"

#include "bluez/manager_p.h"
#include "bluez/adapter_p.h"
#include "bluez/device_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(const QBluetoothAddress &deviceAdapter)
    :   lastError(QBluetoothDeviceDiscoveryAgent::NoError), m_adapterAddress(deviceAdapter), pendingCancel(false), pendingStart(false),
        adapter(0)
{
    manager = new OrgBluezManagerInterface(QLatin1String("org.bluez"), QLatin1String("/"),
                                           QDBusConnection::systemBus());
    inquiryType = QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry;
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
    delete manager;
    delete adapter;
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
    QDBusPendingReply<QDBusObjectPath> reply;

    if (m_adapterAddress.isNull())
        reply = manager->DefaultAdapter();
    else
        reply = manager->FindAdapter(m_adapterAddress.toString());
    reply.waitForFinished();

    if (reply.isError()) {
        errorString = reply.error().message();
        qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << "ERROR: " << errorString;
        lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
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
        errorString = propertiesReply.error().message();
        delete adapter;
        adapter = 0;
        qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << "ERROR: " << errorString;
        lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
        Q_Q(QBluetoothDeviceDiscoveryAgent);
        delete adapter;
        adapter = 0;
        emit q->error(lastError);
        return;
    }

    if (!propertiesReply.value().value(QStringLiteral("Powered")).toBool()) {
        qCDebug(QT_BT_BLUEZ) << "Aborting device discovery due to offline Bluetooth Adapter";
        lastError = QBluetoothDeviceDiscoveryAgent::PoweredOffError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Device is powered off");
        delete adapter;
        adapter = 0;
        emit q->error(lastError);
        return;
    }

    QDBusPendingReply<> discoveryReply = adapter->StartDiscovery();
    if (discoveryReply.isError()) {
        delete adapter;
        adapter = 0;
        errorString = discoveryReply.error().message();
        lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
        Q_Q(QBluetoothDeviceDiscoveryAgent);
        emit q->error(lastError);
        qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << "ERROR: " << errorString;
        return;
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    if (adapter) {
        qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO;
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

    qCDebug(QT_BT_BLUEZ) << "Discovered: " << address << btName
             << "Num UUIDs" << dict.value(QLatin1String("UUIDs")).toStringList().count()
             << "total device" << discoveredDevices.count() << "cached"
             << dict.value(QLatin1String("Cached")).toBool()
             << "RSSI" << dict.value(QLatin1String("RSSI")).toInt();

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
                qCDebug(QT_BT_BLUEZ) << "Duplicate: " << address;
                return;
            }
            discoveredDevices.replace(i, device);
            Q_Q(QBluetoothDeviceDiscoveryAgent);
            qCDebug(QT_BT_BLUEZ) << "Updated: " << address;

            emit q->deviceDiscovered(device);
            return; // this works if the list doesn't contain duplicates. Don't let it.
        }
    }
    qCDebug(QT_BT_BLUEZ) << "Emit: " << address;
    discoveredDevices.append(device);
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    emit q->deviceDiscovered(device);
}

void QBluetoothDeviceDiscoveryAgentPrivate::_q_propertyChanged(const QString &name,
                                                               const QDBusVariant &value)
{
    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << name << value.variant();

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

QT_END_NAMESPACE

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

#ifndef QBLUETOOTHSERVICEDISCOVERYAGENT_P_H
#define QBLUETOOTHSERVICEDISCOVERYAGENT_P_H

#include "qbluetoothaddress.h"
#include "qbluetoothdeviceinfo.h"
#include "qbluetoothserviceinfo.h"
#include "qbluetoothservicediscoveryagent.h"

#include <QStack>

#ifdef QT_BLUEZ_BLUETOOTH
class OrgBluezManagerInterface;
class OrgBluezAdapterInterface;
class OrgBluezDeviceInterface;
QT_BEGIN_NAMESPACE
class QDBusPendingCallWatcher;
class QXmlStreamReader;
QT_END_NAMESPACE
#endif

#ifdef QT_QNX_BLUETOOTH
#include "qnx/ppshelpers_p.h"
#include <fcntl.h>
#include <unistd.h>
#include <QTimer>
#include <btapi/btdevice.h>
#endif

QT_BEGIN_NAMESPACE

class QBluetoothDeviceDiscoveryAgent;
#ifdef QT_ANDROID_BLUETOOTH
class ServiceDiscoveryBroadcastReceiver;
class LocalDeviceBroadcastReceiver;
#include <QtAndroidExtras/QAndroidJniObject>
#include <QtBluetooth/QBluetoothLocalDevice>
#endif

class QBluetoothServiceDiscoveryAgentPrivate
#ifdef QT_QNX_BLUETOOTH
: public QObject
{
    Q_OBJECT
#else
{
#endif
    Q_DECLARE_PUBLIC(QBluetoothServiceDiscoveryAgent)

public:
    enum DiscoveryState {
        Inactive,
        DeviceDiscovery,
        ServiceDiscovery,
    };

    QBluetoothServiceDiscoveryAgentPrivate(const QBluetoothAddress &deviceAdapter);
    ~QBluetoothServiceDiscoveryAgentPrivate();

    void startDeviceDiscovery();
    void stopDeviceDiscovery();
    void startServiceDiscovery();
    void stopServiceDiscovery();

    void setDiscoveryState(DiscoveryState s) { state = s; }
    inline DiscoveryState discoveryState() { return state; }

    void setDiscoveryMode(QBluetoothServiceDiscoveryAgent::DiscoveryMode m) { mode = m; }
    QBluetoothServiceDiscoveryAgent::DiscoveryMode DiscoveryMode() { return mode; }

    // private slots
    void _q_deviceDiscoveryFinished();
    void _q_deviceDiscovered(const QBluetoothDeviceInfo &info);
    void _q_serviceDiscoveryFinished();
    void _q_deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error);
#ifdef QT_BLUEZ_BLUETOOTH
    void _q_discoveredServices(QDBusPendingCallWatcher *watcher);
    void _q_createdDevice(QDBusPendingCallWatcher *watcher);
#endif
#ifdef QT_ANDROID_BLUETOOTH
    void _q_processFetchedUuids(const QBluetoothAddress &address, const QList<QBluetoothUuid> &uuids);

    void populateDiscoveredServices(const QBluetoothDeviceInfo &remoteDevice,
                                    const QList<QBluetoothUuid> &uuids);
    void _q_fetchUuidsTimeout();
    void _q_hostModeStateChanged(QBluetoothLocalDevice::HostMode state);
#endif

private:
    void start(const QBluetoothAddress &address);
    void stop();

#ifdef QT_BLUEZ_BLUETOOTH
    QVariant readAttributeValue(QXmlStreamReader &xml);
#endif

#ifdef QT_QNX_BLUETOOTH
private Q_SLOTS:
    void remoteDevicesChanged(int fd);
    void controlReply(ppsResult result);
    void controlEvent(ppsResult result);
    void queryTimeout();
#ifdef QT_QNX_BT_BLUETOOTH
    static void deviceServicesDiscoveryCallback(bt_sdp_list_t *, void *, uint8_t);
#endif

private:
    int m_rdfd;
    QSocketNotifier *rdNotifier;
    QTimer m_queryTimer;
    bool m_btInitialized;
#endif

public:
    QBluetoothServiceDiscoveryAgent::Error error;
    QString errorString;
    QBluetoothAddress deviceAddress;
    QList<QBluetoothServiceInfo> discoveredServices;
    QList<QBluetoothDeviceInfo> discoveredDevices;
    QBluetoothAddress m_deviceAdapterAddress;

private:
    DiscoveryState state;
    QList<QBluetoothUuid> uuidFilter;

    QBluetoothDeviceDiscoveryAgent *deviceDiscoveryAgent;

    QBluetoothServiceDiscoveryAgent::DiscoveryMode mode;

    bool singleDevice;

#ifdef QT_BLUEZ_BLUETOOTH
    OrgBluezManagerInterface *manager;
    OrgBluezAdapterInterface *adapter;
    OrgBluezDeviceInterface *device;
#endif

#ifdef QT_ANDROID_BLUETOOTH
    ServiceDiscoveryBroadcastReceiver *receiver;
    LocalDeviceBroadcastReceiver *localDeviceReceiver;

    QAndroidJniObject btAdapter;
    QMap<QBluetoothAddress,QPair<QBluetoothDeviceInfo,QList<QBluetoothUuid> > > sdpCache;
#endif

protected:
    QBluetoothServiceDiscoveryAgent *q_ptr;
};

QT_END_NAMESPACE

#endif

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

#ifndef QBLUETOOTHDEVICEDISCOVERYAGENT_P_H
#define QBLUETOOTHDEVICEDISCOVERYAGENT_P_H

#include "qbluetoothdevicediscoveryagent.h"
#ifdef QT_ANDROID_BLUETOOTH
#include <QtAndroidExtras/QAndroidJniObject>
#include "android/devicediscoverybroadcastreceiver_p.h"
#endif

#include <QtCore/QVariantMap>

#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothLocalDevice>

#ifdef QT_BLUEZ_BLUETOOTH
class OrgBluezManagerInterface;
class OrgBluezAdapterInterface;

QT_BEGIN_NAMESPACE
class QDBusVariant;
QT_END_NAMESPACE
#elif defined(QT_QNX_BLUETOOTH)
#include "qnx/ppshelpers_p.h"
#include <QTimer>
#endif

QT_BEGIN_NAMESPACE

class QBluetoothDeviceDiscoveryAgentPrivate
#if defined(QT_QNX_BLUETOOTH) || defined(QT_ANDROID_BLUETOOTH)
: public QObject {
    Q_OBJECT
#else
{
#endif
    Q_DECLARE_PUBLIC(QBluetoothDeviceDiscoveryAgent)
public:
    QBluetoothDeviceDiscoveryAgentPrivate(const QBluetoothAddress &deviceAdapter);
    ~QBluetoothDeviceDiscoveryAgentPrivate();

    void start();
    void stop();
    bool isActive() const;

#ifdef QT_BLUEZ_BLUETOOTH
    void _q_deviceFound(const QString &address, const QVariantMap &dict);
    void _q_propertyChanged(const QString &name, const QDBusVariant &value);
#endif

private:
    QList<QBluetoothDeviceInfo> discoveredDevices;
    QBluetoothDeviceDiscoveryAgent::InquiryType inquiryType;

    QBluetoothDeviceDiscoveryAgent::Error lastError;
    QString errorString;

#ifdef QT_ANDROID_BLUETOOTH
private Q_SLOTS:
    void processDiscoveryFinished();
    void processDiscoveredDevices(const QBluetoothDeviceInfo& info);

private:
    DeviceDiscoveryBroadcastReceiver* receiver;
    QBluetoothAddress m_adapterAddress;
    bool m_active;
    QAndroidJniObject adapter;

    bool pendingCancel, pendingStart;
#elif defined(QT_BLUEZ_BLUETOOTH)
    QBluetoothAddress m_adapterAddress;
    bool pendingCancel;
    bool pendingStart;
    OrgBluezManagerInterface *manager;
    OrgBluezAdapterInterface *adapter;
#elif defined(QT_QNX_BLUETOOTH)
    private Q_SLOTS:
    void finished();
    void remoteDevicesChanged(int);
    void controlReply(ppsResult result);
    void controlEvent(ppsResult result);
    void startDeviceSearch();

private:
    QSocketNotifier *m_rdNotifier;
    QTimer m_finishedTimer;

    int m_rdfd;
    bool m_active;
    enum Ops{
        None,
        Cancel,
        Start
    };
    Ops m_nextOp;
    Ops m_currentOp;
    void processNextOp();
    bool isFinished;
#endif

    QBluetoothDeviceDiscoveryAgent *q_ptr;

};

QT_END_NAMESPACE

#endif

/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef QBLUETOOTHDEVICEDISCOVERYAGENT_P_H
#define QBLUETOOTHDEVICEDISCOVERYAGENT_P_H

#include "qbluetoothdevicediscoveryagent.h"

#include <QVariantMap>

#ifdef QT_BLUEZ_BLUETOOTH
class OrgBluezManagerInterface;
class OrgBluezAdapterInterface;

QT_BEGIN_NAMESPACE
class QDBusVariant;
QT_END_NAMESPACE
#endif

#ifdef QT_SYMBIAN_BLUETOOTH
#include "symbian/bluetoothlinkmanagerdevicediscoverer.h"
#include <es_sock.h>
#include <bt_sock.h>
#endif

QT_BEGIN_HEADER

QTBLUETOOTH_BEGIN_NAMESPACE

class QBluetoothDeviceDiscoveryAgentPrivate
{
    Q_DECLARE_PUBLIC(QBluetoothDeviceDiscoveryAgent)
public:
    QBluetoothDeviceDiscoveryAgentPrivate();
    ~QBluetoothDeviceDiscoveryAgentPrivate();

    void start();
    void stop();
    bool isActive() const;

#ifdef QT_SYMBIAN_BLUETOOTH
// private slots
    void _q_newDeviceFound(const QBluetoothDeviceInfo &device);
    void _q_setError(QBluetoothDeviceDiscoveryAgent::Error errorCode, 
            QString errorDescription);
#endif

#ifdef QT_BLUEZ_BLUETOOTH
    void _q_deviceFound(const QString &address, const QVariantMap &dict);
    void _q_propertyChanged(const QString &name, const QDBusVariant &value);
#endif

private:
#ifdef QT_SYMBIAN_BLUETOOTH
    void allocate();
    uint inquiryTypeToIAC() const;
#endif

    QList<QBluetoothDeviceInfo> discoveredDevices;
    QBluetoothDeviceDiscoveryAgent::InquiryType inquiryType;

    QBluetoothDeviceDiscoveryAgent::Error lastError;
    QString errorString;

    bool pendingCancel;
    bool pendingStart;

#ifdef QT_BLUEZ_BLUETOOTH
    OrgBluezManagerInterface *manager;
    OrgBluezAdapterInterface *adapter;
#endif

#ifdef QT_SYMBIAN_BLUETOOTH
    // shared socket servet among RHostResolvers
    RSocketServ m_socketServer;
    // active object for device discovery
    BluetoothLinkManagerDeviceDiscoverer *m_deviceDiscovery;

#endif

    QBluetoothDeviceDiscoveryAgent *q_ptr;

};

QTBLUETOOTH_END_NAMESPACE

QT_END_HEADER

#endif

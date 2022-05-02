/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Copyright (C) 2014 Denis Shienkov <denis.shienkov@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QBLUETOOTHLOCALDEVICE_P_H
#define QBLUETOOTHLOCALDEVICE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtBluetooth/qtbluetoothglobal.h>

#include "qbluetoothlocaldevice.h"

#if QT_CONFIG(bluez)
#include <QObject>
#include <QDBusObjectPath>
#include <QDBusMessage>
#include <QSet>
#include "bluez/bluez5_helper_p.h"

class OrgBluezAdapter1Interface;
class OrgFreedesktopDBusPropertiesInterface;
class OrgFreedesktopDBusObjectManagerInterface;
class OrgBluezDevice1Interface;

QT_BEGIN_NAMESPACE
class QDBusPendingCallWatcher;
QT_END_NAMESPACE
#endif

#ifdef QT_ANDROID_BLUETOOTH
#include <jni.h>
#include <QtCore/QJniEnvironment>
#include <QtCore/QJniObject>
#include <QtCore/QPair>
#endif

QT_BEGIN_NAMESPACE

extern void registerQBluetoothLocalDeviceMetaType();

class QBluetoothAddress;

#ifdef QT_ANDROID_BLUETOOTH
class LocalDeviceBroadcastReceiver;
class QBluetoothLocalDevicePrivate : public QObject
{
    Q_OBJECT
public:
    QBluetoothLocalDevicePrivate(
        QBluetoothLocalDevice *q, const QBluetoothAddress &address = QBluetoothAddress());
    ~QBluetoothLocalDevicePrivate();

    QJniObject *adapter();
    void initialize(const QBluetoothAddress &address);
    static bool startDiscovery();
    static bool cancelDiscovery();
    static bool isDiscovering();
    bool isValid() const;

private slots:
    void processHostModeChange(QBluetoothLocalDevice::HostMode newMode);
    void processPairingStateChanged(const QBluetoothAddress &address,
                                    QBluetoothLocalDevice::Pairing pairing);
    void processConnectDeviceChanges(const QBluetoothAddress &address, bool isConnectEvent);

private:
    QBluetoothLocalDevice *q_ptr;
    QJniObject *obj = nullptr;

    int pendingPairing(const QBluetoothAddress &address);

public:
    LocalDeviceBroadcastReceiver *receiver;
    bool pendingHostModeTransition = false;
    QList<QPair<QBluetoothAddress, bool> > pendingPairings;

    QList<QBluetoothAddress> connectedDevices;
};

#elif QT_CONFIG(bluez)
class QBluetoothLocalDevicePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QBluetoothLocalDevice)
public:
    QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *q,
                                 QBluetoothAddress localAddress = QBluetoothAddress());
    ~QBluetoothLocalDevicePrivate();

    QSet<QBluetoothAddress> connectedDevicesSet;
    OrgBluezAdapter1Interface *adapter = nullptr;
    OrgFreedesktopDBusPropertiesInterface *adapterProperties = nullptr;
    OrgFreedesktopDBusObjectManagerInterface *manager = nullptr;
    QMap<QString, OrgFreedesktopDBusPropertiesInterface *> deviceChangeMonitors;

    QList<QBluetoothAddress> connectedDevices() const;

    QBluetoothAddress localAddress;
    QBluetoothAddress address;
    QBluetoothLocalDevice::Pairing pairing;
    OrgBluezDevice1Interface *pairingTarget = nullptr;
    QTimer *pairingDiscoveryTimer = nullptr;
    QBluetoothLocalDevice::HostMode currentMode;
    int pendingHostModeChange;

public slots:
    void pairingCompleted(QDBusPendingCallWatcher *);

    bool isValid() const;

    void requestPairing(const QBluetoothAddress &address,
                        QBluetoothLocalDevice::Pairing targetPairing);

private Q_SLOTS:
    void PropertiesChanged(const QString &interface,
                           const QVariantMap &changed_properties,
                           const QStringList &invalidated_properties,
                           const QDBusMessage &signal);
    void InterfacesAdded(const QDBusObjectPath &object_path,
                         InterfaceList interfaces_and_properties);
    void InterfacesRemoved(const QDBusObjectPath &object_path,
                           const QStringList &interfaces);
    void processPairing(const QString &objectPath, QBluetoothLocalDevice::Pairing target);
    void pairingDiscoveryTimedOut();

private:
    void connectDeviceChanges();

    QString deviceAdapterPath;

    QBluetoothLocalDevice *q_ptr;

    void initializeAdapter();
};

#elif defined(QT_WINRT_BLUETOOTH)
class QBluetoothLocalDevicePrivate : public QObject
{
    Q_DECLARE_PUBLIC(QBluetoothLocalDevice)
public:
    QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *q,
                                 QBluetoothAddress = QBluetoothAddress());
    ~QBluetoothLocalDevicePrivate();

    bool isValid() const;

private:
    QBluetoothLocalDevice *q_ptr;
};
#elif !defined(QT_OSX_BLUETOOTH) // dummy backend
class QBluetoothLocalDevicePrivate : public QObject
{
public:
    QBluetoothLocalDevicePrivate(QBluetoothLocalDevice * = nullptr,
                                 QBluetoothAddress = QBluetoothAddress())
    {
    }

    bool isValid() const
    {
        return false;
    }
};
#endif

QT_END_NAMESPACE

#endif // QBLUETOOTHLOCALDEVICE_P_H

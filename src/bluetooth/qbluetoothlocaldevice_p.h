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

#ifndef QBLUETOOTHLOCALDEVICE_P_H
#define QBLUETOOTHLOCALDEVICE_P_H

#include <QtBluetooth/qbluetoothglobal.h>

#include "qbluetoothlocaldevice.h"

#ifdef QT_BLUEZ_BLUETOOTH
#include <QObject>
#include <QDBusContext>
#include <QDBusObjectPath>
#include <QDBusMessage>
#include <QSet>

class OrgBluezAdapterInterface;
class OrgBluezAgentAdaptor;
class OrgBluezDeviceInterface;
class OrgBluezManagerInterface;

QT_BEGIN_NAMESPACE
class QDBusPendingCallWatcher;
QT_END_NAMESPACE

#elif defined(QT_QNX_BLUETOOTH)
#include <QSocketNotifier>
#include "qnx/ppshelpers_p.h"
#endif
#ifdef QT_ANDROID_BLUETOOTH
#include <jni.h>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtAndroidExtras/QAndroidJniObject>
#include <QtCore/QPair>
#endif

QT_BEGIN_NAMESPACE

class QBluetoothAddress;

#ifdef QT_ANDROID_BLUETOOTH
class LocalDeviceBroadcastReceiver;
class QBluetoothLocalDevicePrivate : public QObject
{
    Q_OBJECT
public:
    QBluetoothLocalDevicePrivate(
            QBluetoothLocalDevice *q,
            const QBluetoothAddress &address = QBluetoothAddress());
    ~QBluetoothLocalDevicePrivate();

    QAndroidJniObject *adapter();
    void initialize(const QBluetoothAddress& address);
    static bool startDiscovery();
    static bool cancelDiscovery();
    static bool isDiscovering();
    bool isValid() const;


private slots:
    void processHostModeChange(QBluetoothLocalDevice::HostMode newMode);
    void processPairingStateChanged(const QBluetoothAddress &address,
                            QBluetoothLocalDevice::Pairing pairing);
    void processConnectDeviceChanges(const QBluetoothAddress &address, bool isConnectEvent);
    void processDisplayConfirmation(const QBluetoothAddress &address, const QString &pin);

private:
    QBluetoothLocalDevice *q_ptr;
    QAndroidJniObject *obj;

    int pendingPairing(const QBluetoothAddress &address);

public:
    LocalDeviceBroadcastReceiver *receiver;
    bool pendingHostModeTransition;
    QList<QPair<QBluetoothAddress, bool> > pendingPairings;

    QList<QBluetoothAddress> connectedDevices;
};

#elif defined(QT_BLUEZ_BLUETOOTH)
class QBluetoothLocalDevicePrivate : public QObject,
                                     protected QDBusContext
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QBluetoothLocalDevice)
public:
    QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *q, QBluetoothAddress localAddress = QBluetoothAddress());
    ~QBluetoothLocalDevicePrivate();

    QSet<OrgBluezDeviceInterface *> devices;
    QSet<QBluetoothAddress> connectedDevicesSet;
    OrgBluezAdapterInterface *adapter;
    OrgBluezAgentAdaptor *agent;
    OrgBluezManagerInterface *manager;

    QList<QBluetoothAddress> connectedDevices() const;

    QString agent_path;
    QBluetoothAddress localAddress;
    QBluetoothAddress address;
    QBluetoothLocalDevice::Pairing pairing;
    QBluetoothLocalDevice::HostMode currentMode;
    int pendingHostModeChange;

public Q_SLOTS: // METHODS
    void Authorize(const QDBusObjectPath &in0, const QString &in1);
    void Cancel();
    void ConfirmModeChange(const QString &in0);
    void DisplayPasskey(const QDBusObjectPath &in0, uint in1, uchar in2);
    void Release();
    uint RequestPasskey(const QDBusObjectPath &in0);

    void RequestConfirmation(const QDBusObjectPath &in0, uint in1);
    QString RequestPinCode(const QDBusObjectPath &in0);

    void pairingCompleted(QDBusPendingCallWatcher*);

    void PropertyChanged(QString,QDBusVariant);
    void _q_deviceCreated(const QDBusObjectPath &device);
    void _q_deviceRemoved(const QDBusObjectPath &device);
    void _q_devicePropertyChanged(const QString &property, const QDBusVariant &value);
    bool isValid() const;
    void adapterRemoved(const QDBusObjectPath &device);

private:
    void createCache();
    void connectDeviceChanges();

    QDBusMessage msgConfirmation;
    QDBusConnection *msgConnection;

    QBluetoothLocalDevice *q_ptr;

    void initializeAdapter();
};

#elif defined(QT_QNX_BLUETOOTH)

class QBluetoothLocalDevicePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QBluetoothLocalDevice)
public:
    QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *);
    ~QBluetoothLocalDevicePrivate();

    static QString name();
    static QBluetoothAddress address();

    void powerOn();
    void powerOff();

    void setHostMode(QBluetoothLocalDevice::HostMode mode);
    QBluetoothLocalDevice::HostMode hostMode() const;

    void requestPairing(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing);

    void setAccess(int);
    // This method will be used for emitting signals.
    void connectedDevices();

    Q_INVOKABLE void controlReply(ppsResult res);
    Q_INVOKABLE void controlEvent(ppsResult res);

    bool isValid() const;

private:
    QBluetoothLocalDevice *q_ptr;
    bool isValidDevice;
    QList<QBluetoothAddress> connectedDevicesSet;
};
#else
class QBluetoothLocalDevicePrivate : public QObject
{
public:
    bool isValid() const
    {
        return false;
    }
};
#endif

QT_END_NAMESPACE

#endif // QBLUETOOTHLOCALDEVICE_P_H

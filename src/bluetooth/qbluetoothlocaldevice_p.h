/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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

#include "qbluetoothglobal.h"

#include "qbluetoothlocaldevice.h"

#ifdef QT_BLUEZ_BLUETOOTH
#include <QObject>
#include <QDBusContext>
#include <QDBusObjectPath>
#include <QDBusMessage>

class OrgBluezAdapterInterface;
class OrgBluezAgentAdaptor;

QT_BEGIN_NAMESPACE
class QDBusPendingCallWatcher;
QT_END_NAMESPACE

#endif

QT_BEGIN_HEADER

QTBLUETOOTH_BEGIN_NAMESPACE

class QBluetoothAddress;

#if defined(QT_BLUEZ_BLUETOOTH)
class QBluetoothLocalDevicePrivate : public QObject,
                                     protected QDBusContext
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QBluetoothLocalDevice)
public:
    QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *q, QBluetoothAddress localAddress = QBluetoothAddress());
    ~QBluetoothLocalDevicePrivate();

    OrgBluezAdapterInterface *adapter;
    OrgBluezAgentAdaptor *agent;
    QString agent_path;
    QBluetoothAddress localAddress;
    QBluetoothAddress address;
    QBluetoothLocalDevice::Pairing pairing;
    QBluetoothLocalDevice::HostMode currentMode;

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

private:
    QDBusMessage msgConfirmation;
    QDBusConnection *msgConnection;

    QBluetoothLocalDevice *q_ptr;

    void initializeAdapter();
};

#else
class QBluetoothLocalDevicePrivate : public QObject
{
};
#endif

QTBLUETOOTH_END_NAMESPACE

QT_END_HEADER

#endif // QBLUETOOTHLOCALDEVICE_P_H

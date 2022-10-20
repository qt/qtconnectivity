/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/QLoggingCategory>
#include <QtCore/qrandom.h>
#include "localdevicebroadcastreceiver_p.h"
#include "android/jni_android_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

const char *scanModes[] = {"SCAN_MODE_NONE", "SCAN_MODE_CONNECTABLE", "SCAN_MODE_CONNECTABLE_DISCOVERABLE"};
const char *bondModes[] = {"BOND_NONE", "BOND_BONDING", "BOND_BONDED"};

LocalDeviceBroadcastReceiver::LocalDeviceBroadcastReceiver(QObject *parent) :
    AndroidBroadcastReceiver(parent), previousScanMode(0)
{
    addAction(valueForStaticField(JavaNames::BluetoothDevice, JavaNames::ActionBondStateChanged));
    addAction(valueForStaticField(JavaNames::BluetoothAdapter, JavaNames::ActionScanModeChanged));
    addAction(valueForStaticField(JavaNames::BluetoothDevice, JavaNames::ActionAclConnected));
    addAction(valueForStaticField(JavaNames::BluetoothDevice, JavaNames::ActionAclDisconnected));

    //cache integer values for host & bonding mode
    //don't use the java fields directly but refer to them by name
    for (uint i = 0; i < (sizeof(hostModePreset)/sizeof(hostModePreset[0])); i++) {
        hostModePreset[i] = QJniObject::getStaticField<jint>(
                                                "android/bluetooth/BluetoothAdapter",
                                                scanModes[i]);
    }

    for (uint i = 0; i < (sizeof(bondingModePreset)/sizeof(bondingModePreset[0])); i++) {
        bondingModePreset[i] = QJniObject::getStaticField<jint>(
                                                "android/bluetooth/BluetoothDevice",
                                                bondModes[i]);
    }
}

void LocalDeviceBroadcastReceiver::onReceive(JNIEnv *env, jobject context, jobject intent)
{
    Q_UNUSED(context);
    Q_UNUSED(env);

    QJniObject intentObject(intent);
    const QString action = intentObject.callObjectMethod("getAction", "()Ljava/lang/String;").toString();
    qCDebug(QT_BT_ANDROID) << QStringLiteral("LocalDeviceBroadcastReceiver::onReceive() - event: %1").arg(action);

    if (action == valueForStaticField(JavaNames::BluetoothAdapter,
                                      JavaNames::ActionScanModeChanged).toString()) {

        const QJniObject extrasBundle =
                intentObject.callObjectMethod("getExtras","()Landroid/os/Bundle;");
        const QJniObject keyExtra = valueForStaticField(JavaNames::BluetoothAdapter,
                                                               JavaNames::ExtraScanMode);

        int extra = extrasBundle.callMethod<jint>("getInt",
                                                  "(Ljava/lang/String;)I",
                                                  keyExtra.object<jstring>());

        if (previousScanMode != extra) {
            previousScanMode = extra;

            if (extra == hostModePreset[0])
                emit hostModeStateChanged(QBluetoothLocalDevice::HostPoweredOff);
            else if (extra == hostModePreset[1])
                emit hostModeStateChanged(QBluetoothLocalDevice::HostConnectable);
            else if (extra == hostModePreset[2])
                emit hostModeStateChanged(QBluetoothLocalDevice::HostDiscoverable);
            else
                qCWarning(QT_BT_ANDROID) << "Unknown Host State";
        }
    } else if (action == valueForStaticField(JavaNames::BluetoothDevice,
                                             JavaNames::ActionBondStateChanged).toString()) {
        //get BluetoothDevice
        QJniObject keyExtra = valueForStaticField(JavaNames::BluetoothDevice,
                                                         JavaNames::ExtraDevice);
        const QJniObject bluetoothDevice =
                intentObject.callObjectMethod("getParcelableExtra",
                                              "(Ljava/lang/String;)Landroid/os/Parcelable;",
                                              keyExtra.object<jstring>());

        //get new bond state
        keyExtra = valueForStaticField(JavaNames::BluetoothDevice, JavaNames::ExtraBondState);
        const QJniObject extrasBundle =
                intentObject.callObjectMethod("getExtras","()Landroid/os/Bundle;");
        int bondState = extrasBundle.callMethod<jint>("getInt",
                                                      "(Ljava/lang/String;)I",
                                                      keyExtra.object<jstring>());

        QBluetoothAddress address(bluetoothDevice.callObjectMethod<jstring>("getAddress").toString());
        if (address.isNull())
                return;

        if (bondState == bondingModePreset[0])
            emit pairingStateChanged(address, QBluetoothLocalDevice::Unpaired);
        else if (bondState == bondingModePreset[1])
            ; //we ignore this as Qt doesn't have equivalent API value
        else if (bondState == bondingModePreset[2])
            emit pairingStateChanged(address, QBluetoothLocalDevice::Paired);
        else
            qCWarning(QT_BT_ANDROID) << "Unknown BOND_STATE_CHANGED value:" << bondState;

    } else if (action == valueForStaticField(JavaNames::BluetoothDevice,
                                             JavaNames::ActionAclConnected).toString() ||
               action == valueForStaticField(JavaNames::BluetoothDevice,
                                             JavaNames::ActionAclDisconnected).toString()) {

        const QString connectEvent = valueForStaticField(JavaNames::BluetoothDevice,
                                                         JavaNames::ActionAclConnected).toString();
        const bool isConnectEvent =
                action == connectEvent ? true : false;

        //get BluetoothDevice
        const QJniObject keyExtra = valueForStaticField(JavaNames::BluetoothDevice,
                                                               JavaNames::ExtraDevice);
        QJniObject bluetoothDevice =
                intentObject.callObjectMethod("getParcelableExtra",
                                              "(Ljava/lang/String;)Landroid/os/Parcelable;",
                                              keyExtra.object<jstring>());

        QBluetoothAddress address(bluetoothDevice.callObjectMethod<jstring>("getAddress").toString());
        if (address.isNull())
            return;

        emit connectDeviceChanges(address, isConnectEvent);
    }
}

QT_END_NAMESPACE

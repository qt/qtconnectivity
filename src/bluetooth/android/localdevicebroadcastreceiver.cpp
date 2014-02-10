/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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
#include "localdevicebroadcastreceiver_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

LocalDeviceBroadcastReceiver::LocalDeviceBroadcastReceiver(QObject *parent) :
    AndroidBroadcastReceiver(parent), previousScanMode(0)
{
    addAction(QStringLiteral("android.bluetooth.device.action.BOND_STATE_CHANGED"));
    addAction(QStringLiteral("android.bluetooth.adapter.action.SCAN_MODE_CHANGED"));
    addAction(QStringLiteral("android.bluetooth.device.action.ACL_CONNECTED"));
    addAction(QStringLiteral("android.bluetooth.device.action.ACL_DISCONNECTED"));
    addAction(QStringLiteral("android.bluetooth.device.action.PAIRING_REQUEST")); //API 19

}

void LocalDeviceBroadcastReceiver::onReceive(JNIEnv *env, jobject context, jobject intent)
{
    Q_UNUSED(context);
    Q_UNUSED(env);

    QAndroidJniObject intentObject(intent);
    const QString action = intentObject.callObjectMethod("getAction", "()Ljava/lang/String;").toString();
    qCDebug(QT_BT_ANDROID) << QStringLiteral("LocalDeviceBroadcastReceiver::onReceive() - event: %1").arg(action);

    if (action == QStringLiteral("android.bluetooth.adapter.action.SCAN_MODE_CHANGED")) {
        QAndroidJniObject extrasBundle =
                intentObject.callObjectMethod("getExtras","()Landroid/os/Bundle;");
        QAndroidJniObject keyExtra = QAndroidJniObject::fromString(
                    QStringLiteral("android.bluetooth.adapter.extra.SCAN_MODE"));

        int extra = extrasBundle.callMethod<jint>("getInt",
                                                  "(Ljava/lang/String;)I",
                                                  keyExtra.object<jstring>());

        if (previousScanMode != extra) {
            previousScanMode = extra;

            switch (extra) {
                case 20: //BluetoothAdapter.SCAN_MODE_NONE
                    emit hostModeStateChanged(QBluetoothLocalDevice::HostPoweredOff);
                    break;
                case 21: //BluetoothAdapter.SCAN_MODE_CONNECTABLE
                    emit hostModeStateChanged(QBluetoothLocalDevice::HostConnectable);
                    break;
                case 23: //BluetoothAdapter.SCAN_MODE_CONNECTABLE_DISCOVERABLE
                    emit hostModeStateChanged(QBluetoothLocalDevice::HostDiscoverable);
                    break;
                default:
                    qCWarning(QT_BT_ANDROID) << "Unknown Host State";
                    break;
            }
        }
    } else if (action == QStringLiteral("android.bluetooth.device.action.BOND_STATE_CHANGED")) {
        //get BluetoothDevice
        QAndroidJniObject keyExtra = QAndroidJniObject::fromString(
                            QStringLiteral("android.bluetooth.device.extra.DEVICE"));
        QAndroidJniObject bluetoothDevice =
                intentObject.callObjectMethod("getParcelableExtra",
                                              "(Ljava/lang/String;)Landroid/os/Parcelable;",
                                              keyExtra.object<jstring>());

        //get new bond state
        keyExtra = QAndroidJniObject::fromString(
                            QStringLiteral("android.bluetooth.device.extra.BOND_STATE"));
        QAndroidJniObject extrasBundle =
                intentObject.callObjectMethod("getExtras","()Landroid/os/Bundle;");
        int bondState = extrasBundle.callMethod<jint>("getInt",
                                                      "(Ljava/lang/String;)I",
                                                      keyExtra.object<jstring>());

        QBluetoothAddress address(bluetoothDevice.callObjectMethod<jstring>("getAddress").toString());
        if (address.isNull())
                return;

        switch (bondState) {
        case 10: //BluetoothDevice.BOND_NONE
            emit pairingStateChanged(address, QBluetoothLocalDevice::Unpaired);
            break;
        case 11: //BluetoothDevice.BOND_BONDING
            //we ignore this as Qt doesn't have equivalent API.
            break;
        case 12: //BluetoothDevice.BOND_BONDED
            emit pairingStateChanged(address, QBluetoothLocalDevice::Paired);
            break;
        default:
            qCWarning(QT_BT_ANDROID) << "Unknown BOND_STATE_CHANGED value:" << bondState;
            break;
        }
    } else if (action == QStringLiteral("android.bluetooth.device.action.ACL_DISCONNECTED") ||
               action == QStringLiteral("android.bluetooth.device.action.ACL_CONNECTED")) {

        const bool isConnectEvent =
                action == QStringLiteral("android.bluetooth.device.action.ACL_CONNECTED") ? true : false;

        //get BluetoothDevice
        QAndroidJniObject keyExtra = QAndroidJniObject::fromString(
                            QStringLiteral("android.bluetooth.device.extra.DEVICE"));
        QAndroidJniObject bluetoothDevice =
                intentObject.callObjectMethod("getParcelableExtra",
                                              "(Ljava/lang/String;)Landroid/os/Parcelable;",
                                              keyExtra.object<jstring>());

        QBluetoothAddress address(bluetoothDevice.callObjectMethod<jstring>("getAddress").toString());
        if (address.isNull())
            return;

        emit connectDeviceChanges(address, isConnectEvent);
    } else if (action == QStringLiteral("android.bluetooth.device.action.PAIRING_REQUEST")) {

        QAndroidJniObject keyExtra = QAndroidJniObject::fromString(
                    QStringLiteral("android.bluetooth.device.extra.PAIRING_VARIANT"));
        int variant = intentObject.callMethod<jint>("getIntExtra",
                                                    "(Ljava/lang/String;I)I",
                                                    keyExtra.object<jstring>(),
                                                    -1);

        int key = -1;
        switch (variant) {
        case -1: //ignore -> no pairing variant set
            return;
        case 2: //BluetoothDevice.PAIRING_VARIANT_PASSKEY_CONFIRMATION
        {
            keyExtra = QAndroidJniObject::fromString(
                        QStringLiteral("android.bluetooth.device.extra.PAIRING_KEY"));
            key = intentObject.callMethod<jint>("getIntExtra",
                                                    "(Ljava/lang/String;I)I",
                                                    keyExtra.object<jstring>(),
                                                    -1);
            if (key == -1)
                return;

            keyExtra = QAndroidJniObject::fromString(
                                QStringLiteral("android.bluetooth.device.extra.DEVICE"));
            QAndroidJniObject bluetoothDevice =
                    intentObject.callObjectMethod("getParcelableExtra",
                                                  "(Ljava/lang/String;)Landroid/os/Parcelable;",
                                                  keyExtra.object<jstring>());

            //we need to keep a reference around in case the user confirms later on
            pairingDevice = bluetoothDevice;

            QBluetoothAddress address(bluetoothDevice.callObjectMethod<jstring>("getAddress").toString());

            //User has choice to confirm or not. If no confirmation is happening
            //the OS default pairing dialog can be used or timeout occurs.
            emit pairingDisplayConfirmation(address, QString::number(key));
            break;
        }
        default:
            qCWarning(QT_BT_ANDROID) << "Unknown pairing variant: " << variant;
            return;
        }
    }
}

bool LocalDeviceBroadcastReceiver::pairingConfirmation(bool accept)
{
    if (!pairingDevice.isValid())
        return false;

    bool success = pairingDevice.callMethod<jboolean>("setPairingConfirmation",
                                              "(Z)Z", accept);
    pairingDevice = QAndroidJniObject();
    return success;
}

QT_END_NAMESPACE

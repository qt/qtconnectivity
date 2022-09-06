// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "android/servicediscoverybroadcastreceiver_p.h"
#include <QCoreApplication>
#include <QtCore/QLoggingCategory>
#include <QtCore/QJniEnvironment>
#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothDeviceInfo>
#include "android/jni_android_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

ServiceDiscoveryBroadcastReceiver::ServiceDiscoveryBroadcastReceiver(QObject* parent): AndroidBroadcastReceiver(parent)
{
    addAction(valueForStaticField<QtJniTypes::BluetoothDevice, JavaNames::ActionUuid>());
}

void ServiceDiscoveryBroadcastReceiver::onReceive(JNIEnv *env, jobject context, jobject intent)
{
    Q_UNUSED(context);
    Q_UNUSED(env);

    QJniObject intentObject(intent);
    const QString action = intentObject.callMethod<jstring>("getAction").toString();

    qCDebug(QT_BT_ANDROID) << "ServiceDiscoveryBroadcastReceiver::onReceive() - event:" << action;

    if (action == valueForStaticField<QtJniTypes::BluetoothDevice,
                                      JavaNames::ActionUuid>().toString()) {

        QJniObject keyExtra = valueForStaticField<QtJniTypes::BluetoothDevice,
                                                         JavaNames::ExtraUuid>();
        QJniObject parcelableUuids = intentObject.callMethod<QtJniTypes::ParcelableArray>(
                                                "getParcelableArrayExtra",
                                                keyExtra.object<jstring>());
        if (!parcelableUuids.isValid()) {
            emit uuidFetchFinished(QBluetoothAddress(), QList<QBluetoothUuid>());
            return;
        }
        const QList<QBluetoothUuid> result = ServiceDiscoveryBroadcastReceiver::convertParcelableArray(parcelableUuids);

        keyExtra = valueForStaticField<QtJniTypes::BluetoothDevice, JavaNames::ExtraDevice>();
        QJniObject bluetoothDevice =
        intentObject.callMethod<QtJniTypes::Parcelable>("getParcelableExtra",
                                  keyExtra.object<jstring>());
        QBluetoothAddress address;
        if (bluetoothDevice.isValid()) {
            address =
                    QBluetoothAddress(bluetoothDevice.callMethod<jstring>("getAddress").toString());
            emit uuidFetchFinished(address, result);
        } else {
            emit uuidFetchFinished(QBluetoothAddress(), QList<QBluetoothUuid>());
        }
    }
}

QList<QBluetoothUuid> ServiceDiscoveryBroadcastReceiver::convertParcelableArray(const QJniObject &parcelUuidArray)
{
    QList<QBluetoothUuid> result;
    QJniEnvironment env;

    jobjectArray parcels = parcelUuidArray.object<jobjectArray>();
    if (!parcels)
        return result;

    jint size = env->GetArrayLength(parcels);
    for (jint i = 0; i < size; ++i) {
        auto p = QJniObject::fromLocalRef(env->GetObjectArrayElement(parcels, i));

        QBluetoothUuid uuid(p.callMethod<jstring>("toString").toString());
        //qCDebug(QT_BT_ANDROID) << uuid.toString();
        result.append(uuid);
    }

    return result;
}

QT_END_NAMESPACE


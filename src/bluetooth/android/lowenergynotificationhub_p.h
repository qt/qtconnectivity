// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef LOWENERGYNOTIFICATIONHUB_H
#define LOWENERGYNOTIFICATIONHUB_H

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

#include <QtCore/QObject>
#include <QtCore/QReadWriteLock>
#include <QtCore/QJniObject>
#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QLowEnergyController>
#include <QtBluetooth/QLowEnergyService>
#include <QtCore/private/qglobal_p.h>
#include <jni.h>

#include <QtBluetooth/QLowEnergyCharacteristic>

QT_BEGIN_NAMESPACE

class LowEnergyNotificationHub : public QObject
{
    Q_OBJECT
public:
    explicit LowEnergyNotificationHub(const QBluetoothAddress &remote, bool isPeripheral,
                                      QObject *parent = nullptr);
    ~LowEnergyNotificationHub();

    static void lowEnergy_connectionChange(JNIEnv*, jobject, jlong qtObject,
                                           jint errorCode, jint newState);
    static void lowEnergy_mtuChanged(JNIEnv*, jobject, jlong qtObject,
                                           jint mtu);
    static void lowEnergy_servicesDiscovered(JNIEnv*, jobject, jlong qtObject,
                                             jint errorCode, jobject uuidList);
    static void lowEnergy_serviceDetailsDiscovered(JNIEnv *, jobject,
                                                   jlong qtObject, jobject uuid,
                                                   jint startHandle, jint endHandle);
    static void lowEnergy_characteristicRead(JNIEnv*env, jobject, jlong qtObject,
                                             jobject serviceUuid,
                                             jint handle, jobject charUuid,
                                             jint properties, jbyteArray data);
    static void lowEnergy_descriptorRead(JNIEnv *env, jobject, jlong qtObject,
                                         jobject sUuid, jobject cUuid,
                                         jint handle, jobject dUuid, jbyteArray data);
    static void lowEnergy_characteristicWritten(JNIEnv *, jobject, jlong qtObject,
                                                jint charHandle, jbyteArray data,
                                                jint errorCode);
    static void lowEnergy_descriptorWritten(JNIEnv *, jobject, jlong qtObject,
                                            jint descHandle, jbyteArray data,
                                            jint errorCode);
    static void lowEnergy_serverDescriptorWritten(JNIEnv *, jobject, jlong qtObject,
                                                  jobject descriptor, jbyteArray newValue);
    static void lowEnergy_characteristicChanged(JNIEnv *, jobject, jlong qtObject,
                                                jint charHandle, jbyteArray data);
    static void lowEnergy_serverCharacteristicChanged(JNIEnv *, jobject, jlong qtObject,
                                                jobject characteristic, jbyteArray newValue);
    static void lowEnergy_serviceError(JNIEnv *, jobject, jlong qtObject,
                                       jint attributeHandle, int errorCode);
    static void lowEnergy_advertisementError(JNIEnv *, jobject, jlong qtObject,
                                               jint status);

    QJniObject javaObject()
    {
        return jBluetoothLe;
    }

signals:
    void connectionUpdated(QLowEnergyController::ControllerState newState,
            QLowEnergyController::Error errorCode);
    void mtuChanged(int mtu);
    void servicesDiscovered(QLowEnergyController::Error errorCode, const QString &uuids);
    void serviceDetailsDiscoveryFinished(const QString& serviceUuid,
            int startHandle, int endHandle);
    void characteristicRead(const QBluetoothUuid &serviceUuid,
            int handle, const QBluetoothUuid &charUuid,
            int properties, const QByteArray &data);
    void descriptorRead(const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid,
            int handle, const QBluetoothUuid &descUuid, const QByteArray &data);
    void characteristicWritten(int charHandle, const QByteArray &data,
                               QLowEnergyService::ServiceError errorCode);
    void descriptorWritten(int descHandle, const QByteArray &data,
                           QLowEnergyService::ServiceError errorCode);
    void serverDescriptorWritten(const QJniObject &descriptor, const QByteArray &newValue);
    void characteristicChanged(int charHandle, const QByteArray &data);
    void serverCharacteristicChanged(const QJniObject &characteristic, const QByteArray &newValue);
    void serviceError(int attributeHandle, QLowEnergyService::ServiceError errorCode);
    void advertisementError(int status);

public slots:
private:
    static QReadWriteLock lock;

    QJniObject jBluetoothLe;
    long javaToCtoken;

};

QT_END_NAMESPACE

#endif // LOWENERGYNOTIFICATIONHUB_H


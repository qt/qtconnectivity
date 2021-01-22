/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
****************************************************************************/

#include "android/androidbroadcastreceiver_p.h"
#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothLocalDevice>

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

#ifndef LOCALDEVICEBROADCASTRECEIVER_H
#define LOCALDEVICEBROADCASTRECEIVER_H

QT_BEGIN_NAMESPACE

class LocalDeviceBroadcastReceiver : public AndroidBroadcastReceiver
{
    Q_OBJECT
public:
    explicit LocalDeviceBroadcastReceiver(QObject *parent = nullptr);
    virtual ~LocalDeviceBroadcastReceiver() {}
    virtual void onReceive(JNIEnv *env, jobject context, jobject intent);
    virtual void onReceiveLeScan(JNIEnv *, jobject, jint, jbyteArray) {}
    bool pairingConfirmation(bool accept);

signals:
    void hostModeStateChanged(QBluetoothLocalDevice::HostMode state);
    void pairingStateChanged(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing);
    void connectDeviceChanges(const QBluetoothAddress &address, bool isConnectEvent);
    void pairingDisplayConfirmation(const QBluetoothAddress &address, const QString& pin);
    void pairingDisplayPinCode(const QBluetoothAddress &address, const QString& pin);
private:
    int previousScanMode;
    QAndroidJniObject pairingDevice;

    int bondingModePreset[3];
    int hostModePreset[3];
};

QT_END_NAMESPACE

#endif // LOCALDEVICEBROADCASTRECEIVER_H

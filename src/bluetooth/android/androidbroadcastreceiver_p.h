/****************************************************************************
**
** Copyright (C) 2016 Lauri Laanmets (Proekspert AS) <lauri.laanmets@eesti.ee>
** Copyright (C) 2022 The Qt Company Ltd.
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

#ifndef JNIBROADCASTRECEIVER_H
#define JNIBROADCASTRECEIVER_H

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

#include <jni.h>
#include <QtCore/QObject>
#include <android/log.h>
#include <QtCore/QJniObject>

QT_BEGIN_NAMESPACE

void QtBroadcastReceiver_jniOnReceive(JNIEnv *, jobject, jlong, jobject, jobject);

class AndroidBroadcastReceiver: public QObject
{
    Q_OBJECT
public:
    AndroidBroadcastReceiver(QObject* parent = nullptr);
    virtual ~AndroidBroadcastReceiver();

    void addAction(const QJniObject &filter);
    bool isValid() const;
    void unregisterReceiver();

protected:
    friend void QtBroadcastReceiver_jniOnReceive(JNIEnv *, jobject, jlong, jobject, jobject);
    virtual void onReceive(JNIEnv *env, jobject context, jobject intent) = 0;
    friend void QtBluetoothLE_leScanResult(JNIEnv *, jobject, jlong, jobject, jint, jbyteArray);
    virtual void onReceiveLeScan(JNIEnv *env, jobject jBluetoothDevice, jint rssi, jbyteArray scanRecord) = 0;


    QJniObject contextObject;
    QJniObject intentFilterObject;
    QJniObject broadcastReceiverObject;
    bool valid;
};

QT_END_NAMESPACE
#endif // JNIBROADCASTRECEIVER_H

// Copyright (C) 2016 Lauri Laanmets (Proekspert AS) <lauri.laanmets@eesti.ee>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <android/log.h>
#include "android/androidbroadcastreceiver_p.h"
#include <QtCore/QLoggingCategory>
#include <QtCore/qnativeinterface.h>
#include <QtGui/QGuiApplication>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)


AndroidBroadcastReceiver::AndroidBroadcastReceiver(QObject* parent)
    : QObject(parent), valid(false)
{
    // get Qt Context
    contextObject = QJniObject(QNativeInterface::QAndroidApplication::context());

    broadcastReceiverObject = QJniObject("org/qtproject/qt/android/bluetooth/QtBluetoothBroadcastReceiver");
    if (!broadcastReceiverObject.isValid())
        return;
    broadcastReceiverObject.setField<jlong>("qtObject", reinterpret_cast<long>(this));

    intentFilterObject = QJniObject("android/content/IntentFilter");
    if (!intentFilterObject.isValid())
        return;

    valid = true;
}

AndroidBroadcastReceiver::~AndroidBroadcastReceiver()
{
}

bool AndroidBroadcastReceiver::isValid() const
{
    return valid;
}

void AndroidBroadcastReceiver::unregisterReceiver()
{
    if (!valid)
        return;

    broadcastReceiverObject.callMethod<void>("unregisterReceiver");
}

void AndroidBroadcastReceiver::addAction(const QJniObject &action)
{
    if (!valid || !action.isValid())
        return;

    intentFilterObject.callMethod<void>("addAction", "(Ljava/lang/String;)V", action.object<jstring>());

    contextObject.callObjectMethod(
                "registerReceiver",
                "(Landroid/content/BroadcastReceiver;Landroid/content/IntentFilter;)Landroid/content/Intent;",
                broadcastReceiverObject.object<jobject>(),
                intentFilterObject.object<jobject>());
}

QT_END_NAMESPACE

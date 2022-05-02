/****************************************************************************
**
** Copyright (C) 2016 Centria research and development
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
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

#include "androidjninfc_p.h"

#include <android/log.h>

#include "androidmainnewintentlistener_p.h"

#include "qglobal.h"
#include "qbytearray.h"
#include "qdebug.h"

static const char *nfcClassName = "org/qtproject/qt/android/nfc/QtNfc";

static AndroidNfc::MainNfcNewIntentListener mainListener;

QT_BEGIN_ANDROIDNFC_NAMESPACE

bool startDiscovery()
{
    return QJniObject::callStaticMethod<jboolean>(nfcClassName,"start");
}

bool isEnabled()
{
    return QJniObject::callStaticMethod<jboolean>(nfcClassName,"isEnabled");
}

bool isSupported()
{
    return QJniObject::callStaticMethod<jboolean>(nfcClassName,"isSupported");
}

bool stopDiscovery()
{
    return QJniObject::callStaticMethod<jboolean>(nfcClassName,"stop");
}

QJniObject getStartIntent()
{
    QJniObject ret = QJniObject::callStaticObjectMethod(nfcClassName, "getStartIntent", "()Landroid/content/Intent;");
    return ret;
}

bool registerListener(AndroidNfcListenerInterface *listener)
{
    return mainListener.registerListener(listener);
}

bool unregisterListener(AndroidNfcListenerInterface *listener)
{
    return mainListener.unregisterListener(listener);
}

QJniObject getTag(const QJniObject &intent)
{
    QJniObject extraTag = QJniObject::getStaticObjectField("android/nfc/NfcAdapter", "EXTRA_TAG", "Ljava/lang/String;");
    QJniObject tag = intent.callObjectMethod("getParcelableExtra", "(Ljava/lang/String;)Landroid/os/Parcelable;", extraTag.object<jstring>());
    return tag;
}

QT_END_ANDROIDNFC_NAMESPACE

Q_DECL_EXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void * /*reserved*/)
{
    static bool initialized = false;
    if (initialized)
        return JNI_VERSION_1_6;
    initialized = true;

    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    return JNI_VERSION_1_6;
}

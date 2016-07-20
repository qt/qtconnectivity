/****************************************************************************
**
** Copyright (C) 2013 Centria research and development
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "androidjninfc_p.h"

#include <android/log.h>

#include "androidmainnewintentlistener_p.h"

#include "qglobal.h"
#include "qbytearray.h"
#include "qdebug.h"

static const char *nfcClassName = "org/qtproject/qt5/android/nfc/QtNfc";

static AndroidNfc::MainNfcNewIntentListener mainListener;

QT_BEGIN_ANDROIDNFC_NAMESPACE

bool startDiscovery()
{
    return QAndroidJniObject::callStaticMethod<jboolean>(nfcClassName,"start");
}

bool isAvailable()
{
    return QAndroidJniObject::callStaticMethod<jboolean>(nfcClassName,"isAvailable");
}

bool stopDiscovery()
{
    return QAndroidJniObject::callStaticMethod<jboolean>(nfcClassName,"stop");
}

QAndroidJniObject getStartIntent()
{
    QAndroidJniObject ret = QAndroidJniObject::callStaticObjectMethod(nfcClassName, "getStartIntent", "()Landroid/content/Intent;");
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

QAndroidJniObject getTag(const QAndroidJniObject &intent)
{
    QAndroidJniObject extraTag = QAndroidJniObject::getStaticObjectField("android/nfc/NfcAdapter", "EXTRA_TAG", "Ljava/lang/String;");
    QAndroidJniObject tag = intent.callObjectMethod("getParcelableExtra", "(Ljava/lang/String;)Landroid/os/Parcelable;", extraTag.object<jstring>());
    Q_ASSERT_X(tag.isValid(), "getTag", "could not get Tag object");
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

// Copyright (C) 2016 Centria research and development
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "androidjninfc_p.h"

QT_BEGIN_NAMESPACE

namespace QtNfc {

bool startDiscovery()
{
    return QJniObject::callStaticMethod<jboolean>(QtJniTypes::className<QtJniTypes::QtNfc>(),
                                                  "startDiscovery");
}

bool isEnabled()
{
    return QJniObject::callStaticMethod<jboolean>(QtJniTypes::className<QtJniTypes::QtNfc>(),
                                                  "isEnabled");
}

bool isSupported()
{
    return QJniObject::callStaticMethod<jboolean>(QtJniTypes::className<QtJniTypes::QtNfc>(),
                                                  "isSupported");
}

bool stopDiscovery()
{
    return QJniObject::callStaticMethod<jboolean>(QtJniTypes::className<QtJniTypes::QtNfc>(),
                                                  "stopDiscovery");
}

QJniObject getStartIntent()
{
    return QJniObject::callStaticMethod<QtJniTypes::Intent>(
            QtJniTypes::className<QtJniTypes::QtNfc>(), "getStartIntent");
}

QJniObject getTag(const QJniObject &intent)
{
    return QJniObject::callStaticMethod<QtJniTypes::Parcellable>(
            QtJniTypes::className<QtJniTypes::QtNfc>(), "getTag",
            intent.object<QtJniTypes::Intent>());
}

} // namespace QtNfc

QT_END_NAMESPACE

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

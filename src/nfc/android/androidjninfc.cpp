/****************************************************************************
**
** Copyright (C) 2013 Centria research and development
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtNfc module of the Qt Toolkit.
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

#include "androidjninfc_p.h"

#include <android/log.h>

#include "qglobal.h"
#include "qlist.h"
#include "qreadwritelock.h"
#include "qbytearray.h"
#include "qdebug.h"

static JavaVM *javaVM = 0;
static jclass nfcClass;

static jmethodID startDiscoveryId;
static jmethodID stopDiscoveryId;
static jmethodID isAvailableId;

static QList<AndroidNfc::AndroidNfcListenerInterface*> listeners;
QReadWriteLock listenersLock;

QT_BEGIN_ANDROIDNFC_NAMESPACE

AttachedJNIEnv::AttachedJNIEnv()
{
    Q_ASSERT_X(javaVM != 0, "constructor", "javaVM pointer is null");
    attached = false;
    if (javaVM->GetEnv((void**)&jniEnv, JNI_VERSION_1_6) < 0) {
        if (javaVM->AttachCurrentThread(&jniEnv, NULL) < 0) {
            __android_log_print(ANDROID_LOG_ERROR, "Qt", "AttachCurrentThread failed");
            jniEnv = 0;
            return;
        }
        attached = true;
    }
}

AttachedJNIEnv::~AttachedJNIEnv()
{
    if (attached)
        javaVM->DetachCurrentThread();
}

bool startDiscovery()
{
    //__android_log_print(ANDROID_LOG_INFO, "Qt", "QtNfc::startDiscovery()");
    AttachedJNIEnv aenv;
    if (!aenv.jniEnv)
        return false;

    aenv.jniEnv->CallStaticObjectMethod(nfcClass, startDiscoveryId);
    return true;
}

bool isAvailable()
{
    //__android_log_print(ANDROID_LOG_INFO, "Qt", "QtNfc::isAvailable()");
    AttachedJNIEnv aenv;
    if (!aenv.jniEnv)
        return false;

    return aenv.jniEnv->CallStaticBooleanMethod(nfcClass, isAvailableId);
}

bool stopDiscovery()
{
    AttachedJNIEnv aenv;
    if (!aenv.jniEnv)
        return false;

    aenv.jniEnv->CallStaticObjectMethod(nfcClass, stopDiscoveryId);
    return true;
}

bool registerListener(AndroidNfcListenerInterface *listener)
{
    listenersLock.lockForWrite();
    listeners.push_back(listener);
    listenersLock.unlock();
    return true;
}

bool unregisterListener(AndroidNfcListenerInterface *listener)
{
    listenersLock.lockForWrite();
    listeners.removeOne(listener);
    listenersLock.unlock();
    return true;
}

jobject getTag(JNIEnv *env, jobject intent)
{
    jclass intentClass = env->GetObjectClass(intent);
    Q_ASSERT_X(intentClass != 0, "getTag", "could not get Intent class");

    jmethodID getParcelableExtraMID = env->GetMethodID(intentClass,
                                                       "getParcelableExtra",
                                                       "(Ljava/lang/String;)Landroid/os/Parcelable;");
    Q_ASSERT_X(getParcelableExtraMID != 0, "getTag", "could not get method ID for getParcelableExtra()");

    jclass nfcAdapterClass = env->FindClass("android/nfc/NfcAdapter");
    Q_ASSERT_X(nfcAdapterClass != 0, "getTag", "could not find NfcAdapter class");

    jfieldID extraTagFID = env->GetStaticFieldID(nfcAdapterClass, "EXTRA_TAG", "Ljava/lang/String;");
    Q_ASSERT_X(extraTagFID != 0, "getTag", "could not get field ID for EXTRA_TAG");

    jobject extraTag = env->GetStaticObjectField(nfcAdapterClass, extraTagFID);
    Q_ASSERT_X(extraTag != 0, "getTag", "could not get EXTRA_TAG");

    jobject tag = env->CallObjectMethod(intent, getParcelableExtraMID, extraTag);
    Q_ASSERT_X(tag != 0, "getTag", "could not get Tag object");

    return tag;
}

QT_END_ANDROIDNFC_NAMESPACE

static const char logTag[] = "Qt";
static const char classErrorMsg[] = "Can't find class \"%s\"";
static const char methodErrorMsg[] = "Can't find method \"%s%s\"";

static void newIntent(JNIEnv *env, jobject obj, jobject intent)
{
    Q_UNUSED(obj);
    listenersLock.lockForRead();
    foreach (AndroidNfc::AndroidNfcListenerInterface *listener, listeners) {
        // Making new global reference for each listener.
        // Listeners must release reference when it is not used anymore.
        jobject newIntentRef = env->NewGlobalRef(intent);
        listener->newIntent(newIntentRef);
    }
    listenersLock.unlock();
}

static JNINativeMethod methods[] = {
    {"newIntent", "(Landroid/content/Intent;)V", (void*)newIntent}
};

#define FIND_AND_CHECK_CLASS(CLASS_NAME) \
    clazz = env->FindClass(CLASS_NAME); \
    if (!clazz) { \
    return JNI_FALSE; \
    }

#define GET_AND_CHECK_STATIC_METHOD(VAR, CLASS, METHOD_NAME, METHOD_SIGNATURE) \
    VAR = env->GetStaticMethodID(CLASS, METHOD_NAME, METHOD_SIGNATURE); \
    if (!VAR) { \
    return JNI_FALSE; \
    }

Q_DECL_EXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void * /*reserved*/)
{
    //__android_log_print(ANDROID_LOG_INFO, "Qt", "JNI_OnLoad for QtNfc");
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    jclass clazz;
    FIND_AND_CHECK_CLASS("org/qtproject/qt5/android/nfc/QtNfc");
    nfcClass = static_cast<jclass>(env->NewGlobalRef(clazz));

    if (env->RegisterNatives(nfcClass, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        __android_log_print(ANDROID_LOG_FATAL, logTag, "RegisterNatives failed");
        return JNI_FALSE;
    }

    GET_AND_CHECK_STATIC_METHOD(startDiscoveryId, nfcClass, "start", "()V");
    GET_AND_CHECK_STATIC_METHOD(stopDiscoveryId, nfcClass, "stop", "()V");
    GET_AND_CHECK_STATIC_METHOD(isAvailableId, nfcClass, "isAvailable", "()Z");
    javaVM = vm;

    return JNI_VERSION_1_6;
}

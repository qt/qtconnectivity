/****************************************************************************
**
** Copyright (C) 2013 Lauri Laanmets (Proekspert AS) <lauri.laanmets@eesti.ee>
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

#include <jni.h>
#include <android/log.h>
#include <QtCore/QLoggingCategory>
#include <QtBluetooth/qbluetoothglobal.h>
#include <QtAndroidExtras/QAndroidJniObject>
#include "android/androidbroadcastreceiver_p.h"
#include "android/serveracceptancethread_p.h"
#include "android/inputstreamthread_p.h"

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

void QtBroadcastReceiver_jniOnReceive(JNIEnv *env, jobject /*javaObject*/,
                                             jlong qtObject, jobject context, jobject intent)
{
    reinterpret_cast<AndroidBroadcastReceiver*>(qtObject)->onReceive(env, context, intent);
}

static void QtBluetoothSocketServer_errorOccurred(JNIEnv */*env*/, jobject /*javaObject*/,
                                           jlong qtObject, jint errorCode)
{
    reinterpret_cast<ServerAcceptanceThread*>(qtObject)->javaThreadErrorOccurred(errorCode);
}

static void QtBluetoothSocketServer_newSocket(JNIEnv */*env*/, jobject /*javaObject*/,
                                       jlong qtObject, jobject socket)
{
    reinterpret_cast<ServerAcceptanceThread*>(qtObject)->javaNewSocket(socket);
}

static void QtBluetoothInputStreamThread_errorOccurred(JNIEnv */*env*/, jobject /*javaObject*/,
                                           jlong qtObject, jint errorCode)
{
    reinterpret_cast<InputStreamThread*>(qtObject)->javaThreadErrorOccurred(errorCode);
}

static void QtBluetoothInputStreamThread_readyData(JNIEnv */*env*/, jobject /*javaObject*/,
                                       jlong qtObject, jbyteArray buffer, jint bufferLength)
{
    reinterpret_cast<InputStreamThread*>(qtObject)->javaReadyRead(buffer, bufferLength);
}


static JNINativeMethod methods[] = {
    {"jniOnReceive", "(JLandroid/content/Context;Landroid/content/Intent;)V",
                (void *) QtBroadcastReceiver_jniOnReceive},
};

static JNINativeMethod methods_server[] = {
        {"errorOccurred", "(JI)V",
                    (void *) QtBluetoothSocketServer_errorOccurred},
        {"newSocket", "(JLandroid/bluetooth/BluetoothSocket;)V",
                    (void *) QtBluetoothSocketServer_newSocket},
};

static JNINativeMethod methods_inputStream[] = {
        {"errorOccurred", "(JI)V",
                    (void *) QtBluetoothInputStreamThread_errorOccurred},
        {"readyData", "(J[BI)V",
                    (void *) QtBluetoothInputStreamThread_readyData},
};

static const char logTag[] = "QtBluetooth";
static const char classErrorMsg[] = "Can't find class \"%s\"";

#define FIND_AND_CHECK_CLASS(CLASS_NAME) \
clazz = env->FindClass(CLASS_NAME); \
if (!clazz) { \
    __android_log_print(ANDROID_LOG_FATAL, logTag, classErrorMsg, CLASS_NAME); \
    return JNI_FALSE; \
}

static bool registerNatives(JNIEnv *env)
{
    jclass clazz;
    FIND_AND_CHECK_CLASS("org/qtproject/qt5/android/bluetooth/QtBluetoothBroadcastReceiver");


    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        __android_log_print(ANDROID_LOG_FATAL, logTag, "RegisterNatives for BraodcastReceiver failed");
        return false;
    }

    FIND_AND_CHECK_CLASS("org/qtproject/qt5/android/bluetooth/QtBluetoothSocketServer");
    if (env->RegisterNatives(clazz, methods_server, sizeof(methods_server) / sizeof(methods_server[0])) < 0) {
        __android_log_print(ANDROID_LOG_FATAL, logTag, "RegisterNatives for SocketServer failed");
        return false;
    }

    FIND_AND_CHECK_CLASS("org/qtproject/qt5/android/bluetooth/QtBluetoothInputStreamThread");
    if (env->RegisterNatives(clazz, methods_inputStream,
                             sizeof(methods_inputStream) / sizeof(methods_inputStream[0])) < 0) {
        __android_log_print(ANDROID_LOG_FATAL, logTag, "RegisterNatives for InputStreamThread failed");
        return false;
    }

    return true;
}

Q_BLUETOOTH_EXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
{
    typedef union {
        JNIEnv *nativeEnvironment;
        void *venv;
    } UnionJNIEnvToVoid;

    UnionJNIEnvToVoid uenv;
    uenv.venv = 0;

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        __android_log_print(ANDROID_LOG_FATAL, logTag, "GetEnv failed");
        return -1;
    }

    JNIEnv *env = uenv.nativeEnvironment;
    if (!registerNatives(env)) {
        __android_log_print(ANDROID_LOG_FATAL, logTag, "registerNatives failed");
        return -1;
    }

    if (QT_BT_ANDROID().isDebugEnabled())
        __android_log_print(ANDROID_LOG_INFO, logTag, "Bluetooth start");

    return JNI_VERSION_1_4;
}

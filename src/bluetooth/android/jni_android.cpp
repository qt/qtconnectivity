// Copyright (C) 2016 Lauri Laanmets (Proekspert AS) <lauri.laanmets@eesti.ee>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <jni.h>
#include <android/log.h>
#include <QtCore/QLoggingCategory>
#include <QtBluetooth/qtbluetoothglobal.h>
#include "android/jni_android_p.h"
#include "android/androidbroadcastreceiver_p.h"
#include "android/serveracceptancethread_p.h"
#include "android/inputstreamthread_p.h"
#include "android/lowenergynotificationhub_p.h"

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

typedef QHash<QByteArray, QJniObject> JCachedStringFields;
Q_GLOBAL_STATIC(JCachedStringFields, cachedStringFields)

//Java field names
static const char * const javaActionAclConnected = "ACTION_ACL_CONNECTED";
static const char * const javaActionAclDisconnected = "ACTION_ACL_DISCONNECTED";
static const char * const javaActionBondStateChanged = "ACTION_BOND_STATE_CHANGED";
static const char * const javaActionDiscoveryStarted = "ACTION_DISCOVERY_STARTED";
static const char * const javaActionDiscoveryFinished = "ACTION_DISCOVERY_FINISHED";
static const char * const javaActionFound = "ACTION_FOUND";
static const char * const javaActionScanModeChanged = "ACTION_SCAN_MODE_CHANGED";
static const char * const javaActionUuid = "ACTION_UUID";
static const char * const javaExtraBondState = "EXTRA_BOND_STATE";
static const char * const javaExtraDevice = "EXTRA_DEVICE";
static const char * const javaExtraPairingKey = "EXTRA_PAIRING_KEY";
static const char * const javaExtraPairingVariant = "EXTRA_PAIRING_VARIANT";
static const char * const javaExtraRssi = "EXTRA_RSSI";
static const char * const javaExtraScanMode = "EXTRA_SCAN_MODE";
static const char * const javaExtraUuid = "EXTRA_UUID";

/*
 * This function operates on the assumption that each
 * field is of type java/lang/String.
 */
inline QByteArray classNameForStaticField(JavaNames javaName)
{
    switch (javaName) {
    case JavaNames::BluetoothAdapter: {
        constexpr auto cn = QtJniTypes::className<QtJniTypes::BluetoothAdapter>();
        return QByteArray(cn.data(), cn.size());
    }
    case JavaNames::BluetoothDevice: {
        constexpr auto cn = QtJniTypes::className<QtJniTypes::BluetoothDevice>();
        return QByteArray(cn.data(), cn.size());
    }
    default:
        break;
    }
    return {};
}

QJniObject valueForStaticField(JavaNames javaName, JavaNames javaFieldName)
{
    //construct key
    //the switch statements are used to reduce the number of duplicated strings
    //in the library

    const char *fieldName;
    switch (javaFieldName) {
    case JavaNames::ActionAclConnected:
        fieldName = javaActionAclConnected; break;
    case JavaNames::ActionAclDisconnected:
        fieldName = javaActionAclDisconnected; break;
    case JavaNames::ActionBondStateChanged:
        fieldName = javaActionBondStateChanged; break;
    case JavaNames::ActionDiscoveryStarted:
        fieldName = javaActionDiscoveryStarted; break;
    case JavaNames::ActionDiscoveryFinished:
        fieldName = javaActionDiscoveryFinished; break;
    case JavaNames::ActionFound:
        fieldName = javaActionFound; break;
    case JavaNames::ActionScanModeChanged:
        fieldName = javaActionScanModeChanged; break;
    case JavaNames::ActionUuid:
        fieldName = javaActionUuid; break;
    case JavaNames::ExtraBondState:
        fieldName = javaExtraBondState; break;
    case JavaNames::ExtraDevice:
        fieldName = javaExtraDevice; break;
    case JavaNames::ExtraPairingKey:
        fieldName = javaExtraPairingKey; break;
    case JavaNames::ExtraPairingVariant:
        fieldName = javaExtraPairingVariant; break;
    case JavaNames::ExtraRssi:
        fieldName = javaExtraRssi; break;
    case JavaNames::ExtraScanMode:
        fieldName = javaExtraScanMode; break;
    case JavaNames::ExtraUuid:
        fieldName = javaExtraUuid; break;
    default:
        qCWarning(QT_BT_ANDROID) << "Unknown java field name passed to valueForStaticField():" << javaFieldName;
        return QJniObject();
    }

    const QByteArray className = classNameForStaticField(javaName);
    if (className.isEmpty()) {
        qCWarning(QT_BT_ANDROID) << "Unknown java class name passed to valueForStaticField():" << javaName;
        return QJniObject();
    }

    const QByteArray key = className + fieldName;
    JCachedStringFields::iterator it = cachedStringFields()->find(key);
    if (it == cachedStringFields()->end()) {
        QJniEnvironment env;
        QJniObject fieldValue = QJniObject::getStaticObjectField(
                                            className, fieldName, "Ljava/lang/String;");
        if (!fieldValue.isValid()) {
            cachedStringFields()->insert(key, QJniObject());
            return QJniObject();
        }

        cachedStringFields()->insert(key, fieldValue);
        return fieldValue;
    } else {
        return it.value();
    }
}

void QtBroadcastReceiver_jniOnReceive(JNIEnv *env, jobject /*javaObject*/,
                                      jlong qtObject, QtJniTypes::Context context,
                                      QtJniTypes::Intent intent)
{
    reinterpret_cast<AndroidBroadcastReceiver*>(qtObject)->onReceive(env, context, intent);
}
Q_DECLARE_JNI_NATIVE_METHOD(QtBroadcastReceiver_jniOnReceive, jniOnReceive)

static void QtBluetoothSocketServer_errorOccurred(JNIEnv */*env*/, jobject /*javaObject*/,
                                           jlong qtObject, jint errorCode)
{
    reinterpret_cast<ServerAcceptanceThread*>(qtObject)->javaThreadErrorOccurred(errorCode);
}
Q_DECLARE_JNI_NATIVE_METHOD(QtBluetoothSocketServer_errorOccurred, errorOccurred)

static void QtBluetoothSocketServer_newSocket(JNIEnv */*env*/, jobject /*javaObject*/,
                                              jlong qtObject, QtJniTypes::BluetoothSocket socket)
{
    reinterpret_cast<ServerAcceptanceThread*>(qtObject)->javaNewSocket(socket);
}
Q_DECLARE_JNI_NATIVE_METHOD(QtBluetoothSocketServer_newSocket, newSocket)

static void QtBluetoothInputStreamThread_errorOccurred(JNIEnv */*env*/, jobject /*javaObject*/,
                                           jlong qtObject, jint errorCode)
{
    reinterpret_cast<InputStreamThread*>(qtObject)->javaThreadErrorOccurred(errorCode);
}
Q_DECLARE_JNI_NATIVE_METHOD(QtBluetoothInputStreamThread_errorOccurred, errorOccurred)

static void QtBluetoothInputStreamThread_readyData(JNIEnv */*env*/, jobject /*javaObject*/,
                                       jlong qtObject, jbyteArray buffer, jint bufferLength)
{
    reinterpret_cast<InputStreamThread*>(qtObject)->javaReadyRead(buffer, bufferLength);
}
Q_DECLARE_JNI_NATIVE_METHOD(QtBluetoothInputStreamThread_readyData, readyData)

void QtBluetoothLE_leScanResult(JNIEnv *env, jobject, jlong qtObject,
                                QtJniTypes::BluetoothDevice bluetoothDevice, jint rssi,
                                jbyteArray scanRecord)
{
    if (Q_UNLIKELY(qtObject == 0))
        return;

    reinterpret_cast<AndroidBroadcastReceiver*>(qtObject)->onReceiveLeScan(
                                                                env, bluetoothDevice, rssi,
                                                                scanRecord);
}
Q_DECLARE_JNI_NATIVE_METHOD(QtBluetoothLE_leScanResult, leScanResult)

static const char logTag[] = "QtBluetooth";
static const char classErrorMsg[] = "Can't find class \"%s\"";

#define FIND_AND_CHECK_CLASS(CLASS_NAME)                             \
clazz = env.findClass<CLASS_NAME>();                                 \
if (!clazz) {                                                        \
    __android_log_print(ANDROID_LOG_FATAL, logTag, classErrorMsg,    \
                        QtJniTypes::className<CLASS_NAME>().data()); \
    return JNI_FALSE;                                                \
}                                                                    \

#define LEHUB_SCOPED_METHOD(Method) Q_JNI_NATIVE_SCOPED_METHOD(Method, LowEnergyNotificationHub)

static bool registerNatives()
{
    jclass clazz;
    QJniEnvironment env;

    FIND_AND_CHECK_CLASS(QtJniTypes::QtBtBroadcastReceiver);
    if (!env.registerNativeMethods(clazz,
                                  {
                                      Q_JNI_NATIVE_METHOD(QtBroadcastReceiver_jniOnReceive)
                                  }))
    {
        __android_log_print(ANDROID_LOG_FATAL, logTag,
                            "registerNativeMethods for BroadcastReceiver failed");
        return false;
    }

    FIND_AND_CHECK_CLASS(QtJniTypes::QtBtLECentral);
    if (!env.registerNativeMethods(clazz,
                              {
                                   Q_JNI_NATIVE_METHOD(QtBluetoothLE_leScanResult),
                                   LEHUB_SCOPED_METHOD(lowEnergy_connectionChange),
                                   LEHUB_SCOPED_METHOD(lowEnergy_mtuChanged),
                                   LEHUB_SCOPED_METHOD(lowEnergy_servicesDiscovered),
                                   LEHUB_SCOPED_METHOD(lowEnergy_serviceDetailsDiscovered),
                                   LEHUB_SCOPED_METHOD(lowEnergy_characteristicRead),
                                   LEHUB_SCOPED_METHOD(lowEnergy_descriptorRead),
                                   LEHUB_SCOPED_METHOD(lowEnergy_characteristicWritten),
                                   LEHUB_SCOPED_METHOD(lowEnergy_descriptorWritten),
                                   LEHUB_SCOPED_METHOD(lowEnergy_characteristicChanged),
                                   LEHUB_SCOPED_METHOD(lowEnergy_serviceError)
                               }))
    {
        __android_log_print(ANDROID_LOG_FATAL, logTag,
                            "registerNativeMethods for QBLuetoothLE failed");
        return false;
    }

    FIND_AND_CHECK_CLASS(QtJniTypes::QtBtLEServer);
    if (!env.registerNativeMethods(clazz,
                            {
                                 LEHUB_SCOPED_METHOD(lowEnergy_connectionChange),
                                 LEHUB_SCOPED_METHOD(lowEnergy_mtuChanged),
                                 LEHUB_SCOPED_METHOD(lowEnergy_advertisementError),
                                 LEHUB_SCOPED_METHOD(lowEnergy_serverCharacteristicChanged),
                                 LEHUB_SCOPED_METHOD(lowEnergy_serverDescriptorWritten)
                             }))
    {
        __android_log_print(ANDROID_LOG_FATAL, logTag,
                            "registerNativeMethods for QBLuetoothLEServer failed");
        return false;
    }

    FIND_AND_CHECK_CLASS(QtJniTypes::QtBtSocketServer);
    if (!env.registerNativeMethods(clazz,
                                  {
                                      Q_JNI_NATIVE_METHOD(QtBluetoothSocketServer_errorOccurred),
                                      Q_JNI_NATIVE_METHOD(QtBluetoothSocketServer_newSocket)
                                  }))
    {
        __android_log_print(ANDROID_LOG_FATAL, logTag,
                            "registerNativeMethods for SocketServer failed");
        return false;
    }

    FIND_AND_CHECK_CLASS(QtJniTypes::QtBtInputStreamThread);
    if (!env.registerNativeMethods(clazz,
                               {
                                   Q_JNI_NATIVE_METHOD(QtBluetoothInputStreamThread_errorOccurred),
                                   Q_JNI_NATIVE_METHOD(QtBluetoothInputStreamThread_readyData)
                               }))
    {
        __android_log_print(ANDROID_LOG_FATAL, logTag,
                            "registerNativeMethods for InputStreamThread failed");
        return false;
    }

    return true;
}

Q_BLUETOOTH_EXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
{
    static bool initialized = false;
    if (initialized)
        return JNI_VERSION_1_6;
    initialized = true;

    typedef union {
        JNIEnv *nativeEnvironment;
        void *venv;
    } UnionJNIEnvToVoid;

    UnionJNIEnvToVoid uenv;
    uenv.venv = 0;

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_6) != JNI_OK) {
        __android_log_print(ANDROID_LOG_FATAL, logTag, "GetEnv failed");
        return -1;
    }

    if (!registerNatives()) {
        __android_log_print(ANDROID_LOG_FATAL, logTag, "registerNatives failed");
        return -1;
    }

    if (QT_BT_ANDROID().isDebugEnabled())
        __android_log_print(ANDROID_LOG_INFO, logTag, "Bluetooth start");

    return JNI_VERSION_1_6;
}

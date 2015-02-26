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

#include "qnearfieldtarget_android_p.h"
#include "android/androidjninfc_p.h"
#include "qdebug.h"

const QString NearFieldTarget::NdefTechology = QString::fromUtf8("android.nfc.tech.Ndef");
const QString NearFieldTarget::NdefFormatableTechnology = QString::fromUtf8("android.nfc.tech.NdefFormatable");
const QString NearFieldTarget::NfcATechnology = QString::fromUtf8("android.nfc.tech.NfcA");
const QString NearFieldTarget::NfcBTechnology = QString::fromUtf8("android.nfc.tech.NfcB");
const QString NearFieldTarget::NfcFTechnology = QString::fromUtf8("android.nfc.tech.NfcF");
const QString NearFieldTarget::NfcVTechnology = QString::fromUtf8("android.nfc.tech.NfcV");
const QString NearFieldTarget::MifareClassicTechnology = QString::fromUtf8("android.nfc.tech.MifareClassic");
const QString NearFieldTarget::MifareUltralightTechnology = QString::fromUtf8("android.nfc.tech.MifareUltralight");


NearFieldTarget::NearFieldTarget(jobject intent, const QByteArray uid, QObject *parent) :
    QNearFieldTarget(parent),
    m_intent(intent),
    m_uid(uid)
{
    updateTechList();
    updateType();
    setupTargetCheckTimer();
}

NearFieldTarget::~NearFieldTarget()
{
    releaseIntent();
    emit targetDestroyed(m_uid);
}

QByteArray NearFieldTarget::uid() const
{
    return m_uid;
}

QNearFieldTarget::Type NearFieldTarget::type() const
{
    return m_type;
}

QNearFieldTarget::AccessMethods NearFieldTarget::accessMethods() const
{
    AccessMethods result = NdefAccess;
    return result;
}

bool NearFieldTarget::hasNdefMessage()
{
    return m_techList.contains(NdefTechology);
}

QNearFieldTarget::RequestId NearFieldTarget::readNdefMessages()
{
    // Making sure that target has NDEF messages
    if (!hasNdefMessage())
        return QNearFieldTarget::RequestId();

    // Making sure that target is still in range
    QNearFieldTarget::RequestId requestId(new QNearFieldTarget::RequestIdPrivate);
    if (m_intent == 0) {
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                  Q_ARG(QNearFieldTarget::Error, QNearFieldTarget::TargetOutOfRangeError),
                                  Q_ARG(const QNearFieldTarget::RequestId&, requestId));
        return requestId;
    }

    // Getting Ndef technology object
    AndroidNfc::AttachedJNIEnv aenv;
    JNIEnv *env = aenv.jniEnv;
    Q_ASSERT_X(env != 0, "readNdefMessages", "env pointer is null");
    jobject ndef = getTagTechnology(NdefTechology, env);
    if (ndef == 0) {
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                  Q_ARG(QNearFieldTarget::Error, QNearFieldTarget::UnsupportedError),
                                  Q_ARG(const QNearFieldTarget::RequestId&, requestId));
        return requestId;
    }

    // Connect
    jclass ndefClass = env->GetObjectClass(ndef);
    jmethodID connectMID = env->GetMethodID(ndefClass, "connect", "()V");
    env->CallVoidMethod(ndef, connectMID);
    if (catchJavaExceptions(env)) {
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                  Q_ARG(QNearFieldTarget::Error, QNearFieldTarget::TargetOutOfRangeError),
                                  Q_ARG(const QNearFieldTarget::RequestId&, requestId));
        return requestId;
    }

    // Get NdefMessage object
    jmethodID getNdefMessageMID = env->GetMethodID(ndefClass, "getNdefMessage", "()Landroid/nfc/NdefMessage;");
    jobject ndefMessage = env->CallObjectMethod(ndef, getNdefMessageMID);
    if (catchJavaExceptions(env))
        ndefMessage = 0;
    if (ndefMessage == 0) {
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                  Q_ARG(QNearFieldTarget::Error, QNearFieldTarget::NdefReadError),
                                  Q_ARG(const QNearFieldTarget::RequestId&, requestId));
        return requestId;
    }

    // Convert to byte array
    jclass ndefMessageClass = env->GetObjectClass(ndefMessage);
    jmethodID toByteArrayMID = env->GetMethodID(ndefMessageClass, "toByteArray", "()[B");
    jbyteArray ndefMessageBA = reinterpret_cast<jbyteArray>(env->CallObjectMethod(ndefMessage, toByteArrayMID));
    QByteArray ndefMessageQBA = jbyteArrayToQByteArray(ndefMessageBA, env);

    // Closing connection
    jmethodID closeMID = env->GetMethodID(ndefClass, "close", "()V");
    env->CallVoidMethod(ndef, closeMID);
    catchJavaExceptions(env);   // IOException at this point does not matter anymore.

    // Sending QNdefMessage, requestCompleted and exit.
    QNdefMessage qNdefMessage = QNdefMessage::fromByteArray(ndefMessageQBA);
    QMetaObject::invokeMethod(this, "ndefMessageRead", Qt::QueuedConnection,
                              Q_ARG(const QNdefMessage&, qNdefMessage));
    QMetaObject::invokeMethod(this, "requestCompleted", Qt::QueuedConnection,
                              Q_ARG(const QNearFieldTarget::RequestId&, requestId));
    QMetaObject::invokeMethod(this, "ndefMessageRead", Qt::QueuedConnection,
                              Q_ARG(const QNdefMessage&, qNdefMessage),
                              Q_ARG(const QNearFieldTarget::RequestId&, requestId));
    return requestId;
}


QNearFieldTarget::RequestId NearFieldTarget::sendCommand(const QByteArray &command)
{
    Q_UNUSED(command);
    Q_EMIT QNearFieldTarget::error(QNearFieldTarget::UnsupportedError, QNearFieldTarget::RequestId());
    return QNearFieldTarget::RequestId();

    //Not supported for now
    /*if (command.size() == 0) {
        Q_EMIT QNearFieldTarget::error(QNearFieldTarget::InvalidParametersError, QNearFieldTarget::RequestId());
        return QNearFieldTarget::RequestId();
    }

    AndroidNfc::AttachedJNIEnv aenv;
    JNIEnv *env = aenv.jniEnv;

    jobject tagTech;
    if (m_techList.contains(NfcATechnology)) {
        tagTech = getTagTechnology(NfcATechnology);
    } else if (m_techList.contains(NfcBTechnology)) {
        tagTech = getTagTechnology(NfcBTechnology);
    } else if (m_techList.contains(NfcFTechnology)) {
        tagTech = getTagTechnology(NfcFTechnology);
    } else if (m_techList.contains(NfcVTechnology)) {
        tagTech = getTagTechnology(NfcVTechnology);
    } else {
        Q_EMIT QNearFieldTarget::error(QNearFieldTarget::UnsupportedError, QNearFieldTarget::RequestId());
        return QNearFieldTarget::RequestId();
    }

    QByteArray ba(ba);

    jclass techClass = env->GetObjectClass(tagTech);
    jmethodID tranceiveMID = env->GetMethodID(techClass, "tranceive", "([B)[B");
    Q_ASSERT_X(tranceiveMID != 0, "sendCommand", "could not find tranceive method");

    jbyteArray jba = env->NewByteArray(ba.size());
    env->SetByteArrayRegion(jba, 0, ba.size(), reinterpret_cast<jbyte*>(ba.data()));

    jbyteArray rsp = reinterpret_cast<jbyteArray>(env->CallObjectMethod(tagTech, tranceiveMID, jba));

    jsize len = env->GetArrayLength(rsp);
    QByteArray rspQBA;
    rspQBA.resize(len);

    env->GetByteArrayRegion(rsp, 0, len, reinterpret_cast<jbyte*>(rspQBA.data()));

    qDebug() << "Send command returned QBA size: " << rspQBA.size();


    env->DeleteLocalRef(jba);


    return QNearFieldTarget::RequestId();*/
}

QNearFieldTarget::RequestId NearFieldTarget::sendCommands(const QList<QByteArray> &commands)
{
    QNearFieldTarget::RequestId requestId;
    for (int i=0; i < commands.size(); i++){
        requestId = sendCommand(commands.at(i));
    }
    return requestId;
}

QNearFieldTarget::RequestId NearFieldTarget::writeNdefMessages(const QList<QNdefMessage> &messages)
{
    if (messages.size() == 0)
        return QNearFieldTarget::RequestId();

    if (messages.size() > 1)
        qWarning("QNearFieldTarget::writeNdefMessages: Android supports writing only one NDEF message per tag.");

    AndroidNfc::AttachedJNIEnv aenv;
    JNIEnv *env = aenv.jniEnv;
    jmethodID writeMethod;
    jobject tagTechnology;
    jclass tagClass;

    // Getting write method
    if (m_techList.contains(NdefFormatableTechnology)) {
        tagTechnology = getTagTechnology(NdefFormatableTechnology, env);
        tagClass = env->GetObjectClass(tagTechnology);
        writeMethod = env->GetMethodID(tagClass, "format", "(Landroid/nfc/NdefMessage;)V");
    } else if (m_techList.contains(NdefTechology)) {
        tagTechnology = getTagTechnology(NdefTechology, env);
        tagClass = env->GetObjectClass(tagTechnology);
        writeMethod = env->GetMethodID(tagClass, "writeNdefMessage", "(Landroid/nfc/NdefMessage;)V");
    } else {
        // An invalid request id will be returned if the target does not support writing NDEF messages.
        return QNearFieldTarget::RequestId();
    }

    // Connecting
    QNearFieldTarget::RequestId requestId = QNearFieldTarget::RequestId(new QNearFieldTarget::RequestIdPrivate());
    jclass ndefMessageClass = env->FindClass("android/nfc/NdefMessage");
    jmethodID connectMethodID = env->GetMethodID(tagClass, "connect", "()V");
    env->CallVoidMethod(tagTechnology, connectMethodID);
    if (catchJavaExceptions(env)) {
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                  Q_ARG(QNearFieldTarget::Error, QNearFieldTarget::TargetOutOfRangeError),
                                  Q_ARG(const QNearFieldTarget::RequestId&, requestId));
        return requestId;
    }

    // Making NdefMessage object
    jmethodID ndefMessageInitMID = env->GetMethodID(ndefMessageClass, "<init>", "([B)V");
    const QNdefMessage &message = messages.first();
    QByteArray ba = message.toByteArray();
    jbyteArray jba = env->NewByteArray(ba.size());
    env->SetByteArrayRegion(jba, 0, ba.size(), reinterpret_cast<jbyte*>(ba.data()));
    jobject jmessage = env->NewObject(ndefMessageClass, ndefMessageInitMID, jba);
    if (catchJavaExceptions(env)) {
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                  Q_ARG(QNearFieldTarget::Error, QNearFieldTarget::UnknownError),
                                  Q_ARG(const QNearFieldTarget::RequestId&, requestId));
        return requestId;
    }

    // Writing
    env->CallVoidMethod(tagTechnology, writeMethod, jmessage);
    bool gotException = catchJavaExceptions(env);
    env->DeleteLocalRef(jba);
    env->DeleteLocalRef(jmessage);
    if (gotException) {
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                  Q_ARG(QNearFieldTarget::Error, QNearFieldTarget::NdefWriteError),
                                  Q_ARG(const QNearFieldTarget::RequestId&, requestId));
        return requestId;
    }

    // Closing connection, sending signal and exit
    jmethodID closeMID = env->GetMethodID(tagClass, "close", "()V");
    env->CallVoidMethod(tagTechnology, closeMID);
    catchJavaExceptions(env);   // IOException at this point does not matter anymore.
    QMetaObject::invokeMethod(this, "ndefMessagesWritten", Qt::QueuedConnection);
    return requestId;
}

void NearFieldTarget::setIntent(jobject intent)
{
    if (m_intent == intent)
        return;

    releaseIntent();
    m_intent = intent;
    if (m_intent) {
        // Updating tech list and type in case of there is another tag with same UID as one before.
        updateTechList();
        updateType();
        m_targetCheckTimer->start();
    }
}

void NearFieldTarget::checkIsTargetLost()
{
    if (m_intent == 0 || m_techList.isEmpty()) {
        handleTargetLost();
        return;
    }

    AndroidNfc::AttachedJNIEnv aenv;
    JNIEnv *env = aenv.jniEnv;

    // Using first available technology to check connection
    QString techStr = m_techList.first();
    jobject tagTechObj = getTagTechnology(techStr, env);
    jclass tagTechClass = env->GetObjectClass(tagTechObj);
    jmethodID connectMID = env->GetMethodID(tagTechClass, "connect","()V");
    jmethodID closeMID = env->GetMethodID(tagTechClass, "close","()V");

    env->CallObjectMethod(tagTechObj, connectMID);
    if (catchJavaExceptions(env)) {
        handleTargetLost();
        return;
    }
    env->CallObjectMethod(tagTechObj, closeMID);
    if (catchJavaExceptions(env))
        handleTargetLost();
}

void NearFieldTarget::releaseIntent()
{
    m_targetCheckTimer->stop();

    if (m_intent == 0)
        return;

    AndroidNfc::AttachedJNIEnv aenv;
    Q_ASSERT_X(aenv.jniEnv != 0, "releaseIntent", "aenv.jniEnv pointer is null");
    aenv.jniEnv->DeleteGlobalRef(m_intent);
    m_intent = 0;
}

void NearFieldTarget::updateTechList()
{
    if (m_intent == 0)
        return;

    // Getting tech list
    AndroidNfc::AttachedJNIEnv aenv;
    JNIEnv *env = aenv.jniEnv;
    jobject tag = AndroidNfc::getTag(env, m_intent);
    jclass tagClass = env->GetObjectClass(tag);
    jmethodID techListMID = env->GetMethodID(tagClass, "getTechList", "()[Ljava/lang/String;");
    jobjectArray techListArray = reinterpret_cast<jobjectArray>(env->CallObjectMethod(tag, techListMID));
    if (techListArray == 0) {
        handleTargetLost();
        return;
    }

    // Converting tech list array to QStringList.
    m_techList.clear();
    jsize techCount = env->GetArrayLength(techListArray);
    for (jsize i = 0; i < techCount; ++i) {
        jstring tech = reinterpret_cast<jstring>(env->GetObjectArrayElement(techListArray, i));
        const char *techStr = env->GetStringUTFChars(tech, JNI_FALSE);
        m_techList.append(QString::fromUtf8(techStr));
        env->ReleaseStringUTFChars(tech, techStr);
    }
}

void NearFieldTarget::updateType()
{
    m_type = getTagType();
}

QNearFieldTarget::Type NearFieldTarget::getTagType() const
{
    AndroidNfc::AttachedJNIEnv aenv;
    JNIEnv *env = aenv.jniEnv;
    Q_ASSERT_X(env != 0, "type", "env pointer is null");

    if (m_techList.contains(NdefTechology)) {
        jobject ndef = getTagTechnology(NdefTechology, env);
        jclass ndefClass = env->GetObjectClass(ndef);

        jmethodID typeMethodID = env->GetMethodID(ndefClass, "getType", "()Ljava/lang/String;");
        jstring jtype = reinterpret_cast<jstring>(env->CallObjectMethod(ndef, typeMethodID));
        const char *type_data = env->GetStringUTFChars(jtype, JNI_FALSE);
        QString qtype = QString::fromUtf8(type_data);
        env->ReleaseStringUTFChars(jtype, type_data);

        QHash<QString, Type> types;
        types.insert(QString::fromUtf8("com.nxp.ndef.mifareclassic"), MifareTag);
        types.insert(QString::fromUtf8("org.nfcforum.ndef.type1"), NfcTagType1);
        types.insert(QString::fromUtf8("org.nfcforum.ndef.type2"), NfcTagType2);
        types.insert(QString::fromUtf8("org.nfcforum.ndef.type3"), NfcTagType3);
        types.insert(QString::fromUtf8("org.nfcforum.ndef.type4"), NfcTagType4);
        if (!types.contains(qtype))
            return ProprietaryTag;
        return types[qtype];
    } else if (m_techList.contains(NfcATechnology)) {
        if (m_techList.contains(MifareClassicTechnology))
            return MifareTag;

        // Checking ATQA/SENS_RES
        // xxx0 0000  xxxx xxxx: Identifies tag Type 1 platform
        jobject nfca = getTagTechnology(NfcATechnology, env);
        jclass nfcaClass = env->GetObjectClass(nfca);
        jmethodID atqaMethodID = env->GetMethodID(nfcaClass, "getAtqa", "()[B");
        jbyteArray atqaBA = reinterpret_cast<jbyteArray>(env->CallObjectMethod(nfca, atqaMethodID));
        QByteArray atqaQBA = jbyteArrayToQByteArray(atqaBA, env);
        if (atqaQBA.isEmpty())
            return ProprietaryTag;
        if ((atqaQBA[0] & 0x1F) == 0x00)
            return NfcTagType1;

        // Checking SAK/SEL_RES
        // xxxx xxxx  x00x x0xx: Identifies tag Type 2 platform
        // xxxx xxxx  x01x x0xx: Identifies tag Type 4 platform
        jmethodID sakMethodID = env->GetMethodID(nfcaClass, "getSak", "()S");
        jshort sakS = env->CallShortMethod(nfca, sakMethodID);
        if ((sakS & 0x0064) == 0x0000)
            return NfcTagType2;
        else if ((sakS & 0x0064) == 0x0020)
            return NfcTagType4;
        return ProprietaryTag;
    } else if (m_techList.contains(NfcBTechnology)) {
        return NfcTagType4;
    } else if (m_techList.contains(NfcFTechnology)) {
        return NfcTagType3;
    }

    return ProprietaryTag;
}

void NearFieldTarget::setupTargetCheckTimer()
{
    m_targetCheckTimer = new QTimer(this);
    m_targetCheckTimer->setInterval(1000);
    connect(m_targetCheckTimer, SIGNAL(timeout()), this, SLOT(checkIsTargetLost()));
    m_targetCheckTimer->start();
}

void NearFieldTarget::handleTargetLost()
{
    releaseIntent();
    emit targetLost(this);
}

jobject NearFieldTarget::getTagTechnology(const QString &tech, JNIEnv *env) const
{
    QString techClass(tech);
    techClass.replace(QLatin1Char('.'), QLatin1Char('/'));

    // Getting requested technology
    jobject tag = AndroidNfc::getTag(env, m_intent);
    jclass tagClass = env->FindClass(techClass.toUtf8().constData());
    const QString sig = QString::fromUtf8("(Landroid/nfc/Tag;)L%1;");
    jmethodID getTagMethodID = env->GetStaticMethodID(tagClass, "get", sig.arg(techClass).toUtf8().constData());
    jobject tagtech = env->CallStaticObjectMethod(tagClass, getTagMethodID, tag);
    return tagtech;
}

QByteArray NearFieldTarget::jbyteArrayToQByteArray(const jbyteArray &byteArray, JNIEnv *env) const
{
    QByteArray resultArray;
    jsize len = env->GetArrayLength(byteArray);
    resultArray.resize(len);
    env->GetByteArrayRegion(byteArray, 0, len, reinterpret_cast<jbyte*>(resultArray.data()));
    return resultArray;
}

bool NearFieldTarget::catchJavaExceptions(JNIEnv *env) const
{
    jthrowable exc = env->ExceptionOccurred();
    if (exc) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return true;
    }
    return false;
}

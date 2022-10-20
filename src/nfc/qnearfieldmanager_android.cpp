/****************************************************************************
**
** Copyright (C) 2016 Centria research and development
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qnearfieldmanager_android_p.h"

#include "qndeffilter.h"
#include "qndefmessage.h"
#include "qndefrecord.h"
#include "qbytearray.h"
#include "qcoreapplication.h"
#include "qdebug.h"
#include "qlist.h"

#include <QCoreApplication>
#include <QScopedPointer>
#include <QtCore/QMetaType>
#include <QtCore/QMetaMethod>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QJniObject, broadcastReceiver)
Q_GLOBAL_STATIC(QList<QNearFieldManagerPrivateImpl *>, broadcastListener)

extern "C"
{
    JNIEXPORT void JNICALL Java_org_qtproject_qt_android_nfc_QtNfcBroadcastReceiver_jniOnReceive(
        JNIEnv */*env*/, jobject /*javaObject*/, jint state)
    {
        QNearFieldManager::AdapterState adapterState = static_cast<QNearFieldManager::AdapterState>((int) state);

        for (const auto listener : qAsConst(*broadcastListener)) {
            Q_EMIT listener->adapterStateChanged(adapterState);
        }
    }
}

QNearFieldManagerPrivateImpl::QNearFieldManagerPrivateImpl() :
    detecting(false)
{
    qRegisterMetaType<QJniObject>("QJniObject");
    qRegisterMetaType<QNdefMessage>("QNdefMessage");

    if (!broadcastReceiver->isValid()) {
        *broadcastReceiver = QJniObject("org/qtproject/qt/android/nfc/QtNfcBroadcastReceiver",
                                        "(Landroid/content/Context;)V", QNativeInterface::QAndroidApplication::context());
    }
    broadcastListener->append(this);
}

QNearFieldManagerPrivateImpl::~QNearFieldManagerPrivateImpl()
{
    broadcastListener->removeOne(this);
    if (broadcastListener->isEmpty()) {
        broadcastReceiver->callMethod<void>("unregisterReceiver");
        *broadcastReceiver = QJniObject();
    }
}

void QNearFieldManagerPrivateImpl::onTargetDetected(QNearFieldTargetPrivateImpl *target)
{
    if (target->q_ptr) {
        Q_EMIT targetDetected(target->q_ptr);
        return;
    }

    Q_EMIT targetDetected(new NearFieldTarget(target, this));
}

void QNearFieldManagerPrivateImpl::onTargetLost(QNearFieldTargetPrivateImpl *target)
{
    Q_EMIT targetLost(target->q_ptr);
}

bool QNearFieldManagerPrivateImpl::isEnabled() const
{
    return AndroidNfc::isEnabled();
}

bool QNearFieldManagerPrivateImpl::isSupported(QNearFieldTarget::AccessMethod accessMethod) const
{
    if (accessMethod == QNearFieldTarget::UnknownAccess)
        return false;

    return AndroidNfc::isSupported();
}

bool QNearFieldManagerPrivateImpl::startTargetDetection(QNearFieldTarget::AccessMethod accessMethod)
{
    if (detecting)
        return false;   // Already detecting targets

    detecting = true;
    requestedMethod = accessMethod;
    updateReceiveState();
    return true;
}

void QNearFieldManagerPrivateImpl::stopTargetDetection(const QString &)
{
    detecting = false;
    updateReceiveState();
    Q_EMIT targetDetectionStopped();
}

void QNearFieldManagerPrivateImpl::newIntent(QJniObject intent)
{
    // This function is called from different thread and is used to move intent to main thread.
    QMetaObject::invokeMethod(this, [this, intent] {
        this->onTargetDiscovered(intent);
    }, Qt::QueuedConnection);
}

QByteArray QNearFieldManagerPrivateImpl::getUid(const QJniObject &intent)
{
    if (!intent.isValid())
        return QByteArray();

    QJniObject tag = AndroidNfc::getTag(intent);
    return getUidforTag(tag);
}

void QNearFieldManagerPrivateImpl::onTargetDiscovered(QJniObject intent)
{
    // Getting UID
    QByteArray uid = getUid(intent);

    // Accepting all targets but only sending signal of requested types.
    QNearFieldTargetPrivateImpl *&target = detectedTargets[uid];
    if (target) {
        target->setIntent(intent);  // Updating existing target
    } else {
        target = new QNearFieldTargetPrivateImpl(intent, uid);

        if (target->accessMethods() & requestedMethod) {
            connect(target, &QNearFieldTargetPrivateImpl::targetDestroyed, this, &QNearFieldManagerPrivateImpl::onTargetDestroyed);
            connect(target, &QNearFieldTargetPrivateImpl::targetLost, this, &QNearFieldManagerPrivateImpl::onTargetLost);
            onTargetDetected(target);
        } else {
            delete target;
            detectedTargets.remove(uid);
        }
    }
}

void QNearFieldManagerPrivateImpl::onTargetDestroyed(const QByteArray &uid)
{
    detectedTargets.remove(uid);
}

QByteArray QNearFieldManagerPrivateImpl::getUidforTag(const QJniObject &tag)
{
    if (!tag.isValid())
        return QByteArray();

    QJniEnvironment env;
    QJniObject tagId = tag.callObjectMethod("getId", "()[B");
    QByteArray uid;
    jsize len = env->GetArrayLength(tagId.object<jbyteArray>());
    uid.resize(len);
    env->GetByteArrayRegion(tagId.object<jbyteArray>(), 0, len, reinterpret_cast<jbyte*>(uid.data()));
    return uid;
}

void QNearFieldManagerPrivateImpl::updateReceiveState()
{
    if (detecting) {
        AndroidNfc::registerListener(this);
    } else {
        AndroidNfc::unregisterListener(this);
    }
}

QT_END_NAMESPACE

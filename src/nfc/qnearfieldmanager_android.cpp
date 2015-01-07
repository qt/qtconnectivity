/***************************************************************************
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

#include "qnearfieldmanager_android_p.h"
#include "qnearfieldtarget_android_p.h"

#include "qndeffilter.h"
#include "qndefmessage.h"
#include "qndefrecord.h"
#include "qbytearray.h"
#include "qcoreapplication.h"
#include "qdebug.h"
#include "qlist.h"

#include <QtCore/QMetaType>
#include <QtCore/QMetaMethod>

QT_BEGIN_NAMESPACE

QNearFieldManagerPrivateImpl::QNearFieldManagerPrivateImpl() :
    m_detecting(false)
{
    qRegisterMetaType<jobject>("jobject");
    qRegisterMetaType<QNdefMessage>("QNdefMessage");
}

QNearFieldManagerPrivateImpl::~QNearFieldManagerPrivateImpl()
{
}

bool QNearFieldManagerPrivateImpl::isAvailable() const
{
    return AndroidNfc::isAvailable();
}

bool QNearFieldManagerPrivateImpl::startTargetDetection()
{
    if (m_detecting)
        return false;   // Already detecting targets

    AndroidNfc::registerListener(this);
    AndroidNfc::startDiscovery();
    m_detecting = true;
    return true;
}

void QNearFieldManagerPrivateImpl::stopTargetDetection()
{
    AndroidNfc::stopDiscovery();
    AndroidNfc::unregisterListener(this);
    m_detecting = false;
}

int QNearFieldManagerPrivateImpl::registerNdefMessageHandler(QObject *object, const QMetaMethod &method)
{
    Q_UNUSED(object);
    Q_UNUSED(method);
    return -1;

    //Not supported for now
    /*
    ndefMessageHandlers.append(QPair<QPair<int, QObject *>,
                               QMetaMethod>(QPair<int, QObject *>(m_handlerID, object), method));

    //Returns the handler ID and increments it afterwards
    return m_handlerID++;
    */
}

int QNearFieldManagerPrivateImpl::registerNdefMessageHandler(const QNdefFilter &filter,
                                                             QObject *object, const QMetaMethod &method)
{
    Q_UNUSED(filter);
    Q_UNUSED(object);
    Q_UNUSED(method);
    return -1;

    //Not supported for now
    /*
    //If no record is set in the filter, we ignore the filter
    if (filter.recordCount()==0)
        return registerNdefMessageHandler(object, method);

    ndefFilterHandlers.append(QPair<QPair<int, QObject*>, QPair<QNdefFilter, QMetaMethod> >
                              (QPair<int, QObject*>(m_handlerID, object),
                               QPair<QNdefFilter, QMetaMethod>(filter, method)));

    // updateNdefFilter();

    return m_handlerID++;
    */
}

bool QNearFieldManagerPrivateImpl::unregisterNdefMessageHandler(int handlerId)
{
    Q_UNUSED(handlerId);
    return false;

    //Not supported for now
    /*
    for (int i=0; i<ndefMessageHandlers.count(); i++) {
        if (ndefMessageHandlers.at(i).first.first == handlerId) {
            ndefMessageHandlers.removeAt(i);
            // updateNdefFilter();
            return true;
        }
    }
    for (int i=0; i<ndefFilterHandlers.count(); i++) {
        if (ndefFilterHandlers.at(i).first.first == handlerId) {
            ndefFilterHandlers.removeAt(i);
            // updateNdefFilter();
            return true;
        }
    }
    return false;
    */
}

void QNearFieldManagerPrivateImpl::requestAccess(QNearFieldManager::TargetAccessModes accessModes)
{
    Q_UNUSED(accessModes);
    //Do nothing, because we dont have access modes for the target
}

void QNearFieldManagerPrivateImpl::releaseAccess(QNearFieldManager::TargetAccessModes accessModes)
{
    Q_UNUSED(accessModes);
    //Do nothing, because we dont have access modes for the target
}

void QNearFieldManagerPrivateImpl::newIntent(jobject intent)
{
    // This function is called from different thread and is used to move intent to main thread.
    QMetaObject::invokeMethod(this, "onTargetDiscovered", Qt::QueuedConnection, Q_ARG(jobject, intent));
}

QByteArray QNearFieldManagerPrivateImpl::getUid(jobject intent)
{
    if (intent == 0)
        return QByteArray();

    AndroidNfc::AttachedJNIEnv aenv;
    JNIEnv *env = aenv.jniEnv;
    jobject tag = AndroidNfc::getTag(env, intent);

    jclass tagClass = env->GetObjectClass(tag);
    Q_ASSERT_X(tagClass != 0, "getUid", "could not get Tag class");

    jmethodID getIdMID = env->GetMethodID(tagClass, "getId", "()[B");
    Q_ASSERT_X(getIdMID != 0, "getUid", "could not get method ID for getId()");

    jbyteArray tagId = reinterpret_cast<jbyteArray>(env->CallObjectMethod(tag, getIdMID));
    Q_ASSERT_X(tagId != 0, "getUid", "getId() returned null object");

    QByteArray uid;
    jsize len = env->GetArrayLength(tagId);
    uid.resize(len);
    env->GetByteArrayRegion(tagId, 0, len, reinterpret_cast<jbyte*>(uid.data()));

    return uid;
}

void QNearFieldManagerPrivateImpl::onTargetDiscovered(jobject intent)
{
    AndroidNfc::AttachedJNIEnv aenv;
    JNIEnv *env = aenv.jniEnv;
    Q_ASSERT_X(env != 0, "onTargetDiscovered", "env pointer is null");

    // Getting tag object and UID
    jobject tag = AndroidNfc::getTag(env, intent);
    QByteArray uid = getUid(env, tag);

    // Accepting all targest but only sending signal of requested types.
    NearFieldTarget *&target = m_detectedTargets[uid];
    if (target) {
        target->setIntent(intent);  // Updating existing target
    } else {
        target = new NearFieldTarget(intent, uid, this);
        connect(target, SIGNAL(targetDestroyed(QByteArray)), this, SLOT(onTargetDestroyed(QByteArray)));
        connect(target, SIGNAL(targetLost(QNearFieldTarget*)), this, SIGNAL(targetLost(QNearFieldTarget*)));
    }
    emit targetDetected(target);
}

void QNearFieldManagerPrivateImpl::onTargetDestroyed(const QByteArray &uid)
{
    m_detectedTargets.remove(uid);
}

QByteArray QNearFieldManagerPrivateImpl::getUid(JNIEnv *env, jobject tag)
{
    jclass tagClass = env->GetObjectClass(tag);
    jmethodID getIdMID = env->GetMethodID(tagClass, "getId", "()[B");
    jbyteArray tagId = reinterpret_cast<jbyteArray>(env->CallObjectMethod(tag, getIdMID));
    QByteArray uid;
    jsize len = env->GetArrayLength(tagId);
    uid.resize(len);
    env->GetByteArrayRegion(tagId, 0, len, reinterpret_cast<jbyte*>(uid.data()));
    return uid;
}

QT_END_NAMESPACE

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

#ifndef QNEARFIELDMANAGER_ANDROID_P_H
#define QNEARFIELDMANAGER_ANDROID_P_H

#include "qnearfieldmanager_p.h"
#include "qnearfieldmanager.h"
#include "qnearfieldtarget.h"
#include "android/androidjninfc_p.h"

#include <QHash>
#include <QMap>

QT_BEGIN_NAMESPACE

typedef QList<QNdefMessage> QNdefMessageList;

class NearFieldTarget;
class QByteArray;
class QNearFieldManagerPrivateImpl : public QNearFieldManagerPrivate, public AndroidNfc::AndroidNfcListenerInterface
{
    Q_OBJECT

public:
    QNearFieldManagerPrivateImpl();
    ~QNearFieldManagerPrivateImpl();

    virtual bool isAvailable() const;
    virtual bool startTargetDetection();
    virtual void stopTargetDetection();
    virtual int registerNdefMessageHandler(QObject *object, const QMetaMethod &method);
    virtual int registerNdefMessageHandler(const QNdefFilter &filter, QObject *object, const QMetaMethod &method);
    virtual bool unregisterNdefMessageHandler(int handlerId);
    virtual void requestAccess(QNearFieldManager::TargetAccessModes accessModes);
    virtual void releaseAccess(QNearFieldManager::TargetAccessModes accessModes);
    virtual void newIntent(jobject intent);
    QByteArray getUid(jobject intent);

public slots:
    void onTargetDiscovered(jobject intent);
    void onTargetDestroyed(const QByteArray &uid);
    void handlerTargetDetected(QNearFieldTarget *target);
    void handlerTargetLost(QNearFieldTarget *target);
    void handlerNdefMessageRead(const QNdefMessage &message, const QNearFieldTarget::RequestId &id);
    void handlerRequestCompleted(const QNearFieldTarget::RequestId &id);
    void handlerError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);

protected:
    static QByteArray getUid(JNIEnv *env, jobject tag);
    void updateReceiveState();

private:
    bool m_detecting;
    QHash<QByteArray, NearFieldTarget*> m_detectedTargets;
    QMap<QNearFieldTarget::RequestId, QNearFieldTarget*> m_idToTarget;

    int m_handlerID;
    QList< QPair<QPair<int, QObject *>, QMetaMethod> > ndefMessageHandlers;
    QList< QPair<QPair<int, QObject *>, QPair<QNdefFilter, QMetaMethod> > > ndefFilterHandlers;
};

QT_END_NAMESPACE

#endif // QNEARFIELDMANAGER_ANDROID_P_H

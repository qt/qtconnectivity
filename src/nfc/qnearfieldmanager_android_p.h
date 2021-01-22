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
****************************************************************************/

#ifndef QNEARFIELDMANAGER_ANDROID_P_H
#define QNEARFIELDMANAGER_ANDROID_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qnearfieldmanager_p.h"
#include "qnearfieldmanager.h"
#include "qnearfieldtarget.h"
#include "android/androidjninfc_p.h"

#include <QHash>
#include <QMap>
#include <QtAndroidExtras/QAndroidJniObject>
#include <QtAndroidExtras/QAndroidJniEnvironment>

QT_BEGIN_NAMESPACE

typedef QList<QNdefMessage> QNdefMessageList;

class NearFieldTarget;
class QByteArray;
class QNearFieldManagerPrivateImpl : public QNearFieldManagerPrivate, public AndroidNfc::AndroidNfcListenerInterface
{
    Q_OBJECT

public:
    QNearFieldManagerPrivateImpl();
    ~QNearFieldManagerPrivateImpl() override;

    bool isAvailable() const override;
    bool isSupported() const override;
    bool startTargetDetection() override;
    void stopTargetDetection() override;
    int registerNdefMessageHandler(QObject *object, const QMetaMethod &method) override;
    int registerNdefMessageHandler(const QNdefFilter &filter, QObject *object, const QMetaMethod &method) override;
    bool unregisterNdefMessageHandler(int handlerId) override;
    void requestAccess(QNearFieldManager::TargetAccessModes accessModes) override;
    void releaseAccess(QNearFieldManager::TargetAccessModes accessModes) override;
    void newIntent(QAndroidJniObject intent);
    QByteArray getUid(const QAndroidJniObject &intent);

public slots:
    void onTargetDiscovered(QAndroidJniObject intent);
    void onTargetDestroyed(const QByteArray &uid);
    void handlerTargetDetected(QNearFieldTarget *target);
    void handlerTargetLost(QNearFieldTarget *target);
    void handlerNdefMessageRead(const QNdefMessage &message, const QNearFieldTarget::RequestId &id);
    void handlerRequestCompleted(const QNearFieldTarget::RequestId &id);
    void handlerError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);

protected:
    static QByteArray getUidforTag(const QAndroidJniObject &tag);
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

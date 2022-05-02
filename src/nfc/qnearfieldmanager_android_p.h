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
******************************************************************************/

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
#include "qnearfieldtarget_android_p.h"
#include "android/androidjninfc_p.h"

#include <QHash>
#include <QMap>
#include <QtCore/QJniObject>
#include <QtCore/QJniEnvironment>

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

    bool isEnabled() const override;
    bool isSupported(QNearFieldTarget::AccessMethod accessMethod) const override;
    bool startTargetDetection(QNearFieldTarget::AccessMethod accessMethod) override;
    void stopTargetDetection(const QString &errorMessage) override;
    void newIntent(QJniObject intent) override;
    QByteArray getUid(const QJniObject &intent);

protected:
    static QByteArray getUidforTag(const QJniObject &tag);
    void updateReceiveState();

private:
    bool detecting;
    QNearFieldTarget::AccessMethod requestedMethod;
    QHash<QByteArray, QNearFieldTargetPrivateImpl*> detectedTargets;

private slots:
    void onTargetDiscovered(QJniObject intent);
    void onTargetDestroyed(const QByteArray &uid);
    void onTargetDetected(QNearFieldTargetPrivateImpl *target);
    void onTargetLost(QNearFieldTargetPrivateImpl *target);
};

QT_END_NAMESPACE

#endif // QNEARFIELDMANAGER_ANDROID_P_H

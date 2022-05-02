/****************************************************************************
**
** Copyright (C) 2020 Governikus GmbH & Co. KG
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

#ifndef QNEARFIELDMANAGER_IOS_P_H
#define QNEARFIELDMANAGER_IOS_P_H

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

#include <QTimer>

#import <os/availability.h>

Q_FORWARD_DECLARE_OBJC_CLASS(QT_MANGLE_NAMESPACE(QIosTagReaderDelegate));

QT_BEGIN_NAMESPACE

class QNearFieldTargetPrivateImpl;

class QNearFieldManagerPrivateImpl : public QNearFieldManagerPrivate
{
    Q_OBJECT

public:
    QNearFieldManagerPrivateImpl();
    ~QNearFieldManagerPrivateImpl();

    bool isEnabled() const override
    {
        return true;
    }

    bool isSupported(QNearFieldTarget::AccessMethod accessMethod) const override;

    bool startTargetDetection(QNearFieldTarget::AccessMethod accessMethod) override;
    void stopTargetDetection(const QString &errorMessage) override;

    void setUserInformation(const QString &message) override;

Q_SIGNALS:
    void tagDiscovered(void *tag);
    void didInvalidateWithError(bool doRestart);

private:
    QT_MANGLE_NAMESPACE(QIosTagReaderDelegate) *delegate API_AVAILABLE(ios(13.0)) = nullptr;
    bool detectionRunning = false;
    bool isRestarting = false;
    QList<QNearFieldTargetPrivateImpl *> detectedTargets;

    void startSession();
    void stopSession(const QString &error);
    void clearTargets();

private Q_SLOTS:
    void onTagDiscovered(void *target);
    void onTargetLost(QNearFieldTargetPrivateImpl *target);
    void onDidInvalidateWithError(bool doRestart);
};


QT_END_NAMESPACE

#endif // QNEARFIELDMANAGER_IOS_P_H

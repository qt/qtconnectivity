/***************************************************************************
**
** Copyright (C) 2016 BlackBerry Limited. All rights reserved.
** Copyright (C) 2016 BasysKom GmbH.
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

#ifndef QNEARFIELDMANAGER_NEARD_H
#define QNEARFIELDMANAGER_NEARD_H

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
#include "neard/neard_helper_p.h"

#include <QDBusObjectPath>
#include <QDBusVariant>
#include <QMap>

class OrgNeardManagerInterface;

QT_BEGIN_NAMESPACE

class QNearFieldManagerPrivateImpl : public QNearFieldManagerPrivate
{
    Q_OBJECT

public:
    QNearFieldManagerPrivateImpl();
    ~QNearFieldManagerPrivateImpl() override;

    bool isAvailable() const override;

    bool isSupported() const override;

    bool startTargetDetection() override;

    void stopTargetDetection() override;

    // not implemented
    int registerNdefMessageHandler(QObject *object, const QMetaMethod &method) override;

    int registerNdefMessageHandler(const QNdefFilter &filter, QObject *object,
                                   const QMetaMethod &method) override;

    bool unregisterNdefMessageHandler(int handlerId) override;

    void requestAccess(QNearFieldManager::TargetAccessModes accessModes) override;

    void releaseAccess(QNearFieldManager::TargetAccessModes accessModes) override;

private Q_SLOTS:
    void handleTagFound(const QDBusObjectPath&);
    void handleTagRemoved(const QDBusObjectPath&);

private:
    QString m_adapterPath;
    QMap<QString, QNearFieldTarget*> m_activeTags;
    NeardHelper *m_neardHelper;
};

QT_END_NAMESPACE


#endif // QNEARFIELDMANAGER_NEARD_H

/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#ifndef QNEARFIELDMANAGERVIRTUALBASE_P_H
#define QNEARFIELDMANAGERVIRTUALBASE_P_H

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

#include <QtCore/QMetaMethod>

QT_BEGIN_NAMESPACE

class QNearFieldManagerPrivateVirtualBase : public QNearFieldManagerPrivate
{
    Q_OBJECT

public:
    QNearFieldManagerPrivateVirtualBase();
    ~QNearFieldManagerPrivateVirtualBase() override;

    bool startTargetDetection() override;
    void stopTargetDetection() override;

    int registerNdefMessageHandler(QObject *object, const QMetaMethod &method) override;
    int registerNdefMessageHandler(const QNdefFilter &filter,
                                   QObject *object, const QMetaMethod &method) override;

    bool unregisterNdefMessageHandler(int id) override;

protected:
    struct Callback {
        QNdefFilter filter;

        QObject *object;
        QMetaMethod method;
    };

    void targetActivated(QNearFieldTarget *target);
    void targetDeactivated(QNearFieldTarget *target);

private:
    int getFreeId();
    void ndefReceived(const QNdefMessage &message, QNearFieldTarget *target);

    QList<Callback> m_registeredHandlers;
    QList<int> m_freeIds;
    QList<QNearFieldTarget::Type> m_detectTargetTypes;
};

QT_END_NAMESPACE

#endif // QNEARFIELDMANAGERVIRTUALBASE_P_H

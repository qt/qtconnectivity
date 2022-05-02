/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

#ifndef QNEARFIELDMANAGER_P_H
#define QNEARFIELDMANAGER_P_H

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

#include "qnearfieldmanager.h"
#include "qnearfieldtarget.h"
#include "qndefrecord.h"

#include "qtnfcglobal.h"

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QNdefFilter;

class Q_AUTOTEST_EXPORT QNearFieldManagerPrivate : public QObject
{
    Q_OBJECT

public:
    explicit QNearFieldManagerPrivate(QObject *parent = nullptr)
    :   QObject(parent)
    {
    }

    virtual ~QNearFieldManagerPrivate()
    {
    }

    virtual bool isEnabled() const
    {
        return false;
    }

    virtual bool isSupported(QNearFieldTarget::AccessMethod) const
    {
        return false;
    }

    virtual bool startTargetDetection(QNearFieldTarget::AccessMethod)
    {
        return false;
    }

    virtual void stopTargetDetection(const QString &)
    {
    }

    virtual void setUserInformation(const QString &)
    {
    }

signals:
    void adapterStateChanged(QNearFieldManager::AdapterState state);
    void targetDetectionStopped();
    void targetDetected(QNearFieldTarget *target);
    void targetLost(QNearFieldTarget *target);
};

QT_END_NAMESPACE

#endif // QNEARFIELDMANAGER_P_H

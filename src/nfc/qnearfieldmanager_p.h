/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QNEARFIELDMANAGER_P_H
#define QNEARFIELDMANAGER_P_H

#include "qnearfieldmanager.h"
#include "qnearfieldtarget.h"
#include "qndefrecord.h"

#include "qnfcglobal.h"

#include <QtCore/QObject>

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

class QNdefFilter;

class Q_AUTOTEST_EXPORT QNearFieldManagerPrivate : public QObject
{
    Q_OBJECT

public:
    explicit QNearFieldManagerPrivate(QObject *parent = 0)
    :   QObject(parent)
    {
    }

    ~QNearFieldManagerPrivate()
    {
    }

    virtual bool isAvailable() const
    {
        return false;
    }

    virtual bool startTargetDetection(const QList<QNearFieldTarget::Type> &targetTypes)
    {
        Q_UNUSED(targetTypes);

        return false;
    }

    virtual void stopTargetDetection()
    {
    }

    virtual int registerNdefMessageHandler(QObject *object, const QMetaMethod &/*method*/)
    {
        Q_UNUSED(object);

        return -1;
    }

    virtual int registerNdefMessageHandler(const QNdefFilter &/*filter*/,
                                           QObject *object, const QMetaMethod &/*method*/)
    {
        Q_UNUSED(object);

        return -1;
    }

    virtual bool unregisterNdefMessageHandler(int handlerId)
    {
        Q_UNUSED(handlerId);

        return false;
    }

    virtual void requestAccess(QNearFieldManager::TargetAccessModes accessModes)
    {
        m_requestedModes |= accessModes;
    }

    virtual void releaseAccess(QNearFieldManager::TargetAccessModes accessModes)
    {
        m_requestedModes &= ~accessModes;
    }

signals:
    void targetDetected(QNearFieldTarget *target);
    void targetLost(QNearFieldTarget *target);

public:
    QNearFieldManager::TargetAccessModes m_requestedModes;
};

QTNFC_END_NAMESPACE

QT_END_HEADER

#endif // QNEARFIELDMANAGER_P_H

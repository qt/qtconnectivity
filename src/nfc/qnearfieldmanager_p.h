/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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

#ifndef QNEARFIELDMANAGER_P_H
#define QNEARFIELDMANAGER_P_H

#include "qnearfieldmanager.h"
#include "qnearfieldtarget.h"
#include "qndefrecord.h"

#include "qnfcglobal.h"

#include <QtCore/QObject>

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

#endif // QNEARFIELDMANAGER_P_H

/***************************************************************************
**
** Copyright (C) 2012 Research In Motion
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

#ifndef QNEARFIELDMANAGER_QNX_P_H
#define QNEARFIELDMANAGER_QNX_P_H

#include "qnearfieldmanager_p.h"
#include "qnearfieldmanager.h"
#include "qnearfieldtarget.h"

#include "qnx/qnxnfcmanager_p.h"

QTNFC_BEGIN_NAMESPACE

class QNearFieldManagerPrivateImpl : public QNearFieldManagerPrivate
{
    Q_OBJECT

public:
    QNearFieldManagerPrivateImpl();
    ~QNearFieldManagerPrivateImpl();

    bool isAvailable() const;

    bool startTargetDetection(const QList<QNearFieldTarget::Type> &targetTypes);

    void stopTargetDetection();

    int registerNdefMessageHandler(QObject *object, const QMetaMethod &method);

    int registerNdefMessageHandler(const QNdefFilter &filter, QObject *object, const QMetaMethod &method);

    bool unregisterNdefMessageHandler(int handlerId);

    void requestAccess(QNearFieldManager::TargetAccessModes accessModes);

    void releaseAccess(QNearFieldManager::TargetAccessModes accessModes);

private Q_SLOTS:
    void handleMessage(const QNdefMessage&, QNearFieldTarget *);
    void newTarget(QNearFieldTarget *target, const QList<QNdefMessage> &);

private:
    void updateNdefFilter();
    QList<QNearFieldTarget::Type> m_detectTargetTypes;

    int m_handlerID;
    QList< QPair<QPair<int, QObject *>, QMetaMethod> > ndefMessageHandlers;
    QList< QPair<QPair<int, QObject *>, QPair<QNdefFilter, QMetaMethod> > > ndefFilterHandlers;
};

QTNFC_END_NAMESPACE

#endif // QNEARFIELDMANAGER_QNX_P_H

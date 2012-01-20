/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: http://www.qt-project.org/
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

#ifndef QNEARFIELDMANAGERVIRTUALBASE_P_H
#define QNEARFIELDMANAGERVIRTUALBASE_P_H

#include "qnearfieldmanager_p.h"

#include <QtCore/QMetaMethod>

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

class QNearFieldManagerPrivateVirtualBase : public QNearFieldManagerPrivate
{
    Q_OBJECT

public:
    QNearFieldManagerPrivateVirtualBase();
    ~QNearFieldManagerPrivateVirtualBase();

    bool startTargetDetection(const QList<QNearFieldTarget::Type> &targetTypes);
    void stopTargetDetection();

    int registerNdefMessageHandler(QObject *object, const QMetaMethod &method);
    int registerNdefMessageHandler(const QNdefFilter &filter,
                                   QObject *object, const QMetaMethod &method);

    bool unregisterNdefMessageHandler(int id);

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

QTNFC_END_NAMESPACE

QT_END_HEADER

#endif // QNEARFIELDMANAGERVIRTUALBASE_P_H

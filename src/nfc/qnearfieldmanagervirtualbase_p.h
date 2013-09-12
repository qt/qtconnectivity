/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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

#ifndef QNEARFIELDMANAGERVIRTUALBASE_P_H
#define QNEARFIELDMANAGERVIRTUALBASE_P_H

#include "qnearfieldmanager_p.h"

#include <QtCore/QMetaMethod>

QT_BEGIN_NAMESPACE

class QNearFieldManagerPrivateVirtualBase : public QNearFieldManagerPrivate
{
    Q_OBJECT

public:
    QNearFieldManagerPrivateVirtualBase();
    ~QNearFieldManagerPrivateVirtualBase();

    bool startTargetDetection();
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

QT_END_NAMESPACE

#endif // QNEARFIELDMANAGERVIRTUALBASE_P_H

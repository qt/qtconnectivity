/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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
#include "qdummyslot.h"
#include "qnfctagtestcommon.h"

QTNFC_USE_NAMESPACE

QDummySlot::QDummySlot(QObject *parent) :
    QObject(parent)
{
}

void QDummySlot::errorHandling(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId& id)
{
    if (id == iReqId)
    {
        qDebug()<<"get the error signal, wait for dummy request"<<endl;
        iOp->run();
        // iOp must use wait
        iOp->checkSignal();
        iOp->checkResponse();
        qDebug()<<"waited request completed"<<endl;
    }
}

void QDummySlot::requestCompletedHandling(const QNearFieldTarget::RequestId& id)
{
    if (id == iReqId)
    {
        qDebug()<<"get the requestCompleted signal, wait for dummy request"<<endl;
        iOp->run();
        // iOp must use wait
        iOp->checkSignal();
        iOp->checkResponse();
        qDebug()<<"waited request completed"<<endl;
    }
}

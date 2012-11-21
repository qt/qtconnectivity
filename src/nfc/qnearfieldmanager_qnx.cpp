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

#include "qnearfieldmanager_qnx_p.h"
#include <QDebug>
#include "qnearfieldtarget_qnx_p.h"
#include <QMetaMethod>
#include "qndeffilter.h"
#include "qndefrecord.h"

QTNFC_BEGIN_NAMESPACE

QNearFieldManagerPrivateImpl::QNearFieldManagerPrivateImpl() :
    handlerID(0)
{
    QNXNFCManager::instance()->registerForNewInstance();
    connect(QNXNFCManager::instance(), SIGNAL(ndefMessage(QNdefMessage, QNearFieldTarget)), this, SLOT(handleMessage(QNdefMessage, QNearFieldTarget)));

    m_requestedModes = QNearFieldManager::NdefWriteTargetAccess;
}

QNearFieldManagerPrivateImpl::~QNearFieldManagerPrivateImpl()
{
    //QNXNFCManager::instance()->unregisterObject(this);
    QNXNFCManager::instance()->unregisterForInstance();
}

bool QNearFieldManagerPrivateImpl::isAvailable() const
{
    return QNXNFCManager::instance()->isAvailable();
}

bool QNearFieldManagerPrivateImpl::startTargetDetection(const QList<QNearFieldTarget::Type> &targetTypes)
{
    if (QNXNFCManager::instance()->startTargetDetection(targetTypes)) {
        connect(QNXNFCManager::instance(), SIGNAL(targetDetected(NearFieldTarget *, const QList<QNdefMessage> &)),
                this, SLOT(newTarget(NearFieldTarget *, const QList<QNdefMessage> &)));
        return true;
    } else {
        qWarning()<<Q_FUNC_INFO<<"Could not start Target detection";
        return false;
    }
}

void QNearFieldManagerPrivateImpl::stopTargetDetection()
{
    disconnect(QNXNFCManager::instance(), SIGNAL(targetDetected(NearFieldTarget *, const QList<QNdefMessage> &)),
               this, SLOT(newTarget(NearFieldTarget *, const QList<QNdefMessage> &)));
    QNXNFCManager::instance()->unregisterTargetDetection(this);
}

int QNearFieldManagerPrivateImpl::registerNdefMessageHandler(QObject *object, const QMetaMethod &method)
{
    ndefMessageHandlers.append(QPair<QPair<int, QObject *>, QMetaMethod>(QPair<int, QObject *>(handlerID, object), method));
    //Wont work any more if a handler is deleted
    return handlerID++;
}

int QNearFieldManagerPrivateImpl::registerNdefMessageHandler(const QNdefFilter &filter,
                                                             QObject *object, const QMetaMethod &method)
{
    ndefFilterHandlers.append(QPair<QPair<int, QObject*>, QPair<QNdefFilter, QMetaMethod> >
                              (QPair<int, QObject*>(handlerID, object), QPair<QNdefFilter, QMetaMethod>(filter, method)));
    return handlerID++;
}

bool QNearFieldManagerPrivateImpl::unregisterNdefMessageHandler(int handlerId)
{
    for (int i=0; i<ndefMessageHandlers.count(); i++) {
        if (ndefMessageHandlers.at(i).first.first == handlerId) {
            ndefMessageHandlers.removeAt(i);
            return true;
        }
    }
    for (int i=0; i<ndefFilterHandlers.count(); i++) {
        if (ndefFilterHandlers.at(i).first.first == handlerId) {
            ndefFilterHandlers.removeAt(i);
            return true;
        }
    }
    return false;
}

void QNearFieldManagerPrivateImpl::requestAccess(QNearFieldManager::TargetAccessModes accessModes)
{
    Q_UNUSED(accessModes);
    //Do nothing, because we dont have access modes for the target
}

void QNearFieldManagerPrivateImpl::releaseAccess(QNearFieldManager::TargetAccessModes accessModes)
{
    Q_UNUSED(accessModes);
    //Do nothing, because we dont have access modes for the target
}

void QNearFieldManagerPrivateImpl::handleMessage(QNdefMessage message, QNearFieldTarget *target)
{
    //For message handlers without filters
    for (int i = 0; i < ndefMessageHandlers.count(); i++) {
        ndefMessageHandlers.at(i).second.invoke(ndefMessageHandlers.at(i).first.second, Q_ARG(QNdefMessage, message), Q_ARG(QNearFieldTarget*, target));
    }

    //For message handlers that specified a filter
    for (int i = 0; i < ndefFilterHandlers.count(); i++) {
        QNdefFilter filter = ndefFilterHandlers.at(i).second.first;
        if (filter.recordCount() > message.count())
            continue;
        for (int j = 0; j < filter.recordCount(); j++) {
            if (message.at(j).typeNameFormat() != filter.recordAt(j).typeNameFormat
                    || message.at(j).type() != filter.recordAt(j).type )
                continue;
        }
        ndefMessageHandlers.at(i).second.invoke(ndefMessageHandlers.at(i).first.second, Q_ARG(QNdefMessage, message), Q_ARG(QNearFieldTarget*, target));
    }
}

void QNearFieldManagerPrivateImpl::newTarget(NearFieldTarget<QNearFieldTarget> *target, const QList<QNdefMessage> &messages)
{
    emit targetDetected(target);
}

QTNFC_END_NAMESPACE

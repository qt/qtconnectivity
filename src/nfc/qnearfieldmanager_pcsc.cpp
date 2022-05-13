/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qnearfieldmanager_pcsc_p.h"
#include "qnearfieldtarget_pcsc_p.h"
#include "pcsc/qpcscmanager_p.h"
#include "pcsc/qpcsccard_p.h"
#include <QtCore/QLoggingCategory>
#include <QtCore/QThread>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

Q_DECLARE_LOGGING_CATEGORY(QT_NFC_PCSC)

/*
    Constructs a new near field manager private implementation.

    This object creates a worker thread with an instance of QPcscManager in
    it. All the communication with QPcscManager is done using signal-slot
    mechanism.
*/
QNearFieldManagerPrivateImpl::QNearFieldManagerPrivateImpl()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    m_worker = new QPcscManager;
    m_workerThread = new QThread(this);
    m_workerThread->setObjectName(u"QtNfcThread"_s);
    m_worker->moveToThread(m_workerThread);

    connect(m_worker, &QPcscManager::cardInserted, this,
            &QNearFieldManagerPrivateImpl::onCardInserted);
    connect(this, &QNearFieldManagerPrivateImpl::startTargetDetectionRequest, m_worker,
            &QPcscManager::onStartTargetDetectionRequest);
    connect(this, &QNearFieldManagerPrivateImpl::stopTargetDetectionRequest, m_worker,
            &QPcscManager::onStopTargetDetectionRequest);

    m_workerThread->start();
}

/*
    Destroys the near field manager private implementation.
*/
QNearFieldManagerPrivateImpl::~QNearFieldManagerPrivateImpl()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    // Destroy the worker. It calls QThread::quit() on the working thread in
    // its destructor.
    QMetaObject::invokeMethod(m_worker, &QObject::deleteLater, Qt::QueuedConnection);
    m_workerThread->wait();
}

bool QNearFieldManagerPrivateImpl::isEnabled() const
{
    return true;
}

bool QNearFieldManagerPrivateImpl::isSupported(QNearFieldTarget::AccessMethod accessMethod) const
{
    switch (accessMethod) {
    case QNearFieldTarget::TagTypeSpecificAccess:
    case QNearFieldTarget::NdefAccess:
        return true;
    default:
        return false;
    }
}

bool QNearFieldManagerPrivateImpl::startTargetDetection(QNearFieldTarget::AccessMethod accessMethod)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    Q_EMIT startTargetDetectionRequest(accessMethod);

    return true;
}

void QNearFieldManagerPrivateImpl::stopTargetDetection(const QString &errorMessage)
{
    Q_UNUSED(errorMessage);

    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    Q_EMIT stopTargetDetectionRequest();
}

/*
    Invoked when the worker has detected a new card.

    The worker will ensure that the card object remains valid until the manager
    emits targetCreatedForCard() signal.
*/
void QNearFieldManagerPrivateImpl::onCardInserted(QPcscCard *card, const QByteArray &uid,
                                                  QNearFieldTarget::AccessMethods accessMethods,
                                                  int maxInputLength)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    auto priv = new QNearFieldTargetPrivateImpl(uid, accessMethods, maxInputLength, this);

    connect(priv, &QNearFieldTargetPrivateImpl::disconnectRequest, card,
            &QPcscCard::onDisconnectRequest);
    connect(priv, &QNearFieldTargetPrivateImpl::destroyed, card, &QPcscCard::onTargetDestroyed);
    connect(priv, &QNearFieldTargetPrivateImpl::sendCommandRequest, card,
            &QPcscCard::onSendCommandRequest);
    connect(priv, &QNearFieldTargetPrivateImpl::readNdefMessagesRequest, card,
            &QPcscCard::onReadNdefMessagesRequest);
    connect(priv, &QNearFieldTargetPrivateImpl::writeNdefMessagesRequest, card,
            &QPcscCard::onWriteNdefMessagesRequest);

    connect(priv, &QNearFieldTargetPrivateImpl::targetLost, this,
            &QNearFieldManagerPrivateImpl::onTargetLost);

    connect(card, &QPcscCard::disconnected, priv, &QNearFieldTargetPrivateImpl::onDisconnected);
    connect(card, &QPcscCard::invalidated, priv, &QNearFieldTargetPrivateImpl::onInvalidated);
    connect(card, &QPcscCard::requestCompleted, priv,
            &QNearFieldTargetPrivateImpl::onRequestCompleted);
    connect(card, &QPcscCard::ndefMessageRead, priv,
            &QNearFieldTargetPrivateImpl::onNdefMessageRead);

    auto target = new QNearFieldTarget(priv, this);

    Q_EMIT targetDetected(target);

    // Let the worker know that the card object can be deleted if it is no
    // longer needed.
    QMetaObject::invokeMethod(card, &QPcscCard::enableAutodelete, Qt::QueuedConnection);
}

void QNearFieldManagerPrivateImpl::onTargetLost(QNearFieldTargetPrivate *target)
{
    Q_EMIT targetLost(target->q_ptr);
}

Q_LOGGING_CATEGORY(QT_NFC_PCSC, "qt.nfc.pcsc")

QT_END_NAMESPACE

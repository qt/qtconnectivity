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

#include "qnearfieldmanagervirtualbase_p.h"
#include "qndefmessage.h"
#include "qtlv_p.h"

QTNFC_BEGIN_NAMESPACE

static inline bool matchesTarget(QNearFieldTarget::Type type,
                                 const QList<QNearFieldTarget::Type> &types)
{
    return types.contains(type) || types.contains(QNearFieldTarget::AnyTarget);
}

QNearFieldManagerPrivateVirtualBase::QNearFieldManagerPrivateVirtualBase()
{
}

QNearFieldManagerPrivateVirtualBase::~QNearFieldManagerPrivateVirtualBase()
{
}

bool QNearFieldManagerPrivateVirtualBase::startTargetDetection(const QList<QNearFieldTarget::Type> &targetTypes)
{
    m_detectTargetTypes = targetTypes;
    return true;
}

void QNearFieldManagerPrivateVirtualBase::stopTargetDetection()
{
    m_detectTargetTypes.clear();
}

int QNearFieldManagerPrivateVirtualBase::getFreeId()
{
    if (!m_freeIds.isEmpty())
        return m_freeIds.takeFirst();

    m_registeredHandlers.append(Callback());
    return m_registeredHandlers.count() - 1;
}

int QNearFieldManagerPrivateVirtualBase::registerNdefMessageHandler(QObject *object,
                                                                    const QMetaMethod &method)
{
    int id = getFreeId();

    Callback &callback = m_registeredHandlers[id];

    callback.filter = QNdefFilter();
    callback.object = object;
    callback.method = method;

    return id;
}

int QNearFieldManagerPrivateVirtualBase::registerNdefMessageHandler(const QNdefFilter &filter,
                                                                    QObject *object,
                                                                    const QMetaMethod &method)
{
    int id = getFreeId();

    Callback &callback = m_registeredHandlers[id];

    callback.filter = filter;
    callback.object = object;
    callback.method = method;

    return id;
}

bool QNearFieldManagerPrivateVirtualBase::unregisterNdefMessageHandler(int id)
{
    if (id < 0 || id >= m_registeredHandlers.count())
        return false;

    m_freeIds.append(id);

    while (m_freeIds.contains(m_registeredHandlers.count() - 1)) {
        m_freeIds.removeAll(m_registeredHandlers.count() - 1);
        m_registeredHandlers.removeLast();
    }

    return true;
}

void QNearFieldManagerPrivateVirtualBase::targetActivated(QNearFieldTarget *target)
{
    if (matchesTarget(target->type(), m_detectTargetTypes))
        emit targetDetected(target);

    if (target->hasNdefMessage()) {
        QTlvReader reader(target);
        while (!reader.atEnd()) {
            if (!reader.readNext()) {
                if (!target->waitForRequestCompleted(reader.requestId()))
                    break;
                else
                    continue;
            }

            // NDEF Message TLV
            if (reader.tag() == 0x03)
                ndefReceived(QNdefMessage::fromByteArray(reader.data()), target);
        }
    }
}

void QNearFieldManagerPrivateVirtualBase::targetDeactivated(QNearFieldTarget *target)
{
    emit targetLost(target);
    QMetaObject::invokeMethod(target, "disconnected");
}

struct VerifyRecord
{
    QNdefFilter::Record filterRecord;
    unsigned int count;
};

void QNearFieldManagerPrivateVirtualBase::ndefReceived(const QNdefMessage &message,
                                                       QNearFieldTarget *target)
{
    for (int i = 0; i < m_registeredHandlers.count(); ++i) {
        if (m_freeIds.contains(i))
            continue;

        Callback &callback = m_registeredHandlers[i];

        bool matched = true;

        QList<VerifyRecord> filterRecords;
        for (int j = 0; j < callback.filter.recordCount(); ++j) {
            VerifyRecord vr;
            vr.count = 0;
            vr.filterRecord = callback.filter.recordAt(j);

            filterRecords.append(vr);
        }

        foreach (const QNdefRecord &record, message) {
            for (int j = 0; matched && (j < filterRecords.count()); ++j) {
                VerifyRecord &vr = filterRecords[j];

                if (vr.filterRecord.typeNameFormat == record.typeNameFormat() &&
                    vr.filterRecord.type == record.type()) {
                    ++vr.count;
                    break;
                } else {
                    if (callback.filter.orderMatch()) {
                        if (vr.filterRecord.minimum <= vr.count &&
                            vr.count <= vr.filterRecord.maximum) {
                            continue;
                        } else {
                            matched = false;
                        }
                    }
                }
            }
        }

        for (int j = 0; matched && (j < filterRecords.count()); ++j) {
            const VerifyRecord &vr = filterRecords.at(j);

            if (vr.filterRecord.minimum <= vr.count && vr.count <= vr.filterRecord.maximum)
                continue;
            else
                matched = false;
        }

        if (matched) {
            callback.method.invoke(callback.object, Q_ARG(QNdefMessage, message),
                                   Q_ARG(QNearFieldTarget *, target));
        }
    }
}

QTNFC_END_NAMESPACE

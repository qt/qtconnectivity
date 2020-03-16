/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "qnearfieldmanager_emulator_p.h"
#include "qnearfieldtarget_emulator_p.h"

#include "qndefmessage.h"
#include "qtlv_p.h"

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

QNearFieldManagerPrivateImpl::QNearFieldManagerPrivateImpl()
{
    TagActivator *activator = TagActivator::instance();
    activator->initialize();

    connect(activator, &TagActivator::tagActivated, this, &QNearFieldManagerPrivateImpl::tagActivated);
    connect(activator, &TagActivator::tagDeactivated, this, &QNearFieldManagerPrivateImpl::tagDeactivated);
}

QNearFieldManagerPrivateImpl::~QNearFieldManagerPrivateImpl()
{
}

bool QNearFieldManagerPrivateImpl::isEnabled() const
{
    return true;
}

bool QNearFieldManagerPrivateImpl::startTargetDetection()
{
    return true;
}

int QNearFieldManagerPrivateImpl::registerNdefMessageHandler(QObject *object,
                                                                    const QMetaMethod &method)
{
    int id = getFreeId();

    Callback &callback = m_registeredHandlers[id];

    callback.filter = QNdefFilter();
    callback.object = object;
    callback.method = method;

    return id;
}

int QNearFieldManagerPrivateImpl::registerNdefMessageHandler(const QNdefFilter &filter,
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

bool QNearFieldManagerPrivateImpl::unregisterNdefMessageHandler(int id)
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

void QNearFieldManagerPrivateImpl::reset()
{
    TagActivator::instance()->reset();
}

void QNearFieldManagerPrivateImpl::tagActivated(TagBase *tag)
{
    QNearFieldTarget *target = m_targets.value(tag).data();
    if (!target) {
        if (dynamic_cast<NfcTagType1 *>(tag))
            target = new TagType1(tag, this);
        else if (dynamic_cast<NfcTagType2 *>(tag))
            target = new TagType2(tag, this);
        else
            qFatal("Unknown emulator tag type");

        m_targets.insert(tag, target);
    }

    targetActivated(target);
}

void QNearFieldManagerPrivateImpl::tagDeactivated(TagBase *tag)
{
    QNearFieldTarget *target = m_targets.value(tag).data();
    if (!target) {
        m_targets.remove(tag);
        return;
    }

    targetDeactivated(target);
}

int QNearFieldManagerPrivateImpl::getFreeId()
{
    if (!m_freeIds.isEmpty())
        return m_freeIds.takeFirst();

    m_registeredHandlers.append(Callback());
    return m_registeredHandlers.count() - 1;
}

void QNearFieldManagerPrivateImpl::targetActivated(QNearFieldTarget *target)
{
    //if (matchesTarget(target->type(), m_detectTargetTypes))
    Q_EMIT targetDetected(target);

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

void QNearFieldManagerPrivateImpl::targetDeactivated(QNearFieldTarget *target)
{
    Q_EMIT targetLost(target);
    QMetaObject::invokeMethod(target, "disconnected");
}

struct VerifyRecord
{
    QNdefFilter::Record filterRecord;
    unsigned int count;
};

void QNearFieldManagerPrivateImpl::ndefReceived(const QNdefMessage &message,
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

        for (const QNdefRecord &record : message) {
            for (int j = 0; matched && (j < filterRecords.count()); ++j) {
                VerifyRecord &vr = filterRecords[j];

                if (vr.filterRecord.typeNameFormat == record.typeNameFormat() &&
                    ( vr.filterRecord.type == record.type() ||
                      vr.filterRecord.type.isEmpty()) ) {
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

QT_END_NAMESPACE

/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
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

bool QNearFieldManagerPrivateImpl::isAvailable() const
{
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

QT_END_NAMESPACE

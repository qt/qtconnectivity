/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

bool QNearFieldManagerPrivateImpl::isSupported(QNearFieldTarget::AccessMethod accessMethod) const
{
    return accessMethod == QNearFieldTarget::NdefAccess
            || accessMethod == QNearFieldTarget::AnyAccess;
}

bool QNearFieldManagerPrivateImpl::startTargetDetection(QNearFieldTarget::AccessMethod accessMethod)
{
    const bool supported = isSupported(accessMethod);
    if (supported)
        TagActivator::instance()->start();

    return supported;
}

void QNearFieldManagerPrivateImpl::stopTargetDetection(const QString &message)
{
    setUserInformation(message);
    TagActivator::instance()->stop();
    emit targetDetectionStopped();
}

void QNearFieldManagerPrivateImpl::setUserInformation(const QString &message)
{
    emit userInformationChanged(message);
}

void QNearFieldManagerPrivateImpl::reset()
{
    TagActivator::instance()->reset();
}

void QNearFieldManagerPrivateImpl::tagActivated(TagBase *tag)
{
    QNearFieldTargetPrivate *target = targets.value(tag).data();
    if (!target) {
        if (dynamic_cast<NfcTagType1 *>(tag))
            target = new TagType1(tag, this);
        else if (dynamic_cast<NfcTagType2 *>(tag))
            target = new TagType2(tag, this);
        else
            qFatal("Unknown emulator tag type");

        targets.insert(tag, target);
    }

    Q_EMIT targetDetected(new NearFieldTarget(target, this));
}

void QNearFieldManagerPrivateImpl::tagDeactivated(TagBase *tag)
{
    QNearFieldTargetPrivate *target = targets.value(tag).data();
    if (!target) {
        targets.remove(tag);
        return;
    }

    Q_EMIT targetLost(target->q_ptr);
    QMetaObject::invokeMethod(target->q_ptr, &QNearFieldTarget::disconnected);
}

QT_END_NAMESPACE

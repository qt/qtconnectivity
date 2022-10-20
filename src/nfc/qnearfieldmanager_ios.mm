/****************************************************************************
**
** Copyright (C) 2020 Governikus GmbH & Co. KG
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

#include "qnearfieldmanager_ios_p.h"

#include "ios/qiostagreaderdelegate_p.h"
#include "qnearfieldtarget_ios_p.h"

#include <QDateTime>

#import <CoreNFC/NFCReaderSession.h>
#import <CoreNFC/NFCNDEFReaderSession.h>
#import <CoreNFC/NFCTagReaderSession.h>

QT_BEGIN_NAMESPACE

QNearFieldManagerPrivateImpl::QNearFieldManagerPrivateImpl()
{
    if (@available(iOS 13, *))
        delegate = [[QT_MANGLE_NAMESPACE(QIosTagReaderDelegate) alloc] initWithListener:this];

    connect(this, &QNearFieldManagerPrivateImpl::tagDiscovered,
            this, &QNearFieldManagerPrivateImpl::onTagDiscovered,
            Qt::QueuedConnection);
    connect(this, &QNearFieldManagerPrivateImpl::didInvalidateWithError,
            this, &QNearFieldManagerPrivateImpl::onDidInvalidateWithError,
            Qt::QueuedConnection);
}

QNearFieldManagerPrivateImpl::~QNearFieldManagerPrivateImpl()
{
    if (@available(iOS 13, *))
        [delegate release];
}

bool QNearFieldManagerPrivateImpl::isSupported(QNearFieldTarget::AccessMethod accessMethod) const
{
    switch (accessMethod) {
    case QNearFieldTarget::AnyAccess:
        return NFCNDEFReaderSession.readingAvailable;
    case QNearFieldTarget::TagTypeSpecificAccess:
        if (@available(iOS 13, *))
            return NFCTagReaderSession.readingAvailable;
        Q_FALLTHROUGH();
    case QNearFieldTarget::UnknownAccess:
    case QNearFieldTarget::NdefAccess:
        return false;
    }
}

bool QNearFieldManagerPrivateImpl::startTargetDetection(QNearFieldTarget::AccessMethod accessMethod)
{
    if (detectionRunning)
        return false;

    switch (accessMethod) {
    case QNearFieldTarget::UnknownAccess:
    case QNearFieldTarget::NdefAccess:
    case QNearFieldTarget::AnyAccess:
        return false;
    case QNearFieldTarget::TagTypeSpecificAccess:
        if (@available(iOS 13, *))
            if (NFCTagReaderSession.readingAvailable) {
                detectionRunning = true;
                startSession();
                return true;
            }
        return false;
    }
}

void QNearFieldManagerPrivateImpl::stopTargetDetection(const QString &errorMessage)
{
    if (detectionRunning) {
       stopSession(errorMessage);
       detectionRunning = false;
       Q_EMIT targetDetectionStopped();
    }
}


void QNearFieldManagerPrivateImpl::startSession()
{
    if (detectionRunning)
        if (@available(iOS 13, *))
            [delegate startSession];
}

void QNearFieldManagerPrivateImpl::stopSession(const QString &error)
{
    clearTargets();

    if (@available(iOS 13, *))
        [delegate stopSession:error];
}

void QNearFieldManagerPrivateImpl::clearTargets()
{
    auto i = detectedTargets.begin();
    while (i != detectedTargets.end()) {
        (*i)->invalidate();
        Q_EMIT targetLost((*i)->q_ptr);
        i = detectedTargets.erase(i);
    }
}


void QNearFieldManagerPrivateImpl::setUserInformation(const QString &message)
{
    if (@available(iOS 13, *))
        [delegate alertMessage:message];
}

void QNearFieldManagerPrivateImpl::onTagDiscovered(void *tag)
{
    QNearFieldTargetPrivateImpl *target = new QNearFieldTargetPrivateImpl(tag);
    detectedTargets += target;
    connect(target, &QNearFieldTargetPrivateImpl::targetLost,
            this, &QNearFieldManagerPrivateImpl::onTargetLost);
    Q_EMIT targetDetected(new NearFieldTarget(target, this));
}

void QNearFieldManagerPrivateImpl::onTargetLost(QNearFieldTargetPrivateImpl *target)
{
    detectedTargets.removeOne(target);
    Q_EMIT targetLost(target->q_ptr);

    if (detectionRunning && detectedTargets.isEmpty())
        onDidInvalidateWithError(true);
}

void QNearFieldManagerPrivateImpl::onDidInvalidateWithError(bool doRestart)
{
    clearTargets();

    if (detectionRunning && doRestart)
    {
        if (!isRestarting) {
            isRestarting = true;
            using namespace std::chrono_literals;
            QTimer::singleShot(2s, this, [this](){
                        isRestarting = false;
                        startSession();
                    });
        }
        return;
    }

    detectionRunning = false;
    Q_EMIT targetDetectionStopped();
}

QT_END_NAMESPACE

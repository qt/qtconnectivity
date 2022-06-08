// Copyright (C) 2020 Governikus GmbH & Co. KG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
    Q_EMIT targetDetected(new QNearFieldTarget(target, this));
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

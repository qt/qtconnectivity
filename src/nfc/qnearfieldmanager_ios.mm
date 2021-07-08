// Copyright (C) 2020 Governikus GmbH & Co. KG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qnearfieldmanager_ios_p.h"

#include "ios/qiosnfcndefsessiondelegate_p.h"
#include "ios/qiostagreaderdelegate_p.h"
#include "ios/qiosndefnotifier_p.h"

#include "qnearfieldtarget_ios_p.h"

#include <QDateTime>

#include <memory>

#import <CoreNFC/NFCReaderSession.h>
#import <CoreNFC/NFCNDEFReaderSession.h>
#import <CoreNFC/NFCTagReaderSession.h>

QT_BEGIN_NAMESPACE

QNearFieldManagerPrivateImpl::QNearFieldManagerPrivateImpl()
{
    auto notifier = std::make_unique<QNfcNdefNotifier>();

    if (@available(iOS 13, *))
        delegate = [[QT_MANGLE_NAMESPACE(QIosTagReaderDelegate) alloc] initWithListener:this];

    connect(this, &QNearFieldManagerPrivateImpl::tagDiscovered,
            this, &QNearFieldManagerPrivateImpl::onTagDiscovered,
            Qt::QueuedConnection);
    connect(this, &QNearFieldManagerPrivateImpl::didInvalidateWithError,
            this, &QNearFieldManagerPrivateImpl::onDidInvalidateWithError,
            Qt::QueuedConnection);

    ndefDelegate = [[QIosNfcNdefSessionDelegate  alloc] initWithNotifier:notifier.get()];
    if (ndefDelegate) {
        auto watchDog = notifier.release(); // Delegate took the ownership.

        connect(watchDog, &QNfcNdefNotifier::tagDetected,
                this, &QNearFieldManagerPrivateImpl::onTagDiscovered,
                Qt::QueuedConnection);
        connect(watchDog, &QNfcNdefNotifier::invalidateWithError,
                this, &QNearFieldManagerPrivateImpl::onDidInvalidateWithError,
                Qt::QueuedConnection);
    } else {
        qCWarning(QT_IOS_NFC, "Failed to allocate NDEF reading session's delegate");
    }
}

QNearFieldManagerPrivateImpl::~QNearFieldManagerPrivateImpl()
{
    if (@available(iOS 13, *))
        [delegate release];

    if (!ndefDelegate)
        return;

    if (auto queue = qt_Nfc_Queue()) {
        dispatch_sync(queue, ^{
            [ndefDelegate abort];
        });
    }

    [ndefDelegate release];
}

bool QNearFieldManagerPrivateImpl::isSupported(QNearFieldTarget::AccessMethod accessMethod) const
{
    switch (accessMethod) {
    case QNearFieldTarget::AnyAccess:
    case QNearFieldTarget::NdefAccess:
        return NFCNDEFReaderSession.readingAvailable;
    case QNearFieldTarget::TagTypeSpecificAccess:
        if (@available(iOS 13, *))
            return NFCTagReaderSession.readingAvailable;
        Q_FALLTHROUGH();
    case QNearFieldTarget::UnknownAccess:
        return false;
    }
}

bool QNearFieldManagerPrivateImpl::startTargetDetection(QNearFieldTarget::AccessMethod accessMethod)
{
    if (detectionRunning)
        return false;

    activeAccessMethod = QNearFieldTarget::UnknownAccess;

    switch (accessMethod) {
    case QNearFieldTarget::UnknownAccess:
    case QNearFieldTarget::AnyAccess:
        return false;
    case QNearFieldTarget::TagTypeSpecificAccess:
        if (@available(iOS 13, *))
            if (NFCTagReaderSession.readingAvailable) {
                detectionRunning = true;
                startSession();
                activeAccessMethod = accessMethod;
                return true;
            }
        return false;
    case QNearFieldTarget::NdefAccess:
        if (NFCNDEFReaderSession.readingAvailable) {
            if (startNdefSession())
                activeAccessMethod = accessMethod;
            return detectionRunning;
        }
        return false;
    }

    return false;
}

void QNearFieldManagerPrivateImpl::stopTargetDetection(const QString &errorMessage)
{
    if (detectionRunning) {
        if (activeAccessMethod == QNearFieldTarget::TagTypeSpecificAccess) {
            stopSession(errorMessage);
        } else if (activeAccessMethod == QNearFieldTarget::NdefAccess) {
            stopNdefSession(errorMessage);
        } else {
            qCWarning(QT_IOS_NFC, "Unknown access method, cannot stop target detection");
            return;
        }

       detectionRunning = false;
       Q_EMIT targetDetectionStopped();
    }
}


void QNearFieldManagerPrivateImpl::startSession()
{
    if (detectionRunning) {
        if (@available(iOS 13, *)) {
            [delegate startSession];
        }
    }
}

bool QNearFieldManagerPrivateImpl::startNdefSession()
{
    if (!ndefDelegate)
        return detectionRunning = false;

    if (auto queue = qt_Nfc_Queue()) {
        dispatch_sync(queue, ^{
            detectionRunning = [ndefDelegate startSession];
        });
        return detectionRunning;
    }

    return detectionRunning = false;
}

void QNearFieldManagerPrivateImpl::stopSession(const QString &error)
{
    Q_ASSERT(activeAccessMethod == QNearFieldTarget::TagTypeSpecificAccess);

    clearTargets();

    if (@available(iOS 13, *)) {
        [delegate stopSession:error];
    }
}

void QNearFieldManagerPrivateImpl::stopNdefSession(const QString &error)
{
    Q_ASSERT(activeAccessMethod == QNearFieldTarget::NdefAccess);

    clearTargets();

    if (auto queue = qt_Nfc_Queue()) {
        dispatch_sync(queue, ^{
            [ndefDelegate stopSession:error];
        });
    }
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
    if (detectionRunning) {
        // Too late!
        qCWarning(QT_IOS_NFC, "User information must be set prior before the targer detection started");
        return;
    }

    [delegate alertMessage:message];

    if (auto queue = qt_Nfc_Queue()) {
        dispatch_sync(queue, ^{
           [ndefDelegate setAlertMessage:message];
        });
    }
}

void QNearFieldManagerPrivateImpl::onTagDiscovered(void *tag)
{
    QNearFieldTargetPrivateImpl *target = nullptr;
    if (activeAccessMethod == QNearFieldTarget::NdefAccess) {
        [id(tag) retain];
        target = new QNearFieldTargetPrivateImpl(ndefDelegate, tag);
    } else {
        target = new QNearFieldTargetPrivateImpl(tag);
    }

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
                        if (activeAccessMethod == QNearFieldTarget::NdefAccess)
                            startNdefSession();
                        else
                            startSession();
                    });
        }
        return;
    }

    detectionRunning = false;
    Q_EMIT targetDetectionStopped();
}

QT_END_NAMESPACE

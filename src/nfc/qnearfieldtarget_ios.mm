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

#include "qnearfieldtarget_ios_p.h"

#import <CoreNFC/NFCReaderSession.h>
#import <CoreNFC/NFCTagReaderSession.h>
#import <CoreNFC/NFCISO7816Tag.h>
#import <CoreNFC/NFCTag.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(ResponseProvider, responseProvider)

void ResponseProvider::provideResponse(QNearFieldTarget::RequestId requestId, bool success, QByteArray recvBuffer) {
    Q_EMIT responseReceived(requestId, success, recvBuffer);
}

void NfcTagDeleter::operator()(void *tag)
{
    [static_cast<id<NFCTag>>(tag) release];
}

QNearFieldTargetPrivateImpl:: QNearFieldTargetPrivateImpl(void *tag, QObject *parent) :
    QNearFieldTargetPrivate(parent),
    nfcTag(tag)
{
    Q_ASSERT(nfcTag);

    QObject::connect(this, &QNearFieldTargetPrivate::error, this, &QNearFieldTargetPrivateImpl::onTargetError);
    QObject::connect(responseProvider, &ResponseProvider::responseReceived, this, &QNearFieldTargetPrivateImpl::onResponseReceived);
    QObject::connect(&targetCheckTimer, &QTimer::timeout, this, &QNearFieldTargetPrivateImpl::onTargetCheck);
    targetCheckTimer.start(500);
}

QNearFieldTargetPrivateImpl::~QNearFieldTargetPrivateImpl()
{
}

void QNearFieldTargetPrivateImpl::invalidate()
{
    queue.clear();
    nfcTag.reset();
    targetCheckTimer.stop();
}

QByteArray QNearFieldTargetPrivateImpl::uid() const
{
    if (!nfcTag)
        return QByteArray();

    if (@available(iOS 13, *)) {
        id<NFCTag> tag = static_cast<id<NFCTag>>(nfcTag.get());
        id<NFCISO7816Tag> iso7816Tag = tag.asNFCISO7816Tag;
        if (iso7816Tag)
            return QByteArray::fromNSData(iso7816Tag.identifier);
    }

    return QByteArray();
}

QNearFieldTarget::Type QNearFieldTargetPrivateImpl::type() const
{
    if (!nfcTag)
        return QNearFieldTarget::ProprietaryTag;

    if (@available(iOS 13, *)) {
        id<NFCTag> tag = static_cast<id<NFCTag>>(nfcTag.get());
        id<NFCISO7816Tag> iso7816Tag = tag.asNFCISO7816Tag;

        if (tag.type != NFCTagTypeISO7816Compatible || iso7816Tag == nil)
             return QNearFieldTarget::ProprietaryTag;

        if (iso7816Tag.historicalBytes != nil && iso7816Tag.applicationData == nil)
            return QNearFieldTarget::NfcTagType4A;

        if (iso7816Tag.historicalBytes == nil && iso7816Tag.applicationData != nil)
            return QNearFieldTarget::NfcTagType4B;

        return QNearFieldTarget::NfcTagType4;
    }

    return QNearFieldTarget::ProprietaryTag;
}

QNearFieldTarget::AccessMethods QNearFieldTargetPrivateImpl::accessMethods() const
{
    if (@available(iOS 13, *)) {
        id<NFCTag> tag = static_cast<id<NFCTag>>(nfcTag.get());
        if (tag && [tag conformsToProtocol:@protocol(NFCISO7816Tag)])
            return QNearFieldTarget::TagTypeSpecificAccess;
    }

    return QNearFieldTarget::UnknownAccess;
}

int QNearFieldTargetPrivateImpl::maxCommandLength() const
{
    if (accessMethods() & QNearFieldTarget::TagTypeSpecificAccess)
        return 0xFEFF;

    return 0;
}

QNearFieldTarget::RequestId QNearFieldTargetPrivateImpl::sendCommand(const QByteArray &command)
{
    QNearFieldTarget::RequestId requestId = QNearFieldTarget::RequestId(new QNearFieldTarget::RequestIdPrivate());

    if (!(accessMethods() & QNearFieldTarget::TagTypeSpecificAccess)) {
        reportError(QNearFieldTarget::UnsupportedError, requestId);
        return requestId;
    }

    queue.enqueue(std::pair(requestId, command));

    if (!connect()) {
        reportError(QNearFieldTarget::TargetOutOfRangeError, requestId);
        return requestId;
    }

    onExecuteRequest();
    return requestId;
}

bool QNearFieldTargetPrivateImpl::isAvailable() const
{
    if (requestInProgress.isValid())
        return true;

    if (@available(iOS 13, *)) {
        id<NFCTag> tag = static_cast<id<NFCTag>>(nfcTag.get());
        return tag && (!connected || tag.available);
    }

    return false;
}

bool QNearFieldTargetPrivateImpl::connect()
{
    if (connected || requestInProgress.isValid())
        return true;

    if (!isAvailable() || queue.isEmpty())
        return false;

    if (@available(iOS 13, *)) {
        requestInProgress = queue.head().first;
        id<NFCTag> tag = static_cast<id<NFCTag>>(nfcTag.get());
        NFCTagReaderSession* session = tag.session;
        [session connectToTag: tag completionHandler: ^(NSError* error){
            const bool success = error == nil;
            QMetaObject::invokeMethod(this, [this, success] {
                requestInProgress = QNearFieldTarget::RequestId();
                if (success) {
                    connected = true;
                    onExecuteRequest();
                } else {
                    const auto requestId = queue.dequeue().first;
                    invalidate();
                    Q_EMIT targetLost(this);
                    reportError(QNearFieldTarget::ConnectionError, requestId);
                }
            });
        }];
        return true;
    }

    return false;
}

void QNearFieldTargetPrivateImpl::onTargetCheck()
{
    if (!isAvailable()) {
        invalidate();
        Q_EMIT targetLost(this);
    }
}

void QNearFieldTargetPrivateImpl::onTargetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id) {
    Q_UNUSED(id)
    if (error == QNearFieldTarget::TimeoutError) {
        invalidate();
        Q_EMIT targetLost(this);
    }
}

void QNearFieldTargetPrivateImpl::onExecuteRequest()
{
    if (!nfcTag || requestInProgress.isValid() || queue.isEmpty())
        return;

    if (@available(iOS 13, *)) {
        const auto request = queue.dequeue();
        requestInProgress = request.first;
        const auto tag = static_cast<id<NFCISO7816Tag>>(nfcTag.get());
        auto *apdu = [[[NFCISO7816APDU alloc] initWithData: request.second.toNSData()] autorelease];
        [tag sendCommandAPDU: apdu completionHandler: ^(NSData* responseData, uint8_t sw1, uint8_t sw2, NSError* error){
            QByteArray recvBuffer = QByteArray::fromNSData(responseData);
            recvBuffer += static_cast<char>(sw1);
            recvBuffer += static_cast<char>(sw2);
            const bool success = error == nil;
            responseProvider->provideResponse(request.first, success, recvBuffer);
        }];
    }
}

void  QNearFieldTargetPrivateImpl::onResponseReceived(QNearFieldTarget::RequestId requestId, bool success, QByteArray recvBuffer)
{
    if (requestInProgress != requestId)
        return;

    requestInProgress = QNearFieldTarget::RequestId();
    if (success) {
        setResponseForRequest(requestId, recvBuffer, true);
        onExecuteRequest();
    } else {
        invalidate();
        Q_EMIT targetLost(this);
        reportError(QNearFieldTarget::CommandError, requestId);
    }
}

QT_END_NAMESPACE

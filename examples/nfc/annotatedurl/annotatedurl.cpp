/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "annotatedurl.h"

#include <QtNfc/qnearfieldmanager.h>
#include <QtNfc/qnearfieldtarget.h>
#include <QtNfc/qndefmessage.h>
#include <QtNfc/qndefrecord.h>
#include <QtNfc/qndefnfctextrecord.h>
#include <QtNfc/qndefnfcurirecord.h>

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtGui/QMouseEvent>
#include <QtGui/QDesktopServices>
#include <QtCore/QDebug>
#include <QtCore/QLocale>
#include <QtCore/QUrl>

AnnotatedUrl::AnnotatedUrl(QObject *parent)
    : QObject(parent),
      manager(new QNearFieldManager(this))
{
    connect(manager, &QNearFieldManager::targetDetected,
            this, &AnnotatedUrl::targetDetected);
    connect(manager, &QNearFieldManager::targetLost,
            this, &AnnotatedUrl::targetLost);
    connect(manager, &QNearFieldManager::adapterStateChanged,
            this, &AnnotatedUrl::handleAdapterStateChange);

//! [populateFilter]
    messageFilter.setOrderMatch(false);
    messageFilter.appendRecord<QNdefNfcTextRecord>(1, 100);
    messageFilter.appendRecord<QNdefNfcUriRecord>(1, 1);
    messageFilter.appendRecord(QNdefRecord::Mime, "", 0, 1);
//! [populateFilter]
}

AnnotatedUrl::~AnnotatedUrl()
{

}

void AnnotatedUrl::startDetection()
{
    if (!manager->isEnabled()) {
        qWarning() << "NFC not enabled";
        emit nfcStateChanged(false);
        return;
    }

    if (manager->startTargetDetection(QNearFieldTarget::NdefAccess))
        emit nfcStateChanged(true);
}

void AnnotatedUrl::targetDetected(QNearFieldTarget *target)
{
    if (!target)
        return;

    connect(target, &QNearFieldTarget::ndefMessageRead,
            this, &AnnotatedUrl::handlePolledNdefMessage);
    connect(target, &QNearFieldTarget::error, this,
            [this]() { emit tagError("Tag read error"); });
    target->readNdefMessages();
}

void AnnotatedUrl::targetLost(QNearFieldTarget *target)
{
    if (target)
        target->deleteLater();
}

void AnnotatedUrl::handlePolledNdefMessage(QNdefMessage message)
{
    QNearFieldTarget *target = qobject_cast<QNearFieldTarget *>(sender());
    handleMessage(message, target);
}

//! [handleAdapterState]
void AnnotatedUrl::handleAdapterStateChange(QNearFieldManager::AdapterState state)
{
    if (state == QNearFieldManager::AdapterState::Online) {
        startDetection();
    } else if (state == QNearFieldManager::AdapterState::Offline) {
        manager->stopTargetDetection();
        emit nfcStateChanged(false);
    }
}
//! [handleAdapterState]

//! [handleMessage 1]
void AnnotatedUrl::handleMessage(const QNdefMessage &message, QNearFieldTarget *target)
{
//! [handleMessage 1]
    Q_UNUSED(target);

//! [handleMessage 2]
    if (!messageFilter.match(message)) {
        emit tagError("Invalid message format");
        return;
    }
//! [handleMessage 2]

    enum {
        MatchedNone,
        MatchedFirst,
        MatchedEnglish,
        MatchedLanguage,
        MatchedLanguageAndCountry
    } bestMatch = MatchedNone;

    QLocale defaultLocale;

    QString title;
    QUrl url;
    QPixmap pixmap;

//! [handleMessage 3]
    for (const QNdefRecord &record : message) {
        if (record.isRecordType<QNdefNfcTextRecord>()) {
            QNdefNfcTextRecord textRecord(record);

            title = textRecord.text();
            QLocale locale(textRecord.locale());
//! [handleMessage 3]
            // already found best match
            if (bestMatch == MatchedLanguageAndCountry) {
                // do nothing
            } else if (bestMatch <= MatchedLanguage && locale == defaultLocale) {
                bestMatch = MatchedLanguageAndCountry;
            } else if (bestMatch <= MatchedEnglish &&
                       locale.language() == defaultLocale.language()) {
                bestMatch = MatchedLanguage;
            } else if (bestMatch <= MatchedFirst && locale.language() == QLocale::English) {
                bestMatch = MatchedEnglish;
            } else if (bestMatch == MatchedNone) {
                bestMatch = MatchedFirst;
            }
//! [handleMessage 4]
        } else if (record.isRecordType<QNdefNfcUriRecord>()) {
            QNdefNfcUriRecord uriRecord(record);

            url = uriRecord.uri();
        } else if (record.typeNameFormat() == QNdefRecord::Mime &&
                   record.type().startsWith("image/")) {
            pixmap = QPixmap::fromImage(QImage::fromData(record.payload()));
        }
//! [handleMessage 4]
//! [handleMessage 5]
    }

    emit annotatedUrl(url, title, pixmap);
}
//! [handleMessage 5]

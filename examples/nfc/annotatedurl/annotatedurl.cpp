/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtNfc module.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
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
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
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

#include <qnearfieldtarget.h>
#include <qndefmessage.h>
#include <qndefrecord.h>
#include <qndefnfctextrecord.h>
#include <qndefnfcurirecord.h>

#include <QtCore/QUrl>
#include <QtCore/QLocale>

#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QDesktopServices>

AnnotatedUrl::AnnotatedUrl(QObject *parent)
:   QObject(parent)
{
}

AnnotatedUrl::~AnnotatedUrl()
{
}

void AnnotatedUrl::handleMessage(const QNdefMessage &message, QNearFieldTarget *target)
{
    Q_UNUSED(target);

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

    foreach (const QNdefRecord &record, message) {
        if (record.isRecordType<QNdefNfcTextRecord>()) {
            QNdefNfcTextRecord textRecord(record);

            QLocale locale(textRecord.locale());

            // already found best match
            if (bestMatch == MatchedLanguageAndCountry) {
                // do nothing
            } else if (bestMatch <= MatchedLanguage && locale == defaultLocale) {
                title = textRecord.text();
                bestMatch = MatchedLanguageAndCountry;
            } else if (bestMatch <= MatchedEnglish &&
                       locale.language() == defaultLocale.language()) {
                title = textRecord.text();
                bestMatch = MatchedLanguage;
            } else if (bestMatch <= MatchedFirst && locale.language() == QLocale::English) {
                title = textRecord.text();
                bestMatch = MatchedEnglish;
            } else if (bestMatch == MatchedNone) {
                title = textRecord.text();
                bestMatch = MatchedFirst;
            }
        } else if (record.isRecordType<QNdefNfcUriRecord>()) {
            QNdefNfcUriRecord uriRecord(record);

            url = uriRecord.uri();
        } else if (record.typeNameFormat() == QNdefRecord::Mime &&
                   record.type().startsWith("image/")) {
            pixmap = QPixmap::fromImage(QImage::fromData(record.payload()));
        }
    }

    emit annotatedUrl(url, title, pixmap);
}


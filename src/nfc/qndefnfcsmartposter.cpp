/****************************************************************************
**
** Copyright (C) 2013 Aaron McCarthy <mccarthy.aaron@gmail.com>
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

#include "qndefnfcsmartposter.h"

#include <QtCore/QLocale>
#include <QtCore/QUrl>

QTNFC_BEGIN_NAMESPACE

QNdefNfcSmartPoster::QNdefNfcSmartPoster()
{
}

QNdefNfcSmartPoster::QNdefNfcSmartPoster(const QNdefNfcSmartPoster &other)
:   QNdefMessage(other)
{
}

QNdefNfcSmartPoster::QNdefNfcSmartPoster(const QNdefMessage &other)
:   QNdefMessage(other)
{
}

bool QNdefNfcSmartPoster::isValid() const
{
    foreach (const QNdefRecord &record, *this) {
        if (!record.isRecordType<TitleRecord>() &&
            !record.isRecordType<UriRecord>() &&
            !record.isRecordType<IconRecord>() &&
            !record.isRecordType<ActionRecord>() &&
            !record.isRecordType<SizeRecord>() &&
            !record.isRecordType<TypeRecord>()) {
            return false;
        }
    }

    return true;
}

void QNdefNfcSmartPoster::setTitle(const QString &title, const QString &locale)
{
    for (int i = 0; i < count(); ++i) {
        const QNdefRecord &record = at(i);
        if (!record.isRecordType<TitleRecord>())
            continue;

        TitleRecord titleRecord(record);
        if (titleRecord.locale() == locale) {
            titleRecord.setText(title);
            replace(i, titleRecord);
            return;
        }
    }

    TitleRecord titleRecord;
    titleRecord.setLocale(locale);
    titleRecord.setText(title);

    append(titleRecord);
}

QString QNdefNfcSmartPoster::title(const QString &locale) const
{
    const QLocale defaultLocale(locale);

    enum {
        MatchedNone,
        MatchedFirst,
        MatchedEnglish,
        MatchedLanguage,
        MatchedLanguageAndCountry
    } bestMatch = MatchedNone;

    QString bestTitle;

    foreach (const QNdefRecord &record, *this) {
        if (!record.isRecordType<TitleRecord>())
            continue;

        TitleRecord titleRecord(record);

        const QLocale locale(titleRecord.locale());

        // already found best match
        if (bestMatch == MatchedLanguageAndCountry) {
            // do nothing
        } else if (bestMatch <= MatchedLanguage && locale == defaultLocale) {
            bestTitle = titleRecord.text();
            bestMatch = MatchedLanguageAndCountry;
        } else if (bestMatch <= MatchedEnglish && locale.language() == defaultLocale.language()) {
            bestTitle = titleRecord.text();
            bestMatch = MatchedLanguage;
        } else if (bestMatch <= MatchedFirst && locale.language() == QLocale::English) {
            bestTitle = titleRecord.text();
            bestMatch = MatchedEnglish;
        } else if (bestMatch == MatchedNone) {
            bestTitle = titleRecord.text();
            bestMatch = MatchedFirst;
        }
    }

    return bestTitle;
}

void QNdefNfcSmartPoster::setUri(const QUrl &uri)
{
    for (int i = 0; i < count(); ++i) {
        const QNdefRecord &record = at(i);
        if (record.isRecordType<UriRecord>()) {
            UriRecord uriRecord(record);
            uriRecord.setUri(uri);
            replace(i, uriRecord);
        }
    }
}

QUrl QNdefNfcSmartPoster::uri() const
{
    foreach (const QNdefRecord &record, *this) {
        if (!record.isRecordType<UriRecord>())
            continue;

        UriRecord uriRecord(record);
        return uriRecord.uri();
    }

    return QUrl();
}

void QNdefNfcSmartPoster::setDefaultAction(Action action)
{
    for (int i = 0; i < count(); ++i) {
        const QNdefRecord &record = at(i);
        if (record.isRecordType<ActionRecord>()) {
            ActionRecord actionRecord(record);
            actionRecord.setAction(action);
            replace(i, actionRecord);
        }
    }
}

QNdefNfcSmartPoster::Action QNdefNfcSmartPoster::defaultAction() const
{
    foreach (const QNdefRecord &record, *this) {
        if (!record.isRecordType<ActionRecord>())
            continue;

        ActionRecord actionRecord(record);
        return actionRecord.action();
    }

    return NoAction;
}

void QNdefNfcSmartPoster::setContentSize(quint32 size)
{
    for (int i = 0; i < count(); ++i) {
        const QNdefRecord &record = at(i);
        if (record.isRecordType<SizeRecord>()) {
            SizeRecord sizeRecord(record);
            sizeRecord.setSize(size);
            replace(i, sizeRecord);
        }
    }
}

quint32 QNdefNfcSmartPoster::contentSize() const
{
    foreach (const QNdefRecord &record, *this) {
        if (!record.isRecordType<SizeRecord>())
            continue;

        SizeRecord sizeRecord(record);
        return sizeRecord.size();
    }

    return 0;
}

void QNdefNfcSmartPoster::setContentType(const QByteArray &mimeType)
{
    for (int i = 0; i < count(); ++i) {
        const QNdefRecord &record = at(i);
        if (record.isRecordType<TypeRecord>()) {
            TypeRecord typeRecord(record);
            typeRecord.setMimeType(mimeType);
            replace(i, typeRecord);
        }
    }
}

QByteArray QNdefNfcSmartPoster::contentType() const
{
    foreach (const QNdefRecord &record, *this) {
        if (!record.isRecordType<TypeRecord>())
            continue;

        TypeRecord typeRecord(record);
        return typeRecord.mimeType();
    }

    return QByteArray();
}

void QNdefNfcSmartPoster::ActionRecord::setAction(Action action)
{
    switch (action) {
    case OpenForEditingAction:
        setPayload(QByteArray(1, 2));
        break;
    case SaveForLaterAction:
        setPayload(QByteArray(1, 1));
        break;
    case NoAction:
    default:
        setPayload(QByteArray(1, 0));
        break;
    }
}

QNdefNfcSmartPoster::Action QNdefNfcSmartPoster::ActionRecord::action() const
{
    if (payload().length() != 1)
        return NoAction;

    switch (quint8(payload().at(0))) {
    case 2:
        return OpenForEditingAction;
    case 1:
        return SaveForLaterAction;
    case 0:
    default:
        return NoAction;
    }
}

void QNdefNfcSmartPoster::SizeRecord::setSize(quint32 size)
{
    QByteArray contentSize;
    contentSize.resize(4);

    contentSize[0] = (size >> 24) & 0xff;
    contentSize[1] = (size >> 16) & 0xff;
    contentSize[2] = (size >> 8) & 0xff;
    contentSize[3] = (size >> 0) & 0xff;

    setPayload(contentSize);
}

quint32 QNdefNfcSmartPoster::SizeRecord::size() const
{
    const QByteArray contentSize = payload();
    if (contentSize.length() != 4)
        return 0;

    return quint8(contentSize.at(0)) << 24 |
           quint8(contentSize.at(1)) << 16 |
           quint8(contentSize.at(2)) << 8 |
           quint8(contentSize.at(3)) << 0;
}

void QNdefNfcSmartPoster::TypeRecord::setMimeType(const QByteArray &mimeType)
{
    setPayload(mimeType);
}

QByteArray QNdefNfcSmartPoster::TypeRecord::mimeType() const
{
    return payload();
}

QTNFC_END_NAMESPACE

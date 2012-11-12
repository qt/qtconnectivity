/***************************************************************************
 **
 ** Copyright (C) 2011 - 2012 Research In Motion
 ** Contact: http://www.qt-project.org/legal
 **
 ** This file is part of the plugins of the Qt Toolkit.
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

#include <qndefnfcsmartposterrecord.h>
#include <qndefnfcsmartposterrecord_p.h>
#include <qndefmessage.h>
#include <qndefnfcurirecord.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QUrl>

QTNFC_BEGIN_NAMESPACE

/*!
    \class QNdefNfcSmartPosterRecord
    \brief The QNdefNfcSmartPosterRecord class provides an NFC RTD-SmartPoster.

    \ingroup connectivity-nfc
    \inmodule QtNfc

    RTD-SmartPoster encapsulates a Smart Poster.
 */

/*!
    \enum QNdefNfcSmartPosterRecord::Action

    This enum describes the course of action that a device should take with the content.

    \value UnspecifiedAction    The action is not defined.
    \value DoAction             Do the action (send the SMS, launch the browser, make the telephone call).
    \value SaveAction           Save for later (store the SMS in INBOX, put the URI in a bookmark, save the telephone number in contacts).
    \value EditAction           Open for editing (open an SMS in the SMS editor, open the URI in a URI editor, open the telephone number for editing).
 */

/*!
    Returns true if the smart poster record contains a title record encoded in \a locale. If \a locale
    is empty then true is returned if at least one title is present regardless of \a locale, otherwise
    false is returned.
 */
bool QNdefNfcSmartPosterRecord::hasTitle(const QString &locale) const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return false;

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcTextRecord>()) {
            QNdefNfcTextRecord text(rec);

            if (locale.isEmpty() || text.locale() == locale)
                return true;
        }
    }

    return false;
}

/*!
    Returns true if the smart poster record contains an action record, otherwise false.
 */
bool QNdefNfcSmartPosterRecord::hasAction() const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return false;

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcActRecord>())
            return true;
    }

    return false;
}

/*!
    Returns true if the smart poster record contains an icon record with mimetype \a mimetype. If \a mimetype
    is empty then true is returned if at least one icon is present regardless of \a mimetype, otherwise
    false is returned.
 */
bool QNdefNfcSmartPosterRecord::hasIcon(const QByteArray &mimetype) const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return false;

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcIconRecord>()) {
            QNdefNfcIconRecord icon(rec);

            if (mimetype.isEmpty() || icon.type() == mimetype)
                return true;
        }
    }

    return false;
}

/*!
    Returns true if the smart poster record contains a size record, otherwise false.
 */
bool QNdefNfcSmartPosterRecord::hasSize() const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return false;

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcSizeRecord>())
            return true;
    }

    return false;
}

/*!
    Returns true if the smart poster record contains a type record, otherwise false.
 */
bool QNdefNfcSmartPosterRecord::hasTypeInfo() const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return false;

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcTypeRecord>())
            return true;
    }

    return false;
}

/*!
    Returns the title from the title record encoded in \a locale if available. If \a locale
    is empty then the title from the first available record is returned regardless of \a locale. In all other
    cases an empty string is returned.
 */
QString QNdefNfcSmartPosterRecord::title(const QString &locale) const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return QString();

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcTextRecord>()) {
            QNdefNfcTextRecord text(rec);

            if (locale.isEmpty() || text.locale() == locale)
                return text.text();
        }
    }

    return QString();
}

/*!
    Returns the locale from the title record encoded in \a locale if available. If \a locale
    is empty then the locale from the first available record is returned regardless of \a locale. In all other
    cases an empty string is returned.
 */
QString QNdefNfcSmartPosterRecord::titleLocale(const QString &locale) const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return QString();

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcTextRecord>()) {
            QNdefNfcTextRecord text(rec);

            if (locale.isEmpty() || text.locale() == locale)
                return text.locale();
        }
    }

    return QString();
}

/*!
    Returns the encoding from the title record encoded in \a locale if available. If \a locale
    is empty then the encoding from the first available record is returned regardless of \a locale. In all other
    cases \a Utf8 is returned.
 */
QNdefNfcTextRecord::Encoding QNdefNfcSmartPosterRecord::titleEncoding(const QString &locale) const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return QNdefNfcTextRecord::Utf8;

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcTextRecord>()) {
            QNdefNfcTextRecord text(rec);

            if (text.locale() == locale || locale.isEmpty())
                return text.encoding();
        }
    }

    return QNdefNfcTextRecord::Utf8;
}

/*!
    Adds a new title record containing the title \a text using locale \a locale and encoded in \a encoding.
    The method inserts the record and returns true if the smart poster does not already contain a title encoded in \a locale,
    otherwise the title is not added and false is returned. Returns true if the smart poster does not already contain a title encoded
    in \a locale, otherwise false
 */
bool QNdefNfcSmartPosterRecord::addTitle(const QString &text, const QString &locale, QNdefNfcTextRecord::Encoding encoding)
{
    const QByteArray p = payload();
    QNdefMessage msg;

    if (!p.isEmpty()) {
        msg = QNdefMessage::fromByteArray(p);

        for (int i = 0; i < msg.length(); ++i) {
            const QNdefRecord &rec = msg.at(i);

            if (rec.isRecordType<QNdefNfcTextRecord>()) {
                QNdefNfcTextRecord text(rec);

                if (text.locale() == locale)
                    return false;
            }
        }
    }

    QNdefNfcTextRecord rec;
    rec.setText(text);
    rec.setLocale(locale);
    rec.setEncoding(encoding);

    msg.append(rec);

    setPayload(msg.toByteArray());
    return true;
}

bool QNdefNfcSmartPosterRecord::removeTitle(const QString &locale)
{
    const QByteArray p = payload();
    bool removed = false;

    if (!p.isEmpty()) {
        QNdefMessage msg = QNdefMessage::fromByteArray(p);
        QNdefMessage newMsg;

        for (int i = 0; i < msg.length(); ++i) {
            const QNdefRecord &rec = msg.at(i);

            if (rec.isRecordType<QNdefNfcTextRecord>()) {
                QNdefNfcTextRecord text(rec);

                if (text.locale() == locale)
                    removed = true;

                else
                    newMsg.append(rec);
            }

            else
                newMsg.append(rec);
        }

        setPayload(newMsg.toByteArray());
    }

    return removed;
}

QStringList QNdefNfcSmartPosterRecord::titleLocales()
{
    const QByteArray p = payload();
    QStringList locales;

    if (p.isEmpty())
        return QStringList();

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcTextRecord>()) {
            QNdefNfcTextRecord text(rec);
            locales.append(text.locale());
        }
    }

    return locales;
}

/*!
    Returns the URI from the URI record if available. Otherwise an empty URI is returned.
 */
QUrl QNdefNfcSmartPosterRecord::uri() const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return QUrl();

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcUriRecord>())
            return QNdefNfcUriRecord(rec).uri();
    }

    return QUrl();
}

/*!
    Sets the URI record to \a url
 */
void QNdefNfcSmartPosterRecord::setUri(const QUrl &url)
{
    const QByteArray p = payload();
    QNdefMessage msg;

    if (!p.isEmpty()) {
        msg = QNdefMessage::fromByteArray(p);

        for (int i = 0; i < msg.length(); ++i) {
            const QNdefRecord &rec = msg.at(i);

            if (rec.isRecordType<QNdefNfcUriRecord>())
                msg.removeAt(i);
        }
    }

    QNdefNfcUriRecord rec;
    rec.setUri(url);

    msg.append(rec);

    setPayload(msg.toByteArray());
}

/*!
    Returns the action from the action record if available. Otherwise \a UnspecifiedAction is returned.
 */
QNdefNfcSmartPosterRecord::Action QNdefNfcSmartPosterRecord::action() const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return UnspecifiedAction;

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcActRecord>())
            return QNdefNfcActRecord(rec).action();
    }

    return UnspecifiedAction;
}

/*!
    Sets the action record to \a act
 */
void QNdefNfcSmartPosterRecord::setAction(Action act)
{
    const QByteArray p = payload();
    QNdefMessage msg;

    if (!p.isEmpty()) {
        msg = QNdefMessage::fromByteArray(p);

        for (int i = 0; i < msg.length(); ++i) {
            const QNdefRecord &rec = msg.at(i);

            if (rec.isRecordType<QNdefNfcActRecord>())
                msg.removeAt(i);
        }
    }

    QNdefNfcActRecord rec;
    rec.setAction(act);

    msg.append(rec);

    setPayload(msg.toByteArray());
}

/*!
    Returns the icon data from the first icon record matching \a mimetype if available.
    Otherwise an empty byte array is returned.
 */
QByteArray QNdefNfcSmartPosterRecord::icon(const QByteArray &mimetype) const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return QByteArray();

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcIconRecord>()) {
            QNdefNfcIconRecord icon(rec);

            if (icon.type() == mimetype)
                return icon.data();
        }
    }

    return QByteArray();
}

/*!
    Returns the icon type from the first icon record matching \a mimetype if available. Otherwise an empty byte array is returned.
 */
QByteArray QNdefNfcSmartPosterRecord::iconType(const QByteArray &mimetype) const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return QByteArray();

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcIconRecord>()) {
            QNdefNfcIconRecord icon(rec);

            if (icon.type() == mimetype)
                return icon.type();
        }
    }

    return QByteArray();
}

/*!
    Returns the icon data from the icon record at position \a index if available. Otherwise an empty byte array is returned.
 */
QByteArray QNdefNfcSmartPosterRecord::icon(int index) const
{
    const QByteArray p = payload();

    if (p.isEmpty()) {
        return QByteArray();
    }

    QNdefMessage msg = QNdefMessage::fromByteArray(p);
    int indexFound = -1;

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcIconRecord>()) {
            indexFound++;

            if (index == indexFound)
                return QNdefNfcIconRecord(rec).data();
        }
    }

    return QByteArray();
}

/*!
    Returns the icon type from the icon record at position \a index if available. Otherwise an empty byte array is returned.
 */
QByteArray QNdefNfcSmartPosterRecord::iconType(int index) const
{
    const QByteArray p = payload();

    if (p.isEmpty()) {
        return QByteArray();
    }

    QNdefMessage msg = QNdefMessage::fromByteArray(p);
    int indexFound = -1;

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcIconRecord>()) {
            indexFound++;

            if (index == indexFound)
                return QNdefNfcIconRecord(rec).type();
        }
    }

    return QByteArray();
}

/*!
    Returns the total number of icon records contained in the smart poster record.
 */
quint32 QNdefNfcSmartPosterRecord::iconCount() const
{
    const QByteArray p = payload();

    if (p.isEmpty()) {
        return 0;
    }

    QNdefMessage msg = QNdefMessage::fromByteArray(p);
    quint32 count = 0;

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcIconRecord>())
            count++;
    }

    return count;
}

QList<QByteArray> QNdefNfcSmartPosterRecord::iconTypes() const
{
    const QByteArray p = payload();
    QList<QByteArray> types;

    if (p.isEmpty()) {
        return QList<QByteArray>();
    }

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcIconRecord>())
            types.append(QNdefNfcIconRecord(rec).type());
    }

    return types;
}

/*!
    Adds a new icon record with type \a type and data \a data.
 */
void QNdefNfcSmartPosterRecord::addIcon(const QByteArray &type, const QByteArray &data)
{
    const QByteArray p = payload();
    QNdefMessage msg;

    if (!p.isEmpty())
        msg = QNdefMessage::fromByteArray(p);

    QNdefNfcIconRecord icon;
    icon.setType(type);
    icon.setData(data);

    msg.append(icon);

    setPayload(msg.toByteArray());
}

void QNdefNfcSmartPosterRecord::removeIcon(const QByteArray &type)
{
    const QByteArray p = payload();

    if (!p.isEmpty()) {
        QNdefMessage msg = QNdefMessage::fromByteArray(p);
        QNdefMessage newMsg;

        for (int i = 0; i < msg.length(); ++i) {
            const QNdefRecord &rec = msg.at(i);

            if (rec.isRecordType<QNdefNfcIconRecord>()) {
                QNdefNfcIconRecord icon(rec);

                if (icon.type() != type)
                    newMsg.append(rec);
            }

            else
                newMsg.append(rec);
        }

        setPayload(newMsg.toByteArray());
    }
}

/*!
    Returns the size from the size record if available. Otherwise returns 0.
 */
quint32 QNdefNfcSmartPosterRecord::size() const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return 0;

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcSizeRecord>())
            return QNdefNfcSizeRecord(rec).size();
    }

    return 0;
}

/*!
    Sets the size record to \a size
 */
void QNdefNfcSmartPosterRecord::setSize(quint32 size)
{
    const QByteArray p = payload();
    QNdefMessage msg;

    if (!p.isEmpty()) {
        msg = QNdefMessage::fromByteArray(p);

        for (int i = 0; i < msg.length(); ++i) {
            const QNdefRecord &rec = msg.at(i);

            if (rec.isRecordType<QNdefNfcSizeRecord>())
                msg.removeAt(i);
        }
    }

    QNdefNfcSizeRecord rec;
    rec.setSize(size);
    msg.append(rec);

    setPayload(msg.toByteArray());
}

/*!
    Returns the type from the type record if available. Otherwise returns an empty byte array.
 */
QByteArray QNdefNfcSmartPosterRecord::typeInfo() const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return QByteArray();

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); ++i) {
        const QNdefRecord &rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcTypeRecord>())
            return QNdefNfcTypeRecord(rec).typeInfo();
    }

    return QByteArray();
}

/*!
    Sets the type record to \a type
 */
void QNdefNfcSmartPosterRecord::setTypeInfo(const QByteArray &type)
{
    const QByteArray p = payload();
    QNdefMessage msg;

    if (!p.isEmpty()) {
        msg = QNdefMessage::fromByteArray(p);

        for (int i = 0; i < msg.length(); ++i) {
            const QNdefRecord &rec = msg.at(i);

            if (rec.isRecordType<QNdefNfcTypeRecord>())
                msg.removeAt(i);
        }
    }

    QNdefNfcTypeRecord rec;
    rec.setTypeInfo(type);
    msg.append(rec);

    setPayload(msg.toByteArray());
}

void QNdefNfcActRecord::setAction(QNdefNfcSmartPosterRecord::Action action)
{
    QByteArray data;
    data[0] = action;

    setPayload(data);
}

QNdefNfcSmartPosterRecord::Action QNdefNfcActRecord::action() const
{
    const QByteArray p = payload();
    QNdefNfcSmartPosterRecord::Action value =
            QNdefNfcSmartPosterRecord::UnspecifiedAction;

    if (!p.isEmpty())
        value = QNdefNfcSmartPosterRecord::Action(p[0]);

    return value;
}

void QNdefNfcIconRecord::setData(const QByteArray &data)
{
    setPayload(data);
}

QByteArray QNdefNfcIconRecord::data() const
{
    return payload();
}

void QNdefNfcSizeRecord::setSize(quint32 size)
{
    QByteArray data;

    data[0] = (int) ((size & 0xFF000000) >> 24);
    data[1] = (int) ((size & 0x00FF0000) >> 16);
    data[2] = (int) ((size & 0x0000FF00) >> 8);
    data[3] = (int) ((size & 0x000000FF));

    setPayload(data);
}

quint32 QNdefNfcSizeRecord::size() const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return 0;

    return ((p[0] << 24) & 0xFF000000) + ((p[1] << 16) & 0x00FF0000)
            + ((p[2] << 8) & 0x0000FF00) + (p[3] & 0x000000FF);
}

void QNdefNfcTypeRecord::setTypeInfo(const QByteArray &type)
{
    setPayload(type);
}

QByteArray QNdefNfcTypeRecord::typeInfo() const
{
    return payload();
}

QTNFC_END_NAMESPACE

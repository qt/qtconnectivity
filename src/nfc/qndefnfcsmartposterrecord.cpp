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

QNdefNfcSmartPosterRecord::QNdefNfcSmartPosterRecord()
    : QNdefRecord(QNdefRecord::NfcRtd, "Sp"),
      m_uri(NULL), m_action(NULL), m_size(NULL), m_type(NULL)
{
    setPayload(QByteArray(0, char(0)));
}

QNdefNfcSmartPosterRecord::QNdefNfcSmartPosterRecord(const QNdefRecord &other)
    : QNdefRecord(other, QNdefRecord::NfcRtd, "Sp"),
      m_uri(NULL), m_action(NULL), m_size(NULL), m_type(NULL)
{
    // Need to set payload again to create internal structure
    setPayload(other.payload());
}

QNdefNfcSmartPosterRecord::~QNdefNfcSmartPosterRecord()
{
    cleanup();
}

void QNdefNfcSmartPosterRecord::cleanup()
{
    // Clean-up existing internal structure
    m_titleList.clear();
    if(m_uri) delete m_uri;
    if(m_action) delete m_action;
    m_iconList.clear();
    if(m_size) delete m_size;
    if(m_type) delete m_type;
}

void QNdefNfcSmartPosterRecord::setPayload(const QByteArray &payload)
{
    QNdefRecord::setPayload(payload);

    cleanup();

    if(!payload.isEmpty()) {
        // Create new structure
        QNdefMessage message = QNdefMessage::fromByteArray(payload);

        // Iterate through all the records contained in the payload's message.
        for (QList<QNdefRecord>::const_iterator iter = message.begin(); iter != message.end(); iter++) {
            QNdefRecord record = *iter;

            // Title
            if (record.isRecordType<QNdefNfcTextRecord>()) {
                addTitleInternal(record);
            }

            // URI
            else if (record.isRecordType<QNdefNfcUriRecord>()) {
                m_uri = new QNdefNfcUriRecord(record);
            }

            // Action
            else if (record.isRecordType<QNdefNfcActRecord>()) {
                m_action = new QNdefNfcActRecord(record);
            }

            // Icon
            else if (record.isRecordType<QNdefNfcIconRecord>()) {
                addIconInternal(record);
            }

            // Size
            else if (record.isRecordType<QNdefNfcSizeRecord>()) {
                m_size = new QNdefNfcSizeRecord(record);
            }

            // Type
            else if (record.isRecordType<QNdefNfcTypeRecord>()) {
                m_type = new QNdefNfcTypeRecord(record);
            }
        }
    }
}

void QNdefNfcSmartPosterRecord::convertToPayload()
{
    QNdefMessage message;

    // Title
    for(int t=0; t<titleCount(); t++)
        message.append(titleRecord(t));

    // URI
    if(m_uri)
        message.append(*m_uri);

    // Action
    if(m_action)
        message.append(*m_action);

    // Icon
    for(int i=0; i<iconCount(); i++)
        message.append(iconRecord(i));

    // Size
    if(m_size)
        message.append(*m_size);

    // Type
    if(m_type)
        message.append(*m_type);

    QNdefRecord::setPayload(message.toByteArray());
}

/*!
    Returns true if the smart poster record contains a title record encoded in \a locale. If \a locale
    is empty then true is returned if at least one title is present regardless of \a locale, otherwise
    false is returned.
 */
bool QNdefNfcSmartPosterRecord::hasTitle(const QString &locale) const
{
    for (int i = 0; i < m_titleList.length(); ++i) {
        const QNdefNfcTextRecord &text = m_titleList[i];

        if (locale.isEmpty() || text.locale() == locale)
            return true;
    }

    return false;
}

/*!
    Returns true if the smart poster record contains an action record, otherwise false.
 */
bool QNdefNfcSmartPosterRecord::hasAction() const
{
    return m_action != NULL;
}

/*!
    Returns true if the smart poster record contains an icon record, otherwise false is returned.
 */
bool QNdefNfcSmartPosterRecord::hasIcon(const QByteArray &mimetype) const
{
    for (int i = 0; i < m_iconList.length(); ++i) {
        const QNdefNfcIconRecord &icon = m_iconList[i];

        if (mimetype.isEmpty() || icon.type() == mimetype)
            return true;
    }

    return false;
}

/*!
    Returns true if the smart poster record contains a size record, otherwise false.
 */
bool QNdefNfcSmartPosterRecord::hasSize() const
{
    return m_size != NULL;
}

/*!
    Returns true if the smart poster record contains a type record, otherwise false.
 */
bool QNdefNfcSmartPosterRecord::hasTypeInfo() const
{
    return m_type != NULL;
}

int QNdefNfcSmartPosterRecord::titleCount() const
{
    return m_titleList.length();
}

QNdefNfcTextRecord QNdefNfcSmartPosterRecord::titleRecord(const int index) const
{
    if (index >= 0 && index < m_titleList.length())
        return m_titleList[index];

    return QNdefNfcTextRecord();
}

/*!
    Returns the title from the title record encoded in \a locale if available. If \a locale
    is empty then the title from the first available record is returned regardless of \a locale. In all other
    cases an empty string is returned.
 */
QString QNdefNfcSmartPosterRecord::title(const QString &locale) const
{
    for (int i = 0; i < m_titleList.length(); ++i) {
        const QNdefNfcTextRecord &text = m_titleList[i];

        if (locale.isEmpty() || text.locale() == locale)
            return text.text();
    }

    return QString();
}

QList<QNdefNfcTextRecord> QNdefNfcSmartPosterRecord::titleRecords() const
{
    return m_titleList;
}

bool QNdefNfcSmartPosterRecord::addTitleInternal(const QNdefNfcTextRecord &text)
{
    for (int i = 0; i < m_titleList.length(); ++i) {
        const QNdefNfcTextRecord &rec = m_titleList[i];

        if (rec.locale() == text.locale())
            return false;
    }

    m_titleList.append(text);
    return true;
}

/*!
    Adds a new title record from the data contained in the text record \a text.
    The method inserts the record and returns true if the text record uses a locale that is not already present in the smart poster,
    otherwise the record is not added and false is returned. Returns true if the text record does not specify a locale already present
    in the smart poster, otherwise false
 */
bool QNdefNfcSmartPosterRecord::addTitle(const QNdefNfcTextRecord &text)
{
    bool status = addTitleInternal(text);

    // Convert to payload
    convertToPayload();

    return status;
}

/*!
    Adds a new title record containing the title \a text using locale \a locale and encoded in \a encoding.
    The method inserts the record and returns true if the smart poster does not already contain a title encoded in \a locale,
    otherwise the title is not added and false is returned. Returns true if the smart poster does not already contain a title encoded
    in \a locale, otherwise false
 */
bool QNdefNfcSmartPosterRecord::addTitle(const QString &text, const QString &locale, QNdefNfcTextRecord::Encoding encoding)
{
    QNdefNfcTextRecord rec;
    rec.setText(text);
    rec.setLocale(locale);
    rec.setEncoding(encoding);

    return addTitle(rec);
}

bool QNdefNfcSmartPosterRecord::removeTitle(const QNdefNfcTextRecord &text)
{
    bool status = false;

    for (int i = 0; i < m_titleList.length(); ++i) {
        const QNdefNfcTextRecord &rec = m_titleList[i];

        if (rec.text() == text.text() && rec.locale() == text.locale() && rec.encoding() == text.encoding()) {
            m_titleList.removeAt(i);
            status = true;
            break;
        }
    }

    // Convert to payload
    convertToPayload();

    return status;
}

bool QNdefNfcSmartPosterRecord::removeTitle(const QString &locale)
{
    bool status = false;

    for (int i = 0; i < m_titleList.length(); ++i) {
        const QNdefNfcTextRecord &rec = m_titleList[i];

        if (rec.locale() == locale) {
            m_titleList.removeAt(i);
            status = true;
            break;
        }
    }

    // Convert to payload
    convertToPayload();

    return status;
}

void QNdefNfcSmartPosterRecord::setTitles(const QList<QNdefNfcTextRecord> &titles)
{
    m_titleList.clear();

    for (int i = 0; i < titles.length(); ++i) {
        m_titleList.append(titles[i]);
    }

    // Convert to payload
    convertToPayload();
}

/*!
    Returns the URI from the URI record if available. Otherwise an empty URI is returned.
 */
QUrl QNdefNfcSmartPosterRecord::uri() const
{
    if(m_uri)
        return m_uri->uri();

    return QUrl();
}

QNdefNfcUriRecord QNdefNfcSmartPosterRecord::uriRecord() const
{
    if(m_uri)
        return *m_uri;

    return QNdefNfcUriRecord();
}

/*!
    Sets the URI record to the uri embedded in \a url
 */
void QNdefNfcSmartPosterRecord::setUri(const QNdefNfcUriRecord &url)
{
    if(m_uri)
        delete m_uri;

    m_uri = new QNdefNfcUriRecord(url);

    // Convert to payload
    convertToPayload();
}

/*!
    Sets the URI record to \a url
 */
void QNdefNfcSmartPosterRecord::setUri(const QUrl &url)
{
    QNdefNfcUriRecord rec;
    rec.setUri(url);

    setUri(rec);
}

/*!
    Returns the action from the action record if available. Otherwise \a UnspecifiedAction is returned.
 */
QNdefNfcSmartPosterRecord::Action QNdefNfcSmartPosterRecord::action() const
{
    if(m_action)
        return m_action->action();

    return UnspecifiedAction;
}

/*!
    Sets the action record to \a act
 */
void QNdefNfcSmartPosterRecord::setAction(Action act)
{
    if(!m_action)
        m_action = new QNdefNfcActRecord();

    m_action->setAction(act);

    // Convert to payload
    convertToPayload();
}

int QNdefNfcSmartPosterRecord::iconCount() const
{
    return m_iconList.length();
}

QNdefNfcIconRecord QNdefNfcSmartPosterRecord::iconRecord(const int index) const
{
    if (index >= 0 && index < m_iconList.length())
        return m_iconList[index];

    return QNdefNfcIconRecord();
}

QByteArray QNdefNfcSmartPosterRecord::icon(const QByteArray& mimetype) const
{
    for (int i = 0; i < m_iconList.length(); ++i) {
        const QNdefNfcIconRecord &icon = m_iconList[i];

        if (mimetype.isEmpty() || icon.type() == mimetype)
            return icon.data();
    }

    return QByteArray();
}

QList<QNdefNfcIconRecord> QNdefNfcSmartPosterRecord::iconRecords() const
{
    return m_iconList;
}

void QNdefNfcSmartPosterRecord::addIconInternal(const QNdefNfcIconRecord &icon)
{
    for (int i = 0; i < m_iconList.length(); ++i) {
        const QNdefNfcIconRecord &rec = m_iconList[i];

        if (rec.type() == icon.type())
            m_iconList.removeAt(i);
    }

    m_iconList.append(icon);
}

void QNdefNfcSmartPosterRecord::addIcon(const QNdefNfcIconRecord &icon)
{
    addIconInternal(icon);

    // Convert to payload
    convertToPayload();
}

void QNdefNfcSmartPosterRecord::addIcon(const QByteArray &type, const QByteArray &data)
{
    QNdefNfcIconRecord rec;
    rec.setType(type);
    rec.setData(data);

    return addIcon(rec);
}

bool QNdefNfcSmartPosterRecord::removeIcon(const QNdefNfcIconRecord &icon)
{
    bool status = false;

    for (int i = 0; i < m_iconList.length(); ++i) {
        const QNdefNfcIconRecord &rec = m_iconList[i];

        if (rec.type() == icon.type() && rec.data() == icon.data()) {
            m_iconList.removeAt(i);
            status = true;
            break;
        }
    }

    // Convert to payload
    convertToPayload();

    return status;
}

bool QNdefNfcSmartPosterRecord::removeIcon(const QByteArray &type)
{
    bool status = false;

    for (int i = 0; i < m_iconList.length(); ++i) {
        const QNdefNfcIconRecord &rec = m_iconList[i];

        if (rec.type() == type) {
            m_iconList.removeAt(i);
            status = true;
            break;
        }
    }

    // Convert to payload
    convertToPayload();

    return status;
}

void QNdefNfcSmartPosterRecord::setIcons(const QList<QNdefNfcIconRecord> &icons)
{
    m_iconList.clear();

    for (int i = 0; i < icons.length(); ++i) {
        m_titleList.append(icons[i]);
    }

    // Convert to payload
    convertToPayload();
}

/*!
    Returns the size from the size record if available. Otherwise returns 0.
 */
quint32 QNdefNfcSmartPosterRecord::size() const
{
    if(m_size)
        return m_size->size();

    return 0;
}

/*!
    Sets the size record to \a size
 */
void QNdefNfcSmartPosterRecord::setSize(quint32 size)
{
    if(!m_size)
        m_size = new QNdefNfcSizeRecord();

    m_size->setSize(size);

    // Convert to payload
    convertToPayload();
}

/*!
    Returns the type from the type record if available. Otherwise returns an empty byte array.
 */
QByteArray QNdefNfcSmartPosterRecord::typeInfo() const
{
    if(m_type)
        return m_type->typeInfo();

    return QByteArray();
}

/*!
    Sets the type record to \a type
 */
void QNdefNfcSmartPosterRecord::setTypeInfo(const QByteArray &type)
{
    if(m_type)
        delete m_type;

    m_type = new QNdefNfcTypeRecord();
    m_type->setTypeInfo(type);

    // Convert to payload
    convertToPayload();
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

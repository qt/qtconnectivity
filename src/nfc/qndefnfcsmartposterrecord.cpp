#include <qndefnfcsmartposterrecord.h>
#include <qndefnfcsmartposterrecord_p.h>
#include <qndefmessage.h>
#include <qndefnfcurirecord.h>

#include <QtCore/QString>
#include <QtCore/QUrl>

QTNFC_BEGIN_NAMESPACE

/*!
 * @class QNdefNfcSmartPosterRecord
 * @brief The QNdefNfcSmartPosterRecord class provides an NFC RTD-SmartPoster.
 * @ingroup connectivity-nfc
 * @since 1.2
 *
 * @xmlonly
 * <apigrouping group="Device and Communication/NFC"/>
 * @endxmlonly    
 *  
 * RTD-SmartPoster encapsulates a Smart Poster.
 */

/*!
 * @enum QNdefNfcSmartPosterRecord::ActionValue
 * This enum describes the course of action that a device should take with the content.
 */
 
/*!
 * @var QNdefNfcSmartPosterRecord::ActionValue QNdefNfcSmartPosterRecord::Unset
 * The action is not defined.
 * @var QNdefNfcSmartPosterRecord::ActionValue QNdefNfcSmartPosterRecord::Do
 * Do the action (send the SMS, launch the browser, make the telephone call).
 * @var QNdefNfcSmartPosterRecord::ActionValue QNdefNfcSmartPosterRecord::Save
 * Save for later (store the SMS in INBOX, put the URI in a bookmark, save the telephone number in contacts).
 * @var QNdefNfcSmartPosterRecord::ActionValue QNdefNfcSmartPosterRecord::Open
 * Open for editing (open an SMS in the SMS editor, open the URI in a URI editor, open the telephone number for editing).
 */

/*!
 *  @return Returns true if the smart poster record contains a title record encoded in @c locale. If @c locale
 *  is empty then true is returned if at least one title is present regardless of @c locale, otherwise
 *  false is returned.
 *  @param locale The locale to match with the title record
 */
bool QNdefNfcSmartPosterRecord::hasTitle(const QString& locale) const {
    const QByteArray p = payload();

    if (p.isEmpty())
        return false;

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcTextRecord>()) {
            QNdefNfcTextRecord text(rec);

            if (text.locale() == locale || locale.isEmpty()) {
                return true;
            }
        }
    }

    return false;
}

/*!
 *  @return Returns true if the smart poster record contains an action record, otherwise false.
 */
bool QNdefNfcSmartPosterRecord::hasAction() const {
    const QByteArray p = payload();

    if (p.isEmpty())
        return false;

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcActRecord>()) {
            return true;
        }
    }

    return false;
}

/*!
 *  @return Returns true if the smart poster record contains an icon record with mimetype @c mimetype. If @c mimetype
 *  is empty then true is returned if at least one icon is present regardless of @c mimetype, otherwise
 *  false is returned.
 *  @param mimetype The MIME type that encodes the icon's image type
 */
bool QNdefNfcSmartPosterRecord::hasIcon(const QByteArray& mimetype) const {
    const QByteArray p = payload();

    if (p.isEmpty())
        return false;

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcIconRecord>()) {
            QNdefNfcIconRecord icon(rec);

            if (icon.type() == mimetype || mimetype.isEmpty()) {
                return true;
            }
        }
    }

    return false;
}

/*!
 *  @return Returns true if the smart poster record contains a size record, otherwise false.
 */
bool QNdefNfcSmartPosterRecord::hasSize() const {
    const QByteArray p = payload();

    if (p.isEmpty())
        return false;

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcSizeRecord>()) {
            return true;
        }
    }

    return false;
}

/*!
 *  @return Returns true if the smart poster record contains a type record, otherwise false.
 */
bool QNdefNfcSmartPosterRecord::hasTypeInfo() const {
    const QByteArray p = payload();

    if (p.isEmpty())
        return false;

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcTypeRecord>()) {
            return true;
        }
    }

    return false;
}

/*!
 *  @return Returns the title from the title record encoded in @c locale if available. If @c locale
 *  is empty then the title from the first available record is returned regardless of @c locale. In all other
 *  cases an empty string is returned.
 *  @param locale The locale to match with the title record
 */
QString QNdefNfcSmartPosterRecord::title(const QString& locale) const {
    const QByteArray p = payload();

    if (p.isEmpty())
        return QString();

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcTextRecord>()) {
            QNdefNfcTextRecord text(rec);

            if (text.locale() == locale || locale.isEmpty()) {
                return text.text();
            }
        }
    }

    return QString();
}

/*!
 *  @return Returns the locale from the title record encoded in @c locale if available. If @c locale
 *  is empty then the locale from the first available record is returned regardless of @c locale. In all other
 *  cases an empty string is returned.
 *  @param locale The locale to match with the title record
 */
QString QNdefNfcSmartPosterRecord::titleLocale(const QString& locale) const {
    const QByteArray p = payload();

    if (p.isEmpty())
        return QString();

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcTextRecord>()) {
            QNdefNfcTextRecord text(rec);

            if (text.locale() == locale || locale.isEmpty()) {
                return text.locale();
            }
        }
    }

    return QString();
}

/*!
 *  @return Returns the encoding from the title record encoded in @c locale if available. If @c locale
 *  is empty then the encoding from the first available record is returned regardless of @c locale. In all other
 *  cases @c QNdefNfcTextRecord::Utf8 is returned.
 *  @param locale The locale to match with the title record
 */
QNdefNfcTextRecord::Encoding QNdefNfcSmartPosterRecord::titleEncoding(const QString& locale) const {
    const QByteArray p = payload();

    if (p.isEmpty())
        return QNdefNfcTextRecord::Utf8;

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcTextRecord>()) {
            QNdefNfcTextRecord text(rec);

            if (text.locale() == locale || locale.isEmpty()) {
                return text.encoding();
            }
        }
    }

    return QNdefNfcTextRecord::Utf8;
}

/*!
 *  @brief Adds a new title record containing the title @c text using locale @c locale and encoded in @c encoding.
 *  The method inserts the record and returns true if the smart poster does not already contain a title encoded in @c locale,
 *  otherwise the title is not added and false is returned.
 *  @param text The text of the title
 *  @param locale The locale of the title
 *  @param encoding The encoding of the title
 *  @return Returns true if the smart poster does not already contain a title encoded in @c locale, otherwise false
 */
bool QNdefNfcSmartPosterRecord::addTitle(const QString& text, const QString& locale, QNdefNfcTextRecord::Encoding encoding) {
    const QByteArray p = payload();
    QNdefMessage msg;

    if (!p.isEmpty()) {
        msg = QNdefMessage::fromByteArray(p);

        for (int i = 0; i < msg.length(); i++) {
            const QNdefRecord& rec = msg.at(i);

            if (rec.isRecordType<QNdefNfcTextRecord>()) {
                QNdefNfcTextRecord text(rec);

                if (text.locale() == locale) {
                    return false;
                }
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

/*!
 *  @return Returns the URI from the URI record if available. Otherwise an empty URI is returned.
 */
QUrl QNdefNfcSmartPosterRecord::uri() const {
    const QByteArray p = payload();

    if (p.isEmpty())
        return QUrl();

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcUriRecord>()) {
            return QNdefNfcUriRecord(rec).uri();
        }
    }

    return QUrl();
}

/*!
 *  @brief Sets the URI record to @c url and returns true if the smart poster
 *  does not already contain a URI record. Otherwise, the URI is not changed
 *  and false is returned.
 *  @param url The URI of the URI record
 *  @return Returns true if the smart poster does not already contain a URI record, otherwise false
 */
bool QNdefNfcSmartPosterRecord::setUri(const QUrl& url) {
    const QByteArray p = payload();
    QNdefMessage msg;

    if (!p.isEmpty()) {
        msg = QNdefMessage::fromByteArray(p);

        for (int i = 0; i < msg.length(); i++) {
            const QNdefRecord& rec = msg.at(i);

            if (rec.isRecordType<QNdefNfcUriRecord>()) {
                return false;
            }
        }
    }

    QNdefNfcUriRecord rec;
    rec.setUri(url);

    msg.append(rec);

    setPayload(msg.toByteArray());
    return true;
}

/*!
 *  @return Returns the action from the action record if available. Otherwise @c ActionValue::Unset is returned.
 */
QNdefNfcSmartPosterRecord::ActionValue QNdefNfcSmartPosterRecord::action() const {
    const QByteArray p = payload();

    if (p.isEmpty())
        return Unset;

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcActRecord>()) {
            return QNdefNfcActRecord(rec).action();
        }
    }

    return Unset;
}

/*!
 *  @brief Sets the action record to @c act and returns true if the smart poster
 *  does not already contain an action record. Otherwise, the action record
 *  is not changed and false is returned.
 *  @param act The @c ActionValue type
 *  @return Returns true if the smart poster record does not already contain an action record, otherwise false
 */
bool QNdefNfcSmartPosterRecord::setAction(ActionValue act) {
    const QByteArray p = payload();
    QNdefMessage msg;

    if (!p.isEmpty()) {
        msg = QNdefMessage::fromByteArray(p);

        for (int i = 0; i < msg.length(); i++) {
            const QNdefRecord& rec = msg.at(i);

            if (rec.isRecordType<QNdefNfcActRecord>()) {
                return false;
            }
        }
    }

    QNdefNfcActRecord rec;
    rec.setAction(act);

    msg.append(rec);

    setPayload(msg.toByteArray());
    return true;
}

/*!
 *  @return Returns the icon data from the first icon record matching @c mimetype if available.
 *  Otherwise an empty byte array is returned.
 *  @param mimetype The MIME type that encodes the icon's image type
 */
QByteArray QNdefNfcSmartPosterRecord::icon(const QByteArray& mimetype) const {
    const QByteArray p = payload();

    if (p.isEmpty())
        return QByteArray();

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcIconRecord>()) {
            QNdefNfcIconRecord icon(rec);

            if (icon.type() == mimetype) {
                return icon.data();
            }
        }
    }

    return QByteArray();
}

/*!
 *  @return Returns the icon type from the first icon record matching @c mimetype if available. Otherwise an empty byte array is returned.
 *  @param mimetype The MIME type that encodes the icon's image type
 */
QByteArray QNdefNfcSmartPosterRecord::iconType(const QByteArray& mimetype) const {
    const QByteArray p = payload();

    if (p.isEmpty())
        return QByteArray();

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcIconRecord>()) {
            QNdefNfcIconRecord icon(rec);

            if (icon.type() == mimetype) {
                return icon.type();
            }
        }
    }

    return QByteArray();
}

/*!
 *  @return Returns the icon data from the icon record at position @c index if available. Otherwise an empty byte array is returned.
 *  @param index The index position of the icon record
 */
QByteArray QNdefNfcSmartPosterRecord::icon(int index) const {
    const QByteArray p = payload();

    if (p.isEmpty()) {
        return QByteArray();
    }

    QNdefMessage msg = QNdefMessage::fromByteArray(p);
    int index_found = -1;

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcIconRecord>()) {
            index_found++;

            if (index == index_found) {
                return QNdefNfcIconRecord(rec).data();
            }
        }
    }

    return QByteArray();
}

/*!
 *  @return Returns the icon type from the icon record at position @c index if available. Otherwise an empty byte array is returned.
 *  @param index The index position of the icon record
 */
QByteArray QNdefNfcSmartPosterRecord::iconType(int index) const {
    const QByteArray p = payload();

    if (p.isEmpty()) {
        return QByteArray();
    }

    QNdefMessage msg = QNdefMessage::fromByteArray(p);
    int index_found = -1;

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcIconRecord>()) {
            index_found++;

            if (index == index_found) {
                return QNdefNfcIconRecord(rec).type();
            }
        }
    }

    return QByteArray();
}

/*!
 *  @return Returns the total number of icon records contained in the smart poster record.
 */
quint32 QNdefNfcSmartPosterRecord::iconCount() const {
    const QByteArray p = payload();

    if (p.isEmpty()) {
        return 0;
    }

    QNdefMessage msg = QNdefMessage::fromByteArray(p);
    quint32 count = 0;

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcIconRecord>()) {
            count++;
        }
    }

    return count;
}

/*!
 *  @brief Adds a new icon record with type @c type and data @c data.
 *  @param type The MIME type that encodes the icon's image type
 *  @param data The image data encoded in the format corresponding to its type
 */
void QNdefNfcSmartPosterRecord::addIcon(const QByteArray& type, const QByteArray& data) {
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

/*!
 *  @return Returns the size from the size record if available. Otherwise returns 0.
 */
quint32 QNdefNfcSmartPosterRecord::size() const {
    const QByteArray p = payload();

    if (p.isEmpty())
        return 0;

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcSizeRecord>()) {
            return QNdefNfcSizeRecord(rec).size();
        }
    }

    return 0;
}

/*!
 *  @brief Sets the size record to @c size if the smart poster does not already
 *  contain a size record.
 *  @param size The size of the size record
 *  @return Returns true if the smart poster does not already contain a size record,
 *  otherwise false.
 */
bool QNdefNfcSmartPosterRecord::setSize(quint32 size) {
    const QByteArray p = payload();
    QNdefMessage msg;

    if (!p.isEmpty()) {
        msg = QNdefMessage::fromByteArray(p);

        for (int i = 0; i < msg.length(); i++) {
            const QNdefRecord& rec = msg.at(i);

            if (rec.isRecordType<QNdefNfcSizeRecord>()) {
                return false;
            }
        }
    }

    QNdefNfcSizeRecord rec;
    rec.setSize(size);
    msg.append(rec);

    setPayload(msg.toByteArray());
    return true;
}

/*!
 *  @return Returns the type from the type record if available. Otherwise returns an empty byte array.
 */
QByteArray QNdefNfcSmartPosterRecord::typeInfo() const {
    const QByteArray p = payload();

    if (p.isEmpty())
        return QByteArray();

    QNdefMessage msg = QNdefMessage::fromByteArray(p);

    for (int i = 0; i < msg.length(); i++) {
        const QNdefRecord& rec = msg.at(i);

        if (rec.isRecordType<QNdefNfcTypeRecord>()) {
            return QNdefNfcTypeRecord(rec).typeInfo();
        }
    }

    return QByteArray();
}

/*!
 *  @brief Sets the type record to @c type if the smart poster does not already contain a type record.
 *  @param type The type of the type record
 *  @return Returns true if the smart poster does not already contain a type record,
 *  otherwise false.
 */
bool QNdefNfcSmartPosterRecord::setTypeInfo(const QByteArray& type) {
    const QByteArray p = payload();
    QNdefMessage msg;

    if (!p.isEmpty()) {
        msg = QNdefMessage::fromByteArray(p);

        for (int i = 0; i < msg.length(); i++) {
            const QNdefRecord& rec = msg.at(i);

            if (rec.isRecordType<QNdefNfcTypeRecord>()) {
                return false;
            }
        }
    }

    QNdefNfcTypeRecord rec;
    rec.setTypeInfo(type);
    msg.append(rec);

    setPayload(msg.toByteArray());
    return true;
}

/*!
    @internal
*/
void QNdefNfcActRecord::setAction(QNdefNfcSmartPosterRecord::ActionValue actionValue) {
    QByteArray data;
    data[0] = actionValue;

    setPayload(data);
}

/*!
    @internal
*/
QNdefNfcSmartPosterRecord::ActionValue QNdefNfcActRecord::action() const {
    const QByteArray p = payload();
    QNdefNfcSmartPosterRecord::ActionValue value =
            QNdefNfcSmartPosterRecord::Unset;

    if (!p.isEmpty())
        value = QNdefNfcSmartPosterRecord::ActionValue(p[0]);

    return value;
}

/*!
    @internal
*/
void QNdefNfcIconRecord::setData(const QByteArray& data) {
    setPayload(data);
}

/*!
    @internal
*/
QByteArray QNdefNfcIconRecord::data() const {
    return payload();
}

/*!
    @internal
*/
void QNdefNfcSizeRecord::setSize(quint32 size) {
    QByteArray data;

    data[0] = (int) ((size & 0xFF000000) >> 24);
    data[1] = (int) ((size & 0x00FF0000) >> 16);
    data[2] = (int) ((size & 0x0000FF00) >> 8);
    data[3] = (int) ((size & 0x000000FF));

    setPayload(data);
}

/*!
    @internal
*/
quint32 QNdefNfcSizeRecord::size() const {
    const QByteArray p = payload();

    if (p.isEmpty())
        return 0;

    return ((p[0] << 24) & 0xFF000000) + ((p[1] << 16) & 0x00FF0000)
            + ((p[2] << 8) & 0x0000FF00) + (p[3] & 0x000000FF);
}

/*!
    @internal
*/
void QNdefNfcTypeRecord::setTypeInfo(const QByteArray &type) {
    setPayload(type);
}

/*!
    @internal
*/
QByteArray QNdefNfcTypeRecord::typeInfo() const {
    return payload();
}

QTNFC_END_NAMESPACE

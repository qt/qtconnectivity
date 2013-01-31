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

#ifndef QNDEFNFCSMARTPOSTERRECORD_H
#define QNDEFNFCSMARTPOSTERRECORD_H

#include <QtCore/QList>
#include <QtNfc/qnfcglobal.h>
#include <QtNfc/QNdefRecord>
#include <qndefnfctextrecord.h>
#include <qndefnfcurirecord.h>

QT_FORWARD_DECLARE_CLASS(QUrl)

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

QT_FORWARD_DECLARE_CLASS(QNdefNfcActRecord)
QT_FORWARD_DECLARE_CLASS(QNdefNfcSizeRecord)
QT_FORWARD_DECLARE_CLASS(QNdefNfcTypeRecord)

#define Q_DECLARE_ISRECORDTYPE_FOR_MIME_NDEF_RECORD(className) \
    QTNFC_BEGIN_NAMESPACE \
    template<> inline bool QNdefRecord::isRecordType<className>() const\
    { \
        return (typeNameFormat() == QNdefRecord::Mime); \
    } \
    QTNFC_END_NAMESPACE

#define Q_DECLARE_MIME_NDEF_RECORD(className, initialPayload) \
    className() : QNdefRecord(QNdefRecord::Mime, "") { setPayload(initialPayload); } \
    className(const QNdefRecord &other) : QNdefRecord(other, QNdefRecord::Mime) { }

class Q_NFC_EXPORT QNdefNfcIconRecord : public QNdefRecord
{
public:
    Q_DECLARE_MIME_NDEF_RECORD(QNdefNfcIconRecord, QByteArray(0, char(0)))

    void setData(const QByteArray &data);
    QByteArray data() const;
};

class Q_NFC_EXPORT QNdefNfcSmartPosterRecord : public QNdefRecord
{
public:
    enum Action {
        UnspecifiedAction = -1,
        DoAction = 0,
        SaveAction = 1,
        EditAction = 2
    };

    QNdefNfcSmartPosterRecord();
    QNdefNfcSmartPosterRecord(const QNdefRecord &other);
    virtual ~QNdefNfcSmartPosterRecord();

    void setPayload(const QByteArray &payload);

    bool hasTitle(const QString &locale = QString()) const;
    bool hasAction() const;
    bool hasIcon(const QByteArray &mimetype = QByteArray()) const;
    bool hasSize() const;
    bool hasTypeInfo() const;

    int titleCount() const;
    QNdefNfcTextRecord titleRecord(const int index) const;
    QString title(const QString &locale = QString()) const;
    QList<QNdefNfcTextRecord> titleRecords() const;

    bool addTitle(const QNdefNfcTextRecord &text);
    bool addTitle(const QString &text, const QString &locale, QNdefNfcTextRecord::Encoding encoding);
    bool removeTitle(const QNdefNfcTextRecord &text);
    bool removeTitle(const QString &locale);
    void setTitles(const QList<QNdefNfcTextRecord> &titles);

    QUrl uri() const;
    QNdefNfcUriRecord uriRecord() const;
    void setUri(const QNdefNfcUriRecord &url);
    void setUri(const QUrl &url);

    Action action() const;
    void setAction(Action act);

    int iconCount() const;
    QNdefNfcIconRecord iconRecord(const int index) const;
    QByteArray icon(const QByteArray& mimetype = QByteArray()) const;

    QList<QNdefNfcIconRecord> iconRecords() const;

    void addIcon(const QNdefNfcIconRecord &icon);
    void addIcon(const QByteArray &type, const QByteArray &data);
    bool removeIcon(const QNdefNfcIconRecord &icon);
    bool removeIcon(const QByteArray &type);
    void setIcons(const QList<QNdefNfcIconRecord> &icons);

    quint32 size() const;
    void setSize(quint32 size);

    QByteArray typeInfo() const;
    void setTypeInfo(const QByteArray &type);

private:
    QList<QNdefNfcTextRecord> m_titleList;
    QNdefNfcUriRecord *m_uri;
    QNdefNfcActRecord *m_action;
    QList<QNdefNfcIconRecord> m_iconList;
    QNdefNfcSizeRecord *m_size;
    QNdefNfcTypeRecord *m_type;

    void cleanup();
    void convertToPayload();

    bool addTitleInternal(const QNdefNfcTextRecord &text);
    void addIconInternal(const QNdefNfcIconRecord &icon);
};

QTNFC_END_NAMESPACE

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcSmartPosterRecord, QNdefRecord::NfcRtd, "Sp")
Q_DECLARE_ISRECORDTYPE_FOR_MIME_NDEF_RECORD(QNdefNfcIconRecord)

QT_END_HEADER

#endif // QNDEFNFCSMARTPOSTERRECORD_H

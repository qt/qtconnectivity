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
 ** Software or, alternatively, in accordance with the terms contained in13	
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

#include <QtNfc/qnfcglobal.h>
#include <QtNfc/QNdefRecord>
#include <qndefnfctextrecord.h>

QT_FORWARD_DECLARE_CLASS(QUrl)

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

class Q_NFC_EXPORT QNdefNfcSmartPosterRecord : public QNdefRecord
{
public:
    enum Action {
        UnspecifiedAction = -1,
        DoAction = 0,
        SaveAction = 1,
        EditAction = 2
    };

    Q_DECLARE_NDEF_RECORD(QNdefNfcSmartPosterRecord, QNdefRecord::NfcRtd, "Sp", QByteArray(0, char(0)))

    bool hasTitle(const QString &locale = QString()) const;
    bool hasAction() const;
    bool hasIcon(const QByteArray &mimetype = QByteArray()) const;
    bool hasSize() const;
    bool hasTypeInfo() const;

    QString title(const QString &locale = QString()) const;
    QString titleLocale(const QString &locale = QString()) const;
    QNdefNfcTextRecord::Encoding titleEncoding(const QString &locale = QString()) const;

    bool addTitle(const QString &text, const QString &locale, QNdefNfcTextRecord::Encoding encoding);
    bool removeTitle(const QString &locale);
    QStringList titleLocales();

    QUrl uri() const;
    bool setUri(const QUrl &url);

    Action action() const;
    bool setAction(Action act);

    QByteArray icon(const QByteArray& mimetype) const;
    QByteArray iconType(const QByteArray& mimetype) const;

    QByteArray icon(int index) const;
    QByteArray iconType(int index) const;
    quint32 iconCount() const;
    QList<QByteArray> iconTypes() const;

    void addIcon(const QByteArray &type, const QByteArray &data);
    void removeIcon(const QByteArray &type);

    quint32 size() const;
    bool setSize(quint32 size);

    QByteArray typeInfo() const;
    bool setTypeInfo(const QByteArray &type);
};

QTNFC_END_NAMESPACE

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcSmartPosterRecord, QNdefRecord::NfcRtd, "Sp")

QT_END_HEADER

#endif // QNDEFNFCSMARTPOSTERRECORD_H

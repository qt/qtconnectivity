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

#ifndef QNDEFNFCSMARTPOSTER_H
#define QNDEFNFCSMARTPOSTER_H

#include <QtNfc/QNdefMessage>
#include <QtNfc/QNdefNfcTextRecord>
#include <QtNfc/QNdefNfcUriRecord>

QTNFC_BEGIN_NAMESPACE

class Q_NFC_EXPORT QNdefNfcSmartPoster : public QNdefMessage
{
public:
    QNdefNfcSmartPoster();
    QNdefNfcSmartPoster(const QNdefNfcSmartPoster &other);
    QNdefNfcSmartPoster(const QNdefMessage &other);

    enum Action {
        NoAction,
        SaveForLaterAction,
        OpenForEditingAction
    };

    typedef QNdefNfcTextRecord TitleRecord;
    typedef QNdefNfcUriRecord UriRecord;
    typedef QNdefRecord IconRecord;

    class ActionRecord : public QNdefRecord
    {
    public:
        Q_DECLARE_NDEF_RECORD(ActionRecord, QNdefRecord::NfcRtd, "act", QByteArray(1, char(0)))

        Action action() const;
        void setAction(Action action);
    };

    class SizeRecord : public QNdefRecord
    {
    public:
        Q_DECLARE_NDEF_RECORD(SizeRecord, QNdefRecord::NfcRtd, "s", QByteArray(4, char(0)))

        quint32 size() const;
        void setSize(quint32 size);
    };

    class TypeRecord : public QNdefRecord
    {
    public:
        Q_DECLARE_NDEF_RECORD(TypeRecord, QNdefRecord::NfcRtd, "t", QByteArray())

        QByteArray mimeType() const;
        void setMimeType(const QByteArray &mimeType);
    };

    bool isValid() const;

    void setTitle(const QString &title, const QString &locale = QString());
    QString title(const QString &locale = QString()) const;

    void setUri(const QUrl &uri);
    QUrl uri() const;

    //void setIcon();
    //QIcon icon() const;

    void setDefaultAction(Action action);
    Action defaultAction() const;

    void setContentSize(quint32 size);
    quint32 contentSize() const;

    void setContentType(const QByteArray &mimeType);
    QByteArray contentType() const;
};

QTNFC_END_NAMESPACE

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcSmartPoster::ActionRecord, QNdefRecord::NfcRtd, "act")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcSmartPoster::SizeRecord, QNdefRecord::NfcRtd, "s")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcSmartPoster::TypeRecord, QNdefRecord::NfcRtd, "t")

#endif // QNDEFNFCSMARTPOSTER_H

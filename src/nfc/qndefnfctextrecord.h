/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QNDEFNFCTEXTRECORD_H
#define QNDEFNFCTEXTRECORD_H

#include <QtNfc/qtnfcglobal.h>
#include <QtNfc/QNdefRecord>

QT_BEGIN_NAMESPACE

class Q_NFC_EXPORT QNdefNfcTextRecord : public QNdefRecord
{
public:
#ifndef Q_QDOC
    Q_DECLARE_NDEF_RECORD(QNdefNfcTextRecord, QNdefRecord::NfcRtd, "T", QByteArray(1, char(0)))
#else
    QNdefNfcTextRecord();
    QNdefNfcTextRecord(const QNdefRecord& other);
#endif

    QString locale() const;
    void setLocale(const QString &locale);

    QString text() const;
    void setText(const QString text);

    enum Encoding {
        Utf8,
        Utf16
    };

    Encoding encoding() const;
    void setEncoding(Encoding encoding);
};

QT_END_NAMESPACE

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcTextRecord, QNdefRecord::NfcRtd, "T")

#endif // QNDEFNFCTEXTRECORD_H

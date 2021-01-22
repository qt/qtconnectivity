/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
****************************************************************************/

#ifndef QNDEFNFCURIRECORD_H
#define QNDEFNFCURIRECORD_H

#include <QtNfc/qtnfcglobal.h>
#include <QtNfc/QNdefRecord>

QT_FORWARD_DECLARE_CLASS(QUrl)

QT_BEGIN_NAMESPACE

class Q_NFC_EXPORT QNdefNfcUriRecord : public QNdefRecord
{
public:
#ifndef Q_QDOC
    Q_DECLARE_NDEF_RECORD(QNdefNfcUriRecord, QNdefRecord::NfcRtd, "U", QByteArray(0, char(0)))
#else
    QNdefNfcUriRecord();
    QNdefNfcUriRecord(const QNdefRecord& other);
#endif

    QUrl uri() const;
    void setUri(const QUrl &uri);
};

QT_END_NAMESPACE

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcUriRecord, QNdefRecord::NfcRtd, "U")

#endif // QNDEFNFCURIRECORD_H

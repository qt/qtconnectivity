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

#ifndef QNDEFMESSAGE_H
#define QNDEFMESSAGE_H

#include <QtCore/QVector>
#include <QtCore/QSet>
#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <QtNfc/qtnfcglobal.h>
#include <QtNfc/QNdefRecord>

QT_BEGIN_NAMESPACE

class Q_NFC_EXPORT QNdefMessage : public QList<QNdefRecord>
{
public:
    inline QNdefMessage() { }
    inline explicit QNdefMessage(const QNdefRecord &record) { append(record); }
    inline QNdefMessage(const QNdefMessage &message) : QList<QNdefRecord>(message) { }
    inline QNdefMessage(const QList<QNdefRecord> &records) : QList<QNdefRecord>(records) { }

    bool operator==(const QNdefMessage &other) const;

    QByteArray toByteArray() const;

    static QNdefMessage fromByteArray(const QByteArray &message);
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QNdefMessage)

#endif // QNDEFMESSAGE_H

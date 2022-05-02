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

#ifndef QNDEFMESSAGE_H
#define QNDEFMESSAGE_H

#include <QtCore/QSet>
#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <QtNfc/qtnfcglobal.h>
#include <QtNfc/QNdefRecord>

QT_BEGIN_NAMESPACE

// This class used to be exported exposing QList methods, see QTBUG-102367.
#if defined(QT_BUILD_NFC_LIB)
#    define Q_NFC_EXPORT_COMPAT QT6_ONLY(Q_NFC_EXPORT)
#else
#    define Q_NFC_EXPORT_COMPAT
#endif

class QNdefMessage : public QList<QNdefRecord>
{
public:
    Q_NFC_EXPORT_COMPAT
    QNdefMessage() = default;
    Q_NFC_EXPORT_COMPAT
    explicit QNdefMessage(const QNdefRecord &record) { append(record); }
    Q_NFC_EXPORT_COMPAT
    QNdefMessage(const QNdefMessage &message) = default;
    Q_NFC_EXPORT_COMPAT
    QNdefMessage(const QList<QNdefRecord> &records) : QList<QNdefRecord>(records) { }

    Q_NFC_EXPORT_COMPAT
    QNdefMessage &operator=(const QNdefMessage &other) = default;
    Q_NFC_EXPORT_COMPAT
    QNdefMessage &operator=(QNdefMessage &&other) noexcept = default;

    Q_NFC_EXPORT bool operator==(const QNdefMessage &other) const;

    Q_NFC_EXPORT QByteArray toByteArray() const;

    Q_NFC_EXPORT static QNdefMessage fromByteArray(const QByteArray &message);
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QNdefMessage)

#endif // QNDEFMESSAGE_H

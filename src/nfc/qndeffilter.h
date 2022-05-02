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

#ifndef QNDEFFILTER_H
#define QNDEFFILTER_H

#include <QtCore/QSharedDataPointer>
#include <QtNfc/qtnfcglobal.h>
#include <QtNfc/QNdefRecord>

QT_BEGIN_NAMESPACE

class QNdefMessage;

class QNdefFilterPrivate;
class Q_NFC_EXPORT QNdefFilter
{
public:
    QNdefFilter();
    QNdefFilter(const QNdefFilter &other);
    ~QNdefFilter();

    void clear();

    void setOrderMatch(bool on);
    bool orderMatch() const;

    struct Record {
        QNdefRecord::TypeNameFormat typeNameFormat;
        QByteArray type;
        unsigned int minimum;
        unsigned int maximum;
    };

    template<typename T>
    bool appendRecord(unsigned int min = 1, unsigned int max = 1);
    bool appendRecord(QNdefRecord::TypeNameFormat typeNameFormat, const QByteArray &type,
                      unsigned int min = 1, unsigned int max = 1);
    bool appendRecord(const Record &record);

    qsizetype recordCount() const;
    Record recordAt(qsizetype i) const;

    QNdefFilter &operator=(const QNdefFilter &other);

    bool match(const QNdefMessage &message) const;

private:
    QSharedDataPointer<QNdefFilterPrivate> d;
};

template <typename T>
bool QNdefFilter::appendRecord(unsigned int min, unsigned int max)
{
    T record;

    return appendRecord(record.typeNameFormat(), record.type(), min, max);
}

QT_END_NAMESPACE

#endif // QNDEFFILTER_H

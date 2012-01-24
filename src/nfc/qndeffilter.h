/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QNDEFFILTER_H
#define QNDEFFILTER_H

#include "qnfcglobal.h"

#include <QtCore/QSharedDataPointer>

#include <qndefrecord.h>

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

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
    void appendRecord(unsigned int min = 1, unsigned int max = 1);
    void appendRecord(QNdefRecord::TypeNameFormat typeNameFormat, const QByteArray &type,
                      unsigned int min = 1, unsigned int max = 1);
    void appendRecord(const Record &record);

    int recordCount() const;
    Record recordAt(int i) const;

    QNdefFilter &operator=(const QNdefFilter &other);

private:
    QSharedDataPointer<QNdefFilterPrivate> d;
};

template <typename T>
void QNdefFilter::appendRecord(unsigned int min, unsigned int max)
{
    T record;

    appendRecord(record.typeNameFormat(), record.type(), min, max);
}

QTNFC_END_NAMESPACE

QT_END_HEADER

#endif // QNDEFFILTER_H

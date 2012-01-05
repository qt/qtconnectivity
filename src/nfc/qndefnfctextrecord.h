/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QNDEFNFCTEXTRECORD_H
#define QNDEFNFCTEXTRECORD_H

#include "qnfcglobal.h"

#include <qndefrecord.h>

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

class Q_NFC_EXPORT QNdefNfcTextRecord : public QNdefRecord
{
public:
    Q_DECLARE_NDEF_RECORD(QNdefNfcTextRecord, QNdefRecord::NfcRtd, "T", QByteArray(1, char(0)))

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

QTNFC_END_NAMESPACE

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcTextRecord, QNdefRecord::NfcRtd, "T")

QT_END_HEADER

#endif // QNDEFNFCTEXTRECORD_H

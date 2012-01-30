/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#include "foorecord.h"
#include <qndefrecord.h>

//! [Declare foo record]
Q_DECLARE_NDEFRECORD(QDeclarativeNdefFooRecord, QNdefRecord::ExternalRtd, "com.example:f")
//! [Declare foo record]

//! [createFooRecord]
static inline QNdefRecord createFooRecord()
{
    QNdefRecord foo;
    foo.setTypeNameFormat(QNdefRecord::ExternalRtd);
    foo.setType("com.example:f");
    foo.setPayload(QByteArray(sizeof(int), char(0)));
    return foo;
}
//! [createFooRecord]

//! [copyFooRecord]
static inline QNdefRecord copyFooRecord(const QNdefRecord &record)
{
    if (record.typeNameFormat() != QNdefRecord::ExternalRtd)
        return createFooRecord();
    if (record.type() != "com.example:f")
        return createFooRecord();

    return record;
}
//! [copyFooRecord]

//! [Constructors]
QDeclarativeNdefFooRecord::QDeclarativeNdefFooRecord(QObject *parent)
:   QDeclarativeNdefRecord(createFooRecord(), parent)
{
}

QDeclarativeNdefFooRecord::QDeclarativeNdefFooRecord(const QNdefRecord &record, QObject *parent)
:   QDeclarativeNdefRecord(copyFooRecord(record), parent)
{
}
//! [Constructors]

QDeclarativeNdefFooRecord::~QDeclarativeNdefFooRecord()
{
}

int QDeclarativeNdefFooRecord::foo() const
{
    QByteArray payload = record().payload();

    int value = payload.at(0) << 24 |
                payload.at(1) << 16 |
                payload.at(2) << 8 |
                payload.at(3) << 0;

    return value;
}

void QDeclarativeNdefFooRecord::setFoo(int value)
{
    if (foo() == value)
        return;

    QByteArray payload;
    payload[0] = (value >> 24) & 0xff;
    payload[1] = (value >> 16) & 0xff;
    payload[2] = (value >> 8) & 0xff;
    payload[3] = (value >> 0) & 0xff;

    QNdefRecord r = record();
    r.setPayload(payload);
    setRecord(r);
    emit fooChanged();
}

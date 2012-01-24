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

#include "qdeclarativendefmimerecord_p.h"

/*!
    \qmlclass NdefMimeRecord QDeclarativeNdefMimeRecord
    \brief The NdefMimeRecord element represents an NFC MIME record.

    \ingroup connectivity-nfc
    \inmodule QtNfc

    \inherits NdefRecord

    The NdefMimeRecord element was introduced in \bold {QtNfc 5.0}.

    The NdefMimeRecord element can contain data with an associated MIME type.  The data is
    accessible from the uri in the \l {NdefMimeRecord::uri}{uri} property.
*/

/*!
    \qmlproperty string NdefMimeRecord::uri

    This property hold the URI from which the MIME data can be fetched from.  Currently this
    property returns a data url.
*/

Q_DECLARE_NDEFRECORD(QDeclarativeNdefMimeRecord, QNdefRecord::Mime, ".*")

static inline QNdefRecord createMimeRecord()
{
    QNdefRecord mimeRecord;
    mimeRecord.setTypeNameFormat(QNdefRecord::Mime);
    return mimeRecord;
}

static inline QNdefRecord castToMimeRecord(const QNdefRecord &record)
{
    if (record.typeNameFormat() != QNdefRecord::Mime)
        return createMimeRecord();
    return record;
}

QDeclarativeNdefMimeRecord::QDeclarativeNdefMimeRecord(QObject *parent)
:   QDeclarativeNdefRecord(createMimeRecord(), parent)
{
}

QDeclarativeNdefMimeRecord::QDeclarativeNdefMimeRecord(const QNdefRecord &record, QObject *parent)
:   QDeclarativeNdefRecord(castToMimeRecord(record), parent)
{
}

QDeclarativeNdefMimeRecord::~QDeclarativeNdefMimeRecord()
{
}

QString QDeclarativeNdefMimeRecord::uri() const
{
    QByteArray dataUri = "data:" + record().type() + ";base64," + record().payload().toBase64();
    return QString::fromAscii(dataUri.constData(), dataUri.length());
}

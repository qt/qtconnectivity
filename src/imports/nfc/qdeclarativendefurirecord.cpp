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

#include "qdeclarativendefurirecord_p.h"

#include <QtCore/QUrl>

/*!
    \qmlclass NdefUriRecord QDeclarativeNdefUriRecord
    \brief The NdefUriRecord type represents an NFC RTD-URI NDEF record.

    \ingroup nfc-qml
    \inmodule QtNfc

    \inherits NdefRecord

    \sa QNdefNfcUriRecord

    The NdefUriRecord type was introduced in \b {QtNfc 5.0}.
*/

/*!
    \qmlproperty string NdefUriRecord::uri

    This property hold the URI stored in this URI record.
*/

Q_DECLARE_NDEFRECORD(QDeclarativeNdefUriRecord, QNdefRecord::NfcRtd, "U")

QDeclarativeNdefUriRecord::QDeclarativeNdefUriRecord(QObject *parent)
:   QDeclarativeNdefRecord(QNdefNfcUriRecord(), parent)
{
}

QDeclarativeNdefUriRecord::QDeclarativeNdefUriRecord(const QNdefRecord &record, QObject *parent)
:   QDeclarativeNdefRecord(QNdefNfcUriRecord(record), parent)
{
}

QDeclarativeNdefUriRecord::~QDeclarativeNdefUriRecord()
{
}

QString QDeclarativeNdefUriRecord::uri() const
{
    QNdefNfcUriRecord uriRecord(record());

    return uriRecord.uri().toString();
}

void QDeclarativeNdefUriRecord::setUri(const QString &uri)
{
    QNdefNfcUriRecord uriRecord(record());

    if (uriRecord.uri() == uri)
        return;

    uriRecord.setUri(uri);
    setRecord(uriRecord);
    emit uriChanged();
}

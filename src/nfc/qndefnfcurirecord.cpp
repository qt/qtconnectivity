/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qndefnfcurirecord.h"

#include <QtCore/QString>
#include <QtCore/QUrl>

#include <QtCore/QDebug>

QTNFC_BEGIN_NAMESPACE

/*!
    \class QNdefNfcUriRecord
    \brief The QNdefNfcUriRecord class provides an NFC RTD-URI

    \ingroup connectivity-nfc
    \inmodule QtConnectivity
    \since 5.0

    RTD-URI encapsulates a URI.
*/

static const char * const abbreviations[] = {
    0,
    "http://www.",
    "https://www.",
    "http://",
    "https://",
    "tel:",
    "mailto:",
    "ftp://anonymous:anonymous@",
    "ftp://ftp.",
    "ftps://",
    "sftp://",
    "smb://",
    "nfs://",
    "ftp://",
    "dav://",
    "news:",
    "telnet://",
    "imap:",
    "rtsp://",
    "urn:",
    "pop:",
    "sip:",
    "sips:",
    "tftp:",
    "btspp://",
    "btl2cap://",
    "btgoep://",
    "tcpobex://",
    "irdaobex://",
    "file://",
    "urn:epc:id:",
    "urn:epc:tag:",
    "urn:epc:pat:",
    "urn:epc:raw:",
    "urn:epc:",
    "urn:nfc:",
};

/*!
    Returns the URI of this URI record.
*/
QUrl QNdefNfcUriRecord::uri() const
{
    QByteArray p = payload();

    if (p.isEmpty())
        return QUrl();

    quint8 code = p.at(0);
    if (code >= sizeof(abbreviations) / sizeof(*abbreviations))
        code = 0;
    p.remove(0, 1);
    if (const char *abbreviation = abbreviations[code])
        p.insert(0, abbreviation);
    return QUrl(QString::fromUtf8(p));
}

/*!
    Sets the URI of this URI record to \a uri.
*/
void QNdefNfcUriRecord::setUri(const QUrl &uri)
{
    int abbrevs = sizeof(abbreviations) / sizeof(*abbreviations);

    for (int i = 1; i < abbrevs; ++i) {
        if (uri.toString().startsWith(QLatin1String(abbreviations[i]))) {
            QByteArray p;

            p[0] = i;
            p += uri.toString().mid(qstrlen(abbreviations[i])).toUtf8();

            setPayload(p);

            return;
        }
    }

    QByteArray p;
    p[0] = 0;
    p += uri.toString().toUtf8();

    setPayload(p);
}

QTNFC_END_NAMESPACE

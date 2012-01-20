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

#include "qndeffilter.h"

#include <QtCore/QList>

QTNFC_BEGIN_NAMESPACE

/*!
    \class QNdefFilter
    \brief The QNdefFilter class provides a filter for matching NDEF messages.

    \ingroup connectivity-nfc
    \inmodule QtConnectivity
    \since 5.0

    The QNdefFilter encapsulates the structure of an NDEF message and is used by
    QNearFieldManager::registerNdefMessageHandler() to match NDEF message that have a particular
    structure.

    The following filter matches NDEF messages that contain a single smart poster record:

    \code
    QNdefFilter filter;
    filter.append(QNdefRecord::NfcRtd, "Sp");
    \endcode

    The following filter matches NDEF messages that contain a URI, a localized piece of text and an
    optional JPEG image.  The order of the records must be in the order specified:

    \code
    QNdefFilter filter;
    filter.setOrderMatch(true);
    filter.appendRecord(QNdefRecord::NfcRtd, "U");
    filter.appendRecord<QNdefNfcTextRecord>();
    filter.appendRecord(QNdefRecord::Mime, "image/jpeg", 0, 1);
    \endcode
*/

/*!
    \fn void QNdefFilter::appendRecord(unsigned int min, unsigned int max)

    Appends a record matching the template parameter to the NDEF filter.  The record must occur
    between \a min and \a max times in the NDEF message.
*/

class QNdefFilterPrivate : public QSharedData
{
public:
    QNdefFilterPrivate();

    bool orderMatching;
    QList<QNdefFilter::Record> filterRecords;
};

QNdefFilterPrivate::QNdefFilterPrivate()
:   orderMatching(false)
{
}

/*!
    Constructs a new NDEF filter.
*/
QNdefFilter::QNdefFilter()
:   d(new QNdefFilterPrivate)
{
}

/*!
    constructs a new NDEF filter that is a copy of \a other.
*/
QNdefFilter::QNdefFilter(const QNdefFilter &other)
:   d(other.d)
{
}

/*!
    Destroys the NDEF filter.
*/
QNdefFilter::~QNdefFilter()
{
}

/*!
    Assigns \a other to this filter and returns a reference to this filter.
*/
QNdefFilter &QNdefFilter::operator=(const QNdefFilter &other)
{
    if (d != other.d)
        d = other.d;

    return *this;
}

/*!
    Clears the filter.
*/
void QNdefFilter::clear()
{
    d->orderMatching = false;
    d->filterRecords.clear();
}

/*!
    Sets the ording requirements of the filter. If \a on is true the filter will only match if the
    order of records in the filter matches the order of the records in the NDEF message. If \a on
    is false the order of the records is not taken into account when matching.

    By default record order is not taken into account.
*/
void QNdefFilter::setOrderMatch(bool on)
{
    d->orderMatching = on;
}

/*!
    Returns true if the filter takes NDEF record order into account when matching; otherwise
    returns false.
*/
bool QNdefFilter::orderMatch() const
{
    return d->orderMatching;
}

/*!
    Appends a record with type name format \a typeNameFormat and type \a type to the NDEF filter.
    The record must occur between \a min and \a max times in the NDEF message.
*/
void QNdefFilter::appendRecord(QNdefRecord::TypeNameFormat typeNameFormat, const QByteArray &type,
                               unsigned int min, unsigned int max)
{
    QNdefFilter::Record record;

    record.typeNameFormat = typeNameFormat;
    record.type = type;
    record.minimum = min;
    record.maximum = max;

    d->filterRecords.append(record);
}

/*!
    Appends \a record to the NDEF filter.
*/
void QNdefFilter::appendRecord(const Record &record)
{
    d->filterRecords.append(record);
}

/*!
    Returns the NDEF record at index \a i.
*/
QNdefFilter::Record QNdefFilter::recordAt(int i) const
{
    return d->filterRecords.at(i);
}

/*!
    Returns the number of NDEF records in the filter.
*/
int QNdefFilter::recordCount() const
{
    return d->filterRecords.count();
}

QTNFC_END_NAMESPACE

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

#include "qdeclarativendeffilter_p.h"

/*!
    \qmlclass NdefFilter QDeclarativeNdefFilter
    \brief The NdefFilter element represents a filtering constraint for NDEF message records.

    \ingroup connectivity-qml
    \inmodule QtConnectivity

    \sa NearField
    \sa QNdefFilter

    The NdefFilter element is part of the \bold {QtMobility.connectivity 1.2} module.

    The NdefFilter element is used with the NearField element to read NDEF messages from NFC Forum
    tags that match a given structure.

    \code
        NearField {
            filter: [
                NdefFilter {
                    type: "urn:nfc:wkt:U"
                    minimum: 1
                    maximum: 1
                }
            ]
        }
    \endcode
*/

/*!
    \qmlproperty string NdefFilter::type

    This property holds the NDEF record type that the filter matches.  This property must be set to
    the fully qualified record type, i.e. including the NIS and NSS prefixes.  For example set to
    \i {urn:nfc:wkt:U} to match NFC RTD-URI records.
*/

/*!
    \qmlproperty int NdefFilter::minimum

    This property holds the minimum number of records of the given type that must be in the NDEF
    message for it match.

    To match any number of records set both the minimum and maximum properties to -1.

    \sa maximum
*/

/*!
    \qmlproperty int NdefFilter::maximum

    This property holds the maximum number of records of the given type that must be in the NDEF
    message for it match.

    To match any number of records set both the minimum and maximum properties to -1.

    \sa minimum
*/

QDeclarativeNdefFilter::QDeclarativeNdefFilter(QObject *parent)
:   QObject(parent), m_minimum(-1), m_maximum(-1)
{
}

QString QDeclarativeNdefFilter::type() const
{
    return m_type;
}

void QDeclarativeNdefFilter::setType(const QString &t)
{
    if (m_type == t)
        return;

    m_type = t;
    emit typeChanged();
}

int QDeclarativeNdefFilter::minimum() const
{
    return m_minimum;
}

void QDeclarativeNdefFilter::setMinimum(int value)
{
    if (m_minimum == value)
        return;

    m_minimum = value;
    emit minimumChanged();
}

int QDeclarativeNdefFilter::maximum() const
{
    return m_maximum;
}

void QDeclarativeNdefFilter::setMaximum(int value)
{
    if (m_maximum == value)
        return;

    m_maximum = value;
    emit maximumChanged();
}

/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarativendeffilter_p.h"

/*!
    \qmltype NdefFilter
    \instantiates QDeclarativeNdefFilter
    \brief The NdefFilter type represents a filtering constraint for NDEF message records.

    \ingroup nfc-qml
    \inmodule QtNfc

    \sa NearField
    \sa QNdefFilter

    The NdefFilter type was introduced in \b {QtNfc 5.0}.

    The NdefFilter type is used with the NearField type to read NDEF messages from NFC Forum
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
    \li {urn:nfc:wkt:U} to match NFC RTD-URI records.
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

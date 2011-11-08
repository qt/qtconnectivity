/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarativenearfield_p.h"
#include "qdeclarativendeffilter_p.h"
#include "qdeclarativendeftextrecord_p.h"
#include "qdeclarativendefurirecord_p.h"
#include "qdeclarativendefmimerecord_p.h"

#include <qndefmessage.h>
#include <qndefnfctextrecord.h>
#include <qndefnfcurirecord.h>

/*!
    \qmlclass NearField QDeclarativeNearField
    \brief The NearField element provides access to NDEF messages stored on NFC Forum tags.

    \ingroup connectivity-qml
    \inmodule QtConnectivity

    \sa NdefFilter
    \sa NdefRecord

    \sa QNearFieldManager
    \sa QNdefMessage
    \sa QNdefRecord

    The NearField element is part of the \bold {QtMobility.connectivity 1.2} module.

    The NearField element can be used to read NDEF messages from NFC Forum tags.  Set the \l filter
    and \l orderMatch properties to match the required NDEF messages.  Once an NDEF message is
    successfully read from a tag the \l messageRecords property is updated.

    \code
        NearField {
            filter: [ NdefFilter { type: "urn:nfc:wkt:U"; minimum: 1; maximum: 1 } ]
            orderMatch: false

            onMessageRecordsChanged: displayMessage()
        }
    \endcode
*/

/*!
    \qmlproperty list<NdefRecord> NearField::messageRecords

    This property contains the list of NDEF records in the last NDEF message read.
*/

/*!
    \qmlproperty list<NdefFilter> NearField::filter

    This property holds the NDEF filter constraints.  The \l messageRecords property will only be
    set to NDEF messages which match the filter.
*/

/*!
    \qmlproperty bool NearField::orderMatch

    This property indicates whether the order of records should be taken into account when matching
    messages.
*/

QDeclarativeNearField::QDeclarativeNearField(QObject *parent)
:   QObject(parent), m_orderMatch(false), m_componentCompleted(false), m_messageUpdating(false),
    m_manager(0), m_messageHandlerId(-1)
{
}

QDeclarativeListProperty<QDeclarativeNdefRecord> QDeclarativeNearField::messageRecords()
{
    return QDeclarativeListProperty<QDeclarativeNdefRecord>(this, 0,
                                                            &QDeclarativeNearField::append_messageRecord,
                                                            &QDeclarativeNearField::count_messageRecords,
                                                            &QDeclarativeNearField::at_messageRecord,
                                                            &QDeclarativeNearField::clear_messageRecords);

}

QDeclarativeListProperty<QDeclarativeNdefFilter> QDeclarativeNearField::filter()
{
    return QDeclarativeListProperty<QDeclarativeNdefFilter>(this, 0,
                                                            &QDeclarativeNearField::append_filter,
                                                            &QDeclarativeNearField::count_filters,
                                                            &QDeclarativeNearField::at_filter,
                                                            &QDeclarativeNearField::clear_filter);
}

bool QDeclarativeNearField::orderMatch() const
{
    return m_orderMatch;
}

void QDeclarativeNearField::setOrderMatch(bool on)
{
    if (m_orderMatch == on)
        return;

    m_orderMatch = on;
    emit orderMatchChanged();
}

void QDeclarativeNearField::componentComplete()
{
    m_componentCompleted = true;

    if (!m_filter.isEmpty())
        registerMessageHandler();
}

void QDeclarativeNearField::registerMessageHandler()
{
    if (!m_manager)
        m_manager = new QNearFieldManager(this);

    if (m_messageHandlerId != -1)
        m_manager->unregisterNdefMessageHandler(m_messageHandlerId);

    // no filter abort
    if (m_filter.isEmpty())
        return;

    QNdefFilter filter;
    filter.setOrderMatch(m_orderMatch);
    foreach (QDeclarativeNdefFilter *f, m_filter) {
        const QString type = f->type();
        uint min = f->minimum() < 0 ? UINT_MAX : f->minimum();
        uint max = f->maximum() < 0 ? UINT_MAX : f->maximum();

        if (type.startsWith(QLatin1String("urn:nfc:wkt:")))
            filter.appendRecord(QNdefRecord::NfcRtd, type.mid(12).toUtf8(), min, max);
        else if (type.startsWith(QLatin1String("urn:nfc:ext:")))
            filter.appendRecord(QNdefRecord::ExternalRtd, type.mid(12).toUtf8(), min, max);
        else if (type.startsWith(QLatin1String("urn:nfc:mime")))
            filter.appendRecord(QNdefRecord::Mime, type.mid(13).toUtf8(), min, max);
        else
            qWarning("Unknown NDEF record type %s", qPrintable(type));
    }

    m_messageHandlerId = m_manager->registerNdefMessageHandler(filter, this, SLOT(_q_handleNdefMessage(QNdefMessage)));
}

void QDeclarativeNearField::_q_handleNdefMessage(const QNdefMessage &message)
{
    m_messageUpdating = true;

    QDeclarativeListReference listRef(this, "messageRecords");

    listRef.clear();

    foreach (const QNdefRecord &record, message)
        listRef.append(qNewDeclarativeNdefRecordForNdefRecord(record));

    m_messageUpdating = false;

    emit messageRecordsChanged();
}

void QDeclarativeNearField::append_messageRecord(QDeclarativeListProperty<QDeclarativeNdefRecord> *list,
                                           QDeclarativeNdefRecord *record)
{
    QDeclarativeNearField *nearField = qobject_cast<QDeclarativeNearField *>(list->object);
    if (!nearField)
        return;

    record->setParent(nearField);
    nearField->m_message.append(record);
    if (!nearField->m_messageUpdating)
        emit nearField->messageRecordsChanged();
}

int QDeclarativeNearField::count_messageRecords(QDeclarativeListProperty<QDeclarativeNdefRecord> *list)
{
    QDeclarativeNearField *nearField = qobject_cast<QDeclarativeNearField *>(list->object);
    if (!nearField)
        return 0;

    return nearField->m_message.count();
}

QDeclarativeNdefRecord *QDeclarativeNearField::at_messageRecord(QDeclarativeListProperty<QDeclarativeNdefRecord> *list,
                                   int index)
{
    QDeclarativeNearField *nearField = qobject_cast<QDeclarativeNearField *>(list->object);
    if (!nearField)
        return 0;

    return nearField->m_message.at(index);
}

void QDeclarativeNearField::clear_messageRecords(QDeclarativeListProperty<QDeclarativeNdefRecord> *list)
{
    QDeclarativeNearField *nearField = qobject_cast<QDeclarativeNearField *>(list->object);
    if (nearField) {
        qDeleteAll(nearField->m_message);
        nearField->m_message.clear();
        if (!nearField->m_messageUpdating)
            emit nearField->messageRecordsChanged();
    }
}

void QDeclarativeNearField::append_filter(QDeclarativeListProperty<QDeclarativeNdefFilter> *list,
                                           QDeclarativeNdefFilter *filter)
{
    QDeclarativeNearField *nearField = qobject_cast<QDeclarativeNearField *>(list->object);
    if (!nearField)
        return;

    filter->setParent(nearField);
    nearField->m_filter.append(filter);
    emit nearField->filterChanged();

    if (nearField->m_componentCompleted)
        nearField->registerMessageHandler();
}

int QDeclarativeNearField::count_filters(QDeclarativeListProperty<QDeclarativeNdefFilter> *list)
{
    QDeclarativeNearField *nearField = qobject_cast<QDeclarativeNearField *>(list->object);
    if (!nearField)
        return 0;

    return nearField->m_filter.count();
}

QDeclarativeNdefFilter *QDeclarativeNearField::at_filter(QDeclarativeListProperty<QDeclarativeNdefFilter> *list,
                                   int index)
{
    QDeclarativeNearField *nearField = qobject_cast<QDeclarativeNearField *>(list->object);
    if (!nearField)
        return 0;

    return nearField->m_filter.at(index);
}

void QDeclarativeNearField::clear_filter(QDeclarativeListProperty<QDeclarativeNdefFilter> *list)
{
    QDeclarativeNearField *nearField = qobject_cast<QDeclarativeNearField *>(list->object);
    if (!nearField)
        return;

    qDeleteAll(nearField->m_filter);
    nearField->m_filter.clear();
    emit nearField->filterChanged();
    if (nearField->m_componentCompleted)
        nearField->registerMessageHandler();
}

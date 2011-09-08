/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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

#include "qdeclarativendefrecord.h"

#include <QtCore/QMap>
#include <QtCore/QRegExp>

/*!
    \class QDeclarativeNdefRecord
    \brief The QDeclarativeNdefRecord class implements the NdefRecord element in QML.

    \ingroup connectivity-nfc
    \inmodule QtConnectivity
    \since 5.0

    \sa NdefRecord

    The QDeclarativeNdefRecord class is the base class for all NdefRecord elements in QML.  To
    support a new NDEF record type in QML subclass this class and expose new properties, member
    functions and signals appropriate for the new record type.  The following must be done to
    create a new NDEF record type in QML:

    \list
        \o The subclass must have a Q_OBJECT macro in its declaration.
        \o The subclass must have an \l {Q_INVOKABLE}{invokable} constructor that takes a
           QNdefRecord and a QObject pointer.
        \o The subclass must be declared as an NDEF record by expanding the Q_DECLARE_NDEFRECORD()
           macro in the implementation file of the subclass.
        \o The subclass must be registered with QML.
    \endlist

    For example the declaration of such a class may look like the following.

    \snippet snippets/connectivity/foorecord.h Foo declaration

    Within the implementation file the Q_DECLARE_NDEFRECORD() macro is expanded:

    \snippet snippets/connectivity/foorecord.cpp Declare foo record

    Finially the application or plugin code calls qmlRegisterType():

    \code
        qmlRegisterType<QDeclarativeNdefFooRecord>(uri, 1, 0, "NdefFooRecord");
    \endcode
*/

/*!
    \qmlclass NdefRecord QDeclarativeNdefRecord
    \brief The NdefRecord element represents a record in an NDEF message.

    \ingroup connectivity-qml
    \inmodule QtConnectivity

    \sa NdefFilter
    \sa NearField

    \sa QNdefRecord

    The NdefRecord element is the base element for all NDEF record elements in QML.  It contains
    a single property holding the type of record.

    This class is not intended to be used directly, but extended from C++.

    \sa QDeclarativeNdefRecord
*/

/*!
    \qmlproperty string NdefRecord::recordType

    This property holds the fully qualified record type of the NDEF record.  The fully qualified
    record type includes the NIS and NSS prefixes.
*/

/*!
    \fn void QDeclarativeNdefRecord::recordTypeChanged()

    This signal is emitted when the record type changes.
*/

/*!
    \property QDeclarativeNdefRecord::recordType

    This property hold the record type of the NDEF record that this class represents.
*/

/*!
    \macro Q_DECLARE_NDEFRECORD(className, typeNameFormat, type)
    \relates QDeclarativeNdefRecord

    This macro ensures that \a className is declared as the class implementing the NDEF record
    identified by \a typeNameFormat and \a type.

    This macro should be expanded in the implementation file for \a className.
*/

static QMap<QString, const QMetaObject *> registeredNdefRecordTypes;

class QDeclarativeNdefRecordPrivate
{
public:
    QNdefRecord record;
};

static QString urnForRecordType(QNdefRecord::TypeNameFormat typeNameFormat, const QByteArray &type)
{
    switch (typeNameFormat) {
    case QNdefRecord::NfcRtd:
        return QLatin1String("urn:nfc:wkt:") + type;
    case QNdefRecord::ExternalRtd:
        return QLatin1String("urn:nfc:ext:") + type;
    case QNdefRecord::Mime:
        return QLatin1String("urn:nfc:mime:") + type;
    default:
        return QString();
    }
}

/*!
    \internal
*/
void qRegisterNdefRecordTypeHelper(const QMetaObject *metaObject,
                                   QNdefRecord::TypeNameFormat typeNameFormat,
                                   const QByteArray &type)
{
    registeredNdefRecordTypes.insert(urnForRecordType(typeNameFormat, type), metaObject);
}

/*!
    \internal
*/
QDeclarativeNdefRecord *qNewDeclarativeNdefRecordForNdefRecord(const QNdefRecord &record)
{
    const QString urn = urnForRecordType(record.typeNameFormat(), record.type());

    QMapIterator<QString, const QMetaObject *> i(registeredNdefRecordTypes);
    while (i.hasNext()) {
        i.next();

        QRegExp ex(i.key());
        if (!ex.exactMatch(urn))
            continue;

        const QMetaObject *metaObject = i.value();
        if (!metaObject)
            continue;

        return static_cast<QDeclarativeNdefRecord *>(metaObject->newInstance(
            Q_ARG(QNdefRecord, record), Q_ARG(QObject *, 0)));
    }

    return new QDeclarativeNdefRecord(record);
}

/*!
    Constructs a new empty QDeclarativeNdefRecord with \a parent.
*/
QDeclarativeNdefRecord::QDeclarativeNdefRecord(QObject *parent)
:   QObject(parent), d_ptr(new QDeclarativeNdefRecordPrivate)
{
}

/*!
   Constructs a new QDeclarativeNdefRecord representing \a record.  The parent of the newly
   constructed object will be set to \a parent.
*/
QDeclarativeNdefRecord::QDeclarativeNdefRecord(const QNdefRecord &record, QObject *parent)
:   QObject(parent), d_ptr(new QDeclarativeNdefRecordPrivate)
{
    d_ptr->record = record;
}

/*!
    Returns the fully qualified record type of the record.  The fully qualified record type
    includes both the NIS and NSS prefixes.
*/
QString QDeclarativeNdefRecord::recordType() const
{
    Q_D(const QDeclarativeNdefRecord);

    if (d->record.typeNameFormat() == QNdefRecord::Empty)
        return QString();

    return urnForRecordType(d->record.typeNameFormat(), d->record.type());
}

/*!
    Sets the record type to \a type if it is not currently equal to \a type; otherwise does
    nothing.  If the record type is set the recordTypeChanged() signal will be emitted.
*/
void QDeclarativeNdefRecord::setRecordType(const QString &type)
{
    if (type == recordType())
        return;

    Q_D(QDeclarativeNdefRecord);

    if (type.startsWith(QLatin1String("urn:nfc:wkt:"))) {
        d->record.setTypeNameFormat(QNdefRecord::NfcRtd);
        d->record.setType(type.mid(12).toUtf8());
    } else if (type.startsWith(QLatin1String("urn:nfc:ext:"))) {
        d->record.setTypeNameFormat(QNdefRecord::ExternalRtd);
        d->record.setType(type.mid(12).toUtf8());
    } else if (type.startsWith(QLatin1String("urn:nfc:mime:"))) {
        d->record.setTypeNameFormat(QNdefRecord::Mime);
        d->record.setType(type.mid(13).toUtf8());
    } else {
        qWarning("Don't know how to decode NDEF type %s\n", qPrintable(type));
    }

    emit recordTypeChanged();
}

/*!
    Returns a copy of the record.
*/
QNdefRecord QDeclarativeNdefRecord::record() const
{
    Q_D(const QDeclarativeNdefRecord);

    return d->record;
}

/*!
    Sets the record to \a record.
*/
void QDeclarativeNdefRecord::setRecord(const QNdefRecord &record)
{
    Q_D(QDeclarativeNdefRecord);

    d->record = record;
}

#include "moc_qdeclarativendefrecord.cpp"

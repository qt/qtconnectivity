/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbluetoothserviceinfo.h"
#include "qbluetoothserviceinfo_p.h"

#include "bluez/manager_p.h"
#include "bluez/service_p.h"

#include <QtCore/QXmlStreamWriter>

QTBLUETOOTH_BEGIN_NAMESPACE

static void writeAttribute(QXmlStreamWriter *stream, const QVariant &attribute)
{
    const QString unsignedFormat(QLatin1String("0x%1"));

    switch (attribute.type()) {
    case QMetaType::Void:
        stream->writeEmptyElement(QLatin1String("nil"));
        break;
    case QMetaType::UChar:
        stream->writeEmptyElement(QLatin1String("uint8"));
        stream->writeAttribute(QLatin1String("value"),
                               unsignedFormat.arg(attribute.value<quint8>(), 2, 16,
                                                  QLatin1Char('0')));
        //stream->writeAttribute(QLatin1String("name"), foo);
        break;
    case QMetaType::UShort:
        stream->writeEmptyElement(QLatin1String("uint16"));
        stream->writeAttribute(QLatin1String("value"),
                               unsignedFormat.arg(attribute.value<quint16>(), 4, 16,
                                                  QLatin1Char('0')));
        //stream->writeAttribute(QLatin1String("name"), foo);
        break;
    case QMetaType::UInt:
        stream->writeEmptyElement(QLatin1String("uint32"));
        stream->writeAttribute(QLatin1String("value"),
                               unsignedFormat.arg(attribute.value<quint32>(), 8, 16,
                                                  QLatin1Char('0')));
        //stream->writeAttribute(QLatin1String("name"), foo);
        break;
    case QMetaType::Char:
        stream->writeEmptyElement(QLatin1String("int8"));
        stream->writeAttribute(QLatin1String("value"),
                               QString::number(attribute.value<uchar>(), 16));
        //stream->writeAttribute(QLatin1String("name"), foo);
        break;
    case QMetaType::Short:
        stream->writeEmptyElement(QLatin1String("int16"));
        stream->writeAttribute(QLatin1String("value"),
                               QString::number(attribute.value<qint16>(), 16));
        //stream->writeAttribute(QLatin1String("name"), foo);
        break;
    case QMetaType::Int:
        stream->writeEmptyElement(QLatin1String("int32"));
        stream->writeAttribute(QLatin1String("value"),
                               QString::number(attribute.value<qint32>(), 16));
        //stream->writeAttribute(QLatin1String("name"), foo);
        break;
    case QMetaType::QString:
        stream->writeEmptyElement(QLatin1String("text"));
        if (/* require hex encoding */ false) {
            stream->writeAttribute(QLatin1String("value"), QString::fromLatin1(
                                       attribute.value<QString>().toUtf8().toHex().constData()));
            stream->writeAttribute(QLatin1String("encoding"), QLatin1String("hex"));
        } else {
            stream->writeAttribute(QLatin1String("value"), attribute.value<QString>());
            stream->writeAttribute(QLatin1String("encoding"), QLatin1String("normal"));
        }
        //stream->writeAttribute(QLatin1String("name"), foo);
        break;
            case QMetaType::Bool:
        stream->writeEmptyElement(QLatin1String("boolean"));
        if (attribute.value<bool>())
            stream->writeAttribute(QLatin1String("value"), QLatin1String("true"));
        else
            stream->writeAttribute(QLatin1String("value"), QLatin1String("false"));
        //stream->writeAttribute(QLatin1String("name"), foo);
        break;
            case QMetaType::QUrl:
        stream->writeEmptyElement(QLatin1String("url"));
        stream->writeAttribute(QLatin1String("value"), attribute.value<QUrl>().toString());
        //stream->writeAttribute(QLatin1String("name"), foo);
        break;
            case QVariant::UserType:
        if (attribute.userType() == qMetaTypeId<QBluetoothUuid>()) {
            stream->writeEmptyElement(QLatin1String("uuid"));

            QBluetoothUuid uuid = attribute.value<QBluetoothUuid>();
            switch (uuid.minimumSize()) {
            case 0:
                stream->writeAttribute(QLatin1String("value"),
                                       unsignedFormat.arg(quint16(0), 4, 16, QLatin1Char('0')));
                break;
            case 2:
                stream->writeAttribute(QLatin1String("value"),
                                       unsignedFormat.arg(uuid.toUInt16(), 4, 16,
                                                          QLatin1Char('0')));
                break;
            case 4:
                stream->writeAttribute(QLatin1String("value"),
                                       unsignedFormat.arg(uuid.toUInt32(), 8, 16,
                                                          QLatin1Char('0')));
                break;
            case 16:
                stream->writeAttribute(QLatin1String("value"), uuid.toString().mid(1, 36));
                break;
            default:
                stream->writeAttribute(QLatin1String("value"), uuid.toString().mid(1, 36));
            }
        } else if (attribute.userType() == qMetaTypeId<QBluetoothServiceInfo::Sequence>()) {
            stream->writeStartElement(QLatin1String("sequence"));
            const QBluetoothServiceInfo::Sequence *sequence =
                    static_cast<const QBluetoothServiceInfo::Sequence *>(attribute.data());
            foreach (const QVariant &v, *sequence)
                writeAttribute(stream, v);
            stream->writeEndElement();
        } else if (attribute.userType() == qMetaTypeId<QBluetoothServiceInfo::Alternative>()) {
            const QBluetoothServiceInfo::Alternative *alternative =
                    static_cast<const QBluetoothServiceInfo::Alternative *>(attribute.data());
            foreach (const QVariant &v, *alternative)
                writeAttribute(stream, v);
            stream->writeEndElement();
        }
        break;
            default:
        qDebug() << "Unknown variant type", attribute.userType();
    }
}

bool QBluetoothServiceInfo::isRegistered() const
{
    Q_D(const QBluetoothServiceInfo);

    return d->registered;
}

bool QBluetoothServiceInfo::registerService() const
{
    Q_D(const QBluetoothServiceInfo);

    return d->registerService();
}

bool QBluetoothServiceInfo::unregisterService() const
{
    Q_D(const QBluetoothServiceInfo);

    if (!d->registered)
        return false;

    if (!d->ensureSdpConnection())
        return false;

    QDBusPendingReply<> reply = d->service->RemoveRecord(d->serviceRecord);
    reply.waitForFinished();
    if (reply.isError())
        return false;

    d->serviceRecord = 0;

    d->registered = false;
    return true;
}

QBluetoothServiceInfoPrivate::QBluetoothServiceInfoPrivate()
:   service(0), serviceRecord(0), registered(false)
{
}

QBluetoothServiceInfoPrivate::~QBluetoothServiceInfoPrivate()
{
}

void QBluetoothServiceInfoPrivate::setRegisteredAttribute(quint16 attributeId, const QVariant &value) const
{
    Q_UNUSED(attributeId);
    Q_UNUSED(value);

    registerService();
}

void QBluetoothServiceInfoPrivate::removeRegisteredAttribute(quint16 attributeId) const
{
    Q_UNUSED(attributeId);

    registerService();
}

bool QBluetoothServiceInfoPrivate::ensureSdpConnection() const
{
    if (service)
        return true;

    OrgBluezManagerInterface manager(QLatin1String("org.bluez"), QLatin1String("/"),
                                     QDBusConnection::systemBus());

    QDBusPendingReply<QDBusObjectPath> reply = manager.FindAdapter(QLatin1String("any"));
    reply.waitForFinished();
    if (reply.isError())
        return false;

    service = new OrgBluezServiceInterface(QLatin1String("org.bluez"), reply.value().path(),
                                           QDBusConnection::systemBus());

    return true;
}

bool QBluetoothServiceInfoPrivate::registerService() const
{
    if (!ensureSdpConnection())
        return false;

    QString xmlServiceRecord;

    QXmlStreamWriter stream(&xmlServiceRecord);
    stream.setAutoFormatting(true);

    stream.writeStartDocument(QLatin1String("1.0"));

    stream.writeStartElement(QLatin1String("record"));

    const QString unsignedFormat(QLatin1String("0x%1"));

    QMap<quint16, QVariant>::ConstIterator i = attributes.constBegin();
    while (i != attributes.constEnd()) {
        QString t = unsignedFormat.arg(i.key(), 4, QLatin1Char('0'));
        stream.writeStartElement(QLatin1String("attribute"));
        stream.writeAttribute(QLatin1String("id"),
                              unsignedFormat.arg(i.key(), 4, 16, QLatin1Char('0')));
        writeAttribute(&stream, i.value());
        stream.writeEndElement();

        ++i;
    }

    stream.writeEndElement();

    stream.writeEndDocument();

//    qDebug() << xmlServiceRecord;

    if (!registered) {
        QDBusPendingReply<uint> reply = service->AddRecord(xmlServiceRecord);
        reply.waitForFinished();
        if (reply.isError())
            return false;

        serviceRecord = reply.value();
    } else {
        QDBusPendingReply<> reply = service->UpdateRecord(serviceRecord, xmlServiceRecord);
        reply.waitForFinished();
        if (reply.isError())
            return false;
    }

    registered = true;
    return true;
}

QTBLUETOOTH_END_NAMESPACE

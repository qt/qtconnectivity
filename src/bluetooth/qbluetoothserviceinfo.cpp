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

#include "qbluetoothserviceinfo.h"
#include "qbluetoothserviceinfo_p.h"

#include <QUrl>

QTBLUETOOTH_BEGIN_NAMESPACE

/*!
    \class QBluetoothServiceInfo::Sequence
    \brief The Sequence class stores attributes of a Bluetooth Data Element Sequence.

    \ingroup connectivity-bluetooth
    \inmodule QtBluetooth
*/

/*!
    \fn QBluetoothServiceInfo::Sequence::Sequence()

    Constructs a new empty sequence.
*/

/*!
    \fn QBluetoothServiceInfo::Sequence::Sequence(const QList<QVariant> &list)

    Constructs a new sequence that is a copy of \a list.
*/

/*!
    \class QBluetoothServiceInfo::Alternative
    \brief The Alternative class stores attributes of a 
	   Bluetooth Data Element Alternative.

    \ingroup connectivity-bluetooth
    \inmodule QtBluetooth
*/

/*!
    \fn QBluetoothServiceInfo::Alternative::Alternative()

    Constructs a new empty alternative.
*/

/*!
    \fn QBluetoothServiceInfo::Alternative::Alternative(const QList<QVariant> &list)

    Constructs a new alternative that is a copy of \a list.
*/

/*!
    \class QBluetoothServiceInfo
    \brief The QBluetoothServiceInfo class provides access to the attribute values of a Bluetooth service.

    \ingroup connectivity-bluetooth
    \inmodule QtBluetooth

    QBluetoothServiceInfo provides information about a service offered by a Bluetooth device.
*/

/*!
    \enum QBluetoothServiceInfo::AttributeId

    Bluetooth service attributes.

    \value ServiceClassIds          UUIDs of service classes that the service conforms to.
    \value ServiceId                UUID that uniquely identifies the service.
    \value ProtocolDescriptorList   List of protocols used by the service.
    \value BrowseGroupList          List of browse groups the service is in.
    \value ServiceAvailability      Value indicating the availability of the service.
    \value PrimaryLanguageBase      Base index for primary language text descriptors.
    \value ServiceRecordHandle      Specifies a service record from which attributes can be retrieved
    \value ServiceName              Name of the Bluetooth service in the primary language.
    \value ServiceDescription       Description of the Bluetooth service in the primary language.
    \value ServiceProvider          Name of the company / entity that provides the Bluetooth
                                    service primary language.
*/

/*!
    \enum QBluetoothServiceInfo::Protocol

    This enum describes the socket protocol used by the service.

    \value UnknownProtocol  The service uses an unknown socket protocol.
    \value L2capProtocol    The service uses the L2CAP socket protocol.
    \value RfcommProtocol   The service uses the RFCOMM socket protocol.
*/

/*!
    \fn bool QBluetoothServiceInfo::isRegistered() const

    Returns true if the service info is registered with the platforms service discovery protocol
    (SDP) implementation; otherwise returns false.
*/

/*!
    \fn bool QBluetoothServiceInfo::registerService() const

    Registers this service with the platforms service discovery protocol (SDP) implementation,
    making it findable by other devices when they perform service discovery.  Returns true if the
    service is successfully registered, otherwise returns false.  Once registered changes to the record
    cannot be made.  The service must be unregistered and registered.
*/

/*!
    \fn bool QBluetoothServiceInfo::unregisterService() const

    Unregisters this service with the platforms service discovery protocol (SDP) implementation.

    This service will not longer be findable by other devices via service discovery.

    Returns true if the service is successfully unregistered, otherwise returns false.
*/

/*!
    \fn void QBluetoothServiceInfo::setAttribute(quint16 attributeId, const QBluetoothUuid &value)

    This is a convenience function.

    Sets the attribute identified by \a attributeId to \a value.
*/

/*!
    \fn void QBluetoothServiceInfo::setAttribute(quint16 attributeId, const QBluetoothServiceInfo::Sequence &value)

    This is a convenience function.

    Sets the attribute identified by \a attributeId to \a value.
*/

/*!
    \fn void QBluetoothServiceInfo::setAttribute(quint16 attributeId, const QBluetoothServiceInfo::Alternative &value)

    This is a convenience function.

    Sets the attribute identified by \a attributeId to \a value.
*/

/*!
    \fn void QBluetoothServiceInfo::setServiceName(const QString &name)

    This is a convenience function. It is equivalent to calling
    setAttribute(QBluetoothServiceInfo::ServiceName, name).

    Sets the service name in the primary language to \a name.

    \sa serviceName(), setAttribute()
*/

/*!
    \fn QString QBluetoothServiceInfo::serviceName() const

    This is a convenience function. It is equivalent to calling
    attribute(QBluetoothServiceInfo::ServiceName).toString().

    Returns the service name in the primary language.

    \sa setServiceName(), attribute()
*/

/*!
    \fn void QBluetoothServiceInfo::setServiceDescription(const QString &description)

    This is a convenience function. It is equivalent to calling
    setAttribute(QBluetoothServiceInfo::ServiceDescription, description).

    Sets the service description in the primary language to \a description.

    \sa serviceDescription(), setAttribute()
*/

/*!
    \fn QString QBluetoothServiceInfo::serviceDescription() const

    This is a convenience function. It is equivalent to calling
    attribute(QBluetoothServiceInfo::ServiceDescription).toString().

    Returns the service description in the primary language.

    \sa setServiceDescription(), attribute()
*/

/*!
    \fn void QBluetoothServiceInfo::setServiceProvider(const QString &provider)

    This is a convenience function. It is equivalent to calling
    setAttribute(QBluetoothServiceInfo::ServiceProvider, provider).

    Sets the service provider in the primary language to \a provider.

    \sa serviceProvider(), setAttribute()
*/

/*!
    \fn QString QBluetoothServiceInfo::serviceProvider() const

    This is a convenience function. It is equivalent to calling
    attribute(QBluetoothServiceInfo::ServiceProvider).toString().

    Returns the service provider in the primary language.

    \sa setServiceProvider(), attribute()
*/

/*!
    \fn void QBluetoothServiceInfo::setServiceAvailability(quint8 availability)

    This is a convenience function. It is equivalent to calling
    setAttribute(QBluetoothServiceInfo::ServiceAvailability, availability).

    Sets the availabiltiy of the service to \a availability.

    \sa serviceAvailability(), setAttribute()
*/

/*!
    \fn quint8 QBluetoothServiceInfo::serviceAvailability() const

    This is a convenience function. It is equivalent to calling
    attribute(QBluetoothServiceInfo::ServiceAvailability).toUInt().

    Returns the availability of the service.

    \sa setServiceAvailability(), attribute()
*/

/*!
    \fn void QBluetoothServiceInfo::setServiceUuid(const QBluetoothUuid &uuid)

    This is a convenience function. It is equivalent to calling
    setAttribute(QBluetoothServiceInfo::ServiceId, uuid).

    Sets the service UUID to \a uuid.

    \sa serviceUuid(), setAttribute()
*/

/*!
    \fn QBluetoothUuid QBluetoothServiceInfo::serviceUuid() const

    This is a convenience function. It is equivalent to calling
    attribute(QBluetoothServiceInfo::ServiceId).value<QBluetoothUuid>().

    Returns the UUID of the service.

    \sa setServiceUuid(), attribute()
*/

/*!
    \fn QList<QBluetoothUuid> QBluetoothServiceInfo::serviceClassUuids() const

    This is a convenience function. It is equivalent to calling
    attribute(QBluetoothServiceInfo::ServiceClassIds).value<QList<QBluetoothUuid> >().

    Returns a list of UUIDs describing the service classes that this service conforms to.

    \sa attribute()
*/


/*!
    Construct a new invalid QBluetoothServiceInfo;
*/
QBluetoothServiceInfo::QBluetoothServiceInfo()
: d_ptr(new QBluetoothServiceInfoPrivate)
{
    d_ptr->q_ptr = this;
}

/*!
    Construct a new QBluetoothServiceInfo that is a copy of \a other.
*/
QBluetoothServiceInfo::QBluetoothServiceInfo(const QBluetoothServiceInfo &other)
: d_ptr(new QBluetoothServiceInfoPrivate)
{
    d_ptr->q_ptr = this;
    *this = other;
}

/*!
    Destroys the QBluetoothServiceInfo object.
*/
QBluetoothServiceInfo::~QBluetoothServiceInfo()
{
    delete d_ptr;
}

/*!
    Returns true if the Bluetooth service info object is valid; otherwise returns false.

    An invalid Bluetooth service info has no attributes.
*/
bool QBluetoothServiceInfo::isValid() const
{
    Q_D(const QBluetoothServiceInfo);

    return !d->attributes.isEmpty();
}

/*!
    Returns true if the Bluetooth service info object is considered complete; otherwise returns false.

    A complete service info contains a ProtocolDescriptorList attribute.
*/
bool QBluetoothServiceInfo::isComplete() const
{
    Q_D(const QBluetoothServiceInfo);

    return d->attributes.keys().contains(ProtocolDescriptorList);
}

/*!
    Returns the address of the Bluetooth device that provides this service.
*/
QBluetoothDeviceInfo QBluetoothServiceInfo::device() const
{
    Q_D(const QBluetoothServiceInfo);

    return d->deviceInfo;
}

/*!
    Sets the Bluetooth device that provides this service to \a device.
*/
void QBluetoothServiceInfo::setDevice(const QBluetoothDeviceInfo &device)
{
    Q_D(QBluetoothServiceInfo);

    d->deviceInfo = device;
}

/*!
    Sets the attribute identified by \a attributeId to \a value.

    IF the service info is registered with the platforms SDP database the database entry is also
    updated.

    \sa isRegistered(), registerService()
*/
void QBluetoothServiceInfo::setAttribute(quint16 attributeId, const QVariant &value)
{
    Q_D(QBluetoothServiceInfo);

    if (value.type() == QVariant::List)
        qDebug() << "tried attribute with type QVariantList" << value;

    d->attributes[attributeId] = value;

    if (isRegistered())
        d->setRegisteredAttribute(attributeId, value);
}

/*!
    Returns the value of the attribute \a attributeId.
*/
QVariant QBluetoothServiceInfo::attribute(quint16 attributeId) const
{
    Q_D(const QBluetoothServiceInfo);

    return d->attributes[attributeId];
}

/*!
    Returns a list of all attribute ids that this service info has.
*/
QList<quint16> QBluetoothServiceInfo::attributes() const
{
    Q_D(const QBluetoothServiceInfo);

    return d->attributes.keys();
}

/*!
    Returns true if the service info contains the attribute \a attributeId; otherwise returns
    false.
*/
bool QBluetoothServiceInfo::contains(quint16 attributeId) const
{
    Q_D(const QBluetoothServiceInfo);

    return d->attributes.contains(attributeId);
}

/*!
    Removes the attribute \a attributeId from this service info.
*/
void QBluetoothServiceInfo::removeAttribute(quint16 attributeId)
{
    Q_D(QBluetoothServiceInfo);

    d->attributes.remove(attributeId);

    if (isRegistered())
        d->removeRegisteredAttribute(attributeId);
}

/*!
    Returns the protocol that this service uses.
*/
QBluetoothServiceInfo::Protocol QBluetoothServiceInfo::socketProtocol() const
{
    QBluetoothServiceInfo::Sequence parameters = protocolDescriptor(QBluetoothUuid::Rfcomm);
    if (!parameters.isEmpty())
        return RfcommProtocol;

    parameters = protocolDescriptor(QBluetoothUuid::L2cap);
    if (!parameters.isEmpty())
        return L2capProtocol;

    return UnknownProtocol;
}

/*!
    This is a convenience function. Returns the protocol/service multiplexer for services which
    support the L2CAP protocol. Otherwise returns -1.

    This function is equivalent to extracting the information from the
    QBluetoothServiceInfo::Sequence returned from
    QBluetoothServiceInfo::attribute(QBluetoothServiceInfo::ProtocolDescriptorList).
*/
int QBluetoothServiceInfo::protocolServiceMultiplexer() const
{
    QBluetoothServiceInfo::Sequence parameters = protocolDescriptor(QBluetoothUuid::L2cap);

    if (parameters.isEmpty())
        return -1;
    else if (parameters.count() == 1)
        return 0;
    else
        return parameters.at(1).toUInt();
}

/*!
    This is a convenience function. Returns the server channel for services which support the
    RFCOMM protocol. Otherwise returns -1.

    This function is equivalent to extracting the information from the
    QBluetoothServiceInfo::Sequence returned from
    QBluetoothServiceInfo::attribute(QBluetootherServiceInfo::ProtocolDescriptorList).
*/
int QBluetoothServiceInfo::serverChannel() const
{
    QBluetoothServiceInfo::Sequence parameters = protocolDescriptor(QBluetoothUuid::Rfcomm);

    if (parameters.isEmpty())
        return -1;
    else if (parameters.count() == 1)
        return 0;
    else
        return parameters.at(1).toUInt();
}

/*!
    Returns the protocol parameters as a QBluetoothServiceInfo::Sequence for protocol \a protocol.

    An empty QBluetoothServiceInfo::Sequence is returned if \a protocol is not supported.
*/
QBluetoothServiceInfo::Sequence QBluetoothServiceInfo::protocolDescriptor(QBluetoothUuid::ProtocolUuid protocol) const
{
    if (!contains(QBluetoothServiceInfo::ProtocolDescriptorList))
        return QBluetoothServiceInfo::Sequence();

    foreach (const QVariant &v, attribute(QBluetoothServiceInfo::ProtocolDescriptorList).value<QBluetoothServiceInfo::Sequence>()) {
        QBluetoothServiceInfo::Sequence parameters = v.value<QBluetoothServiceInfo::Sequence>();
        if(parameters.empty())
            continue;
        if (parameters.at(0).userType() == qMetaTypeId<QBluetoothUuid>()) {
            if (parameters.at(0).value<QBluetoothUuid>() == protocol)
                return parameters;
        }
    }

    return QBluetoothServiceInfo::Sequence();
}

/*!
    Makes a copy of the \a other and assigns it to this QBluetoothServiceInfo object.
*/
QBluetoothServiceInfo &QBluetoothServiceInfo::operator=(const QBluetoothServiceInfo &other)
{
    Q_D(QBluetoothServiceInfo);

    d->attributes = other.d_func()->attributes;
    d->deviceInfo = other.d_func()->deviceInfo;

    return *this;
}

static void dumpAttributeVariant(const QVariant &var, const QString indent)
{
    switch (var.type()) {
    case QMetaType::Void:
        qDebug("%sEmpty", indent.toLocal8Bit().constData());
        break;
    case QMetaType::UChar:
        qDebug("%suchar %u", indent.toLocal8Bit().constData(), var.toUInt());
        break;
    case QMetaType::UShort:
        qDebug("%sushort %u", indent.toLocal8Bit().constData(), var.toUInt());
    case QMetaType::UInt:
        qDebug("%suint %u", indent.toLocal8Bit().constData(), var.toUInt());
        break;
    case QMetaType::Char:
        qDebug("%schar %d", indent.toLocal8Bit().constData(), var.toInt());
        break;
    case QMetaType::Short:
        qDebug("%sshort %d", indent.toLocal8Bit().constData(), var.toInt());
        break;
    case QMetaType::Int:
        qDebug("%sint %d", indent.toLocal8Bit().constData(), var.toInt());
        break;
    case QMetaType::QString:
        qDebug("%sstring %s", indent.toLocal8Bit().constData(), var.toString().toLocal8Bit().constData());
        break;
    case QMetaType::Bool:
        qDebug("%sbool %d", indent.toLocal8Bit().constData(), var.toBool());
        break;
    case QMetaType::QUrl:
        qDebug("%surl %s", indent.toLocal8Bit().constData(), var.toUrl().toString().toLocal8Bit().constData());
        break;
    case QVariant::UserType:
        if (var.userType() == qMetaTypeId<QBluetoothUuid>()) {
            QBluetoothUuid uuid = var.value<QBluetoothUuid>();
            switch (uuid.minimumSize()) {
            case 0:
                qDebug("%suuid NULL", indent.toLocal8Bit().constData());
                break;
            case 2:
                qDebug("%suuid %04x", indent.toLocal8Bit().constData(), uuid.toUInt16());
                break;
            case 4:
                qDebug("%suuid %08x", indent.toLocal8Bit().constData(), uuid.toUInt32());
                break;
            case 16:
                qDebug("%suuid %s", indent.toLocal8Bit().constData(), QByteArray(reinterpret_cast<const char *>(uuid.toUInt128().data), 16).toHex().constData());
                break;
            default:
                qDebug("%suuid ???", indent.toLocal8Bit().constData());
                ;
            }
        } else if (var.userType() == qMetaTypeId<QBluetoothServiceInfo::Sequence>()) {
            qDebug("%sSequence", indent.toLocal8Bit().constData());
            const QBluetoothServiceInfo::Sequence *sequence = static_cast<const QBluetoothServiceInfo::Sequence *>(var.data());
            foreach (const QVariant &v, *sequence)
                dumpAttributeVariant(v, indent + QLatin1Char('\t'));
        } else if (var.userType() == qMetaTypeId<QBluetoothServiceInfo::Alternative>()) {
            qDebug("%sAlternative", indent.toLocal8Bit().constData());
            const QBluetoothServiceInfo::Alternative *alternative = static_cast<const QBluetoothServiceInfo::Alternative *>(var.data());
            foreach (const QVariant &v, *alternative)
                dumpAttributeVariant(v, indent + QLatin1Char('\t'));
        }
        break;
    default:
        qDebug("%sunknown variant type %d", indent.toLocal8Bit().constData(), var.userType());
    }
}


QDebug operator<<(QDebug dbg, const QBluetoothServiceInfo &info)
{
    foreach (quint16 id, info.attributes()) {
        dumpAttributeVariant(info.attribute(id), QString::fromLatin1("(%1)\t").arg(id));
    }
    return dbg;
}

QTBLUETOOTH_END_NAMESPACE

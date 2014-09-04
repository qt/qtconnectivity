/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#include "qbluetoothserviceinfo.h"
#include "qbluetoothdeviceinfo.h"

#include <QtCore/qglobal.h>
#include <QtCore/qmap.h>
#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

class QBluetoothServiceInfoPrivate
{
public:
    QBluetoothServiceInfoPrivate();
    ~QBluetoothServiceInfoPrivate();

    bool registerService(const QBluetoothAddress &localAdapter = QBluetoothAddress());

    bool isRegistered() const;

    bool unregisterService();

    QBluetoothDeviceInfo deviceInfo;
    QMap<quint16, QVariant> attributes;

    QBluetoothServiceInfo::Sequence protocolDescriptor(QBluetoothUuid::ProtocolUuid protocol) const;
    int serverChannel() const;
};

QBluetoothServiceInfoPrivate::QBluetoothServiceInfoPrivate()
{
}

QBluetoothServiceInfoPrivate::~QBluetoothServiceInfoPrivate()
{
}

bool QBluetoothServiceInfoPrivate::registerService(const QBluetoothAddress &localAdapter)
{
    Q_UNUSED(localAdapter)
    return false;
}

bool QBluetoothServiceInfoPrivate::isRegistered() const
{
    return false;
}

bool QBluetoothServiceInfoPrivate::unregisterService()
{
    return false;
}

bool QBluetoothServiceInfo::isRegistered() const
{
    return d_ptr->isRegistered();
}

bool QBluetoothServiceInfo::registerService(const QBluetoothAddress &localAdapter)
{
    return d_ptr->registerService(localAdapter);
}

bool QBluetoothServiceInfo::unregisterService()
{
    return d_ptr->unregisterService();
}

QBluetoothServiceInfo::QBluetoothServiceInfo()
    : d_ptr(QSharedPointer<QBluetoothServiceInfoPrivate>(new QBluetoothServiceInfoPrivate))
{
}

QBluetoothServiceInfo::QBluetoothServiceInfo(const QBluetoothServiceInfo &other)
    : d_ptr(other.d_ptr)
{
}

QBluetoothServiceInfo::~QBluetoothServiceInfo()
{
}

bool QBluetoothServiceInfo::isValid() const
{
    return !d_ptr->attributes.isEmpty();
}

bool QBluetoothServiceInfo::isComplete() const
{
    return d_ptr->attributes.keys().contains(ProtocolDescriptorList);
}

QBluetoothDeviceInfo QBluetoothServiceInfo::device() const
{
    return d_ptr->deviceInfo;
}

void QBluetoothServiceInfo::setDevice(const QBluetoothDeviceInfo &device)
{
    d_ptr->deviceInfo = device;
}

void QBluetoothServiceInfo::setAttribute(quint16 attributeId, const QVariant &value)
{
    d_ptr->attributes[attributeId] = value;
}

QVariant QBluetoothServiceInfo::attribute(quint16 attributeId) const
{
    return d_ptr->attributes.value(attributeId);
}

QList<quint16> QBluetoothServiceInfo::attributes() const
{
    return d_ptr->attributes.keys();
}

bool QBluetoothServiceInfo::contains(quint16 attributeId) const
{
    return d_ptr->attributes.contains(attributeId);
}

void QBluetoothServiceInfo::removeAttribute(quint16 attributeId)
{
    d_ptr->attributes.remove(attributeId);
}

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

int QBluetoothServiceInfo::serverChannel() const
{
    return d_ptr->serverChannel();
}

QBluetoothServiceInfo::Sequence QBluetoothServiceInfo::protocolDescriptor(QBluetoothUuid::ProtocolUuid protocol) const
{
    return d_ptr->protocolDescriptor(protocol);
}

QList<QBluetoothUuid> QBluetoothServiceInfo::serviceClassUuids() const
{
    QList<QBluetoothUuid> results;

    const QVariant var = attribute(QBluetoothServiceInfo::ServiceClassIds);
    if (!var.isValid())
        return results;

    const QBluetoothServiceInfo::Sequence seq = var.value<QBluetoothServiceInfo::Sequence>();
    for (int i = 0; i < seq.count(); i++)
        results.append(seq.at(i).value<QBluetoothUuid>());

    return results;
}

QBluetoothServiceInfo &QBluetoothServiceInfo::operator=(const QBluetoothServiceInfo &other)
{
    d_ptr = other.d_ptr;

    return *this;
}

static void dumpAttributeVariant(const QVariant &var, const QString indent)
{
    switch (int(var.type())) {
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

QDebug operator << (QDebug dbg, const QBluetoothServiceInfo &info)
{
    foreach (quint16 id, info.attributes()) {
        dumpAttributeVariant(info.attribute(id), QString::fromLatin1("(%1)\t").arg(id));
    }
    return dbg;
}

QBluetoothServiceInfo::Sequence QBluetoothServiceInfoPrivate::protocolDescriptor(QBluetoothUuid::ProtocolUuid protocol) const
{
    if (!attributes.contains(QBluetoothServiceInfo::ProtocolDescriptorList))
        return QBluetoothServiceInfo::Sequence();

    foreach (const QVariant &v, attributes.value(QBluetoothServiceInfo::ProtocolDescriptorList).value<QBluetoothServiceInfo::Sequence>()) {
        QBluetoothServiceInfo::Sequence parameters = v.value<QBluetoothServiceInfo::Sequence>();
        if (parameters.empty())
            continue;
        if (parameters.at(0).userType() == qMetaTypeId<QBluetoothUuid>()) {
            if (parameters.at(0).value<QBluetoothUuid>() == protocol)
                return parameters;
        }
    }

    return QBluetoothServiceInfo::Sequence();
}

int QBluetoothServiceInfoPrivate::serverChannel() const
{
    QBluetoothServiceInfo::Sequence parameters = protocolDescriptor(QBluetoothUuid::Rfcomm);

    if (parameters.isEmpty())
        return -1;
    else if (parameters.count() == 1)
        return 0;
    else
        return parameters.at(1).toUInt();
}

QT_END_NAMESPACE

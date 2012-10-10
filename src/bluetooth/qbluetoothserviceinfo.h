/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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

#ifndef QBLUETOOTHSERVICEINFO_H
#define QBLUETOOTHSERVICEINFO_H

#include "qbluetoothglobal.h"

#include <qbluetoothuuid.h>

#include <QtCore/QMetaType>
#include <QtCore/QList>
#include <QtCore/QVariant>

#include <QtCore/QDebug>

QTBLUETOOTH_BEGIN_NAMESPACE

class QBluetoothServiceInfoPrivate;
class QBluetoothDeviceInfo;

class Q_BLUETOOTH_EXPORT QBluetoothServiceInfo
{
public:
    enum AttributeId {
        ServiceRecordHandle = 0x0000,
        ServiceClassIds = 0x0001,
        ServiceId = 0x0003,
        ProtocolDescriptorList = 0x0004,
        BrowseGroupList = 0x0005,
        ServiceAvailability = 0x0008,
        PrimaryLanguageBase = 0x0100,
        ServiceName = PrimaryLanguageBase + 0x0000,
        ServiceDescription = PrimaryLanguageBase + 0x0001,
        ServiceProvider = PrimaryLanguageBase + 0x0002,
    };

    enum Protocol {
        UnknownProtocol,
        L2capProtocol,
        RfcommProtocol
    };

    class Sequence : public QList<QVariant>
    {
    public:
        Sequence() { }
        Sequence(const QList<QVariant> &list) : QList<QVariant>(list) { }
    };

    class Alternative : public QList<QVariant>
    {
    public:
        Alternative() { }
        Alternative(const QList<QVariant> &list) : QList<QVariant>(list) { }
    };

    QBluetoothServiceInfo();
    QBluetoothServiceInfo(const QBluetoothServiceInfo &other);
    ~QBluetoothServiceInfo();

    bool isValid() const;
    bool isComplete() const;

    void setDevice(const QBluetoothDeviceInfo &info);
    QBluetoothDeviceInfo device() const;

    void setAttribute(quint16 attributeId, const QVariant &value);
    void setAttribute(quint16 attributeId, const QBluetoothUuid &value);
    void setAttribute(quint16 attributeId, const QBluetoothServiceInfo::Sequence &value);
    void setAttribute(quint16 attributeId, const QBluetoothServiceInfo::Alternative &value);
    QVariant attribute(quint16 attributeId) const;
    QList<quint16> attributes() const;
    bool contains(quint16 attributeId) const;
    void removeAttribute(quint16 attributeId);

    inline void setServiceName(const QString &name);
    inline QString serviceName() const;
    inline void setServiceDescription(const QString &description);
    inline QString serviceDescription() const;
    inline void setServiceProvider(const QString &provider);
    inline QString serviceProvider() const;

    QBluetoothServiceInfo::Protocol socketProtocol() const;
    int protocolServiceMultiplexer() const;
    int serverChannel() const;

    QBluetoothServiceInfo::Sequence protocolDescriptor(QBluetoothUuid::ProtocolUuid protocol) const;

    inline void setServiceAvailability(quint8 availability);
    inline quint8 serviceAvailability() const;

    inline void setServiceUuid(const QBluetoothUuid &uuid);
    inline QBluetoothUuid serviceUuid() const;

    inline QList<QBluetoothUuid> serviceClassUuids() const;

    QBluetoothServiceInfo &operator=(const QBluetoothServiceInfo &other);

    bool isRegistered() const;
    bool registerService() const;
    bool unregisterService() const;

protected:
    friend Q_BLUETOOTH_EXPORT QDebug operator<<(QDebug, const QBluetoothServiceInfo &);

protected:
    QBluetoothServiceInfoPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(QBluetoothServiceInfo)
};

QTBLUETOOTH_END_NAMESPACE

Q_DECLARE_METATYPE(QtBluetooth::QBluetoothServiceInfo)
Q_DECLARE_METATYPE(QtBluetooth::QBluetoothServiceInfo::Sequence)
Q_DECLARE_METATYPE(QtBluetooth::QBluetoothServiceInfo::Alternative)
Q_DECLARE_METATYPE(QList<QtBluetooth::QBluetoothUuid>)

QTBLUETOOTH_BEGIN_NAMESPACE

inline void QBluetoothServiceInfo::setAttribute(quint16 attributeId, const QBluetoothUuid &value)
{
    setAttribute(attributeId, QVariant::fromValue(value));
}

inline void QBluetoothServiceInfo::setAttribute(quint16 attributeId, const QBluetoothServiceInfo::Sequence &value)
{
    setAttribute(attributeId, QVariant::fromValue(value));
}

inline void QBluetoothServiceInfo::setAttribute(quint16 attributeId, const QBluetoothServiceInfo::Alternative &value)
{
    setAttribute(attributeId, QVariant::fromValue(value));
}

inline void QBluetoothServiceInfo::setServiceName(const QString &name)
{
    setAttribute(ServiceName, QVariant::fromValue(name));
}

inline QString QBluetoothServiceInfo::serviceName() const
{
    return attribute(ServiceName).toString();
}

inline void QBluetoothServiceInfo::setServiceDescription(const QString &description)
{
    setAttribute(ServiceDescription, QVariant::fromValue(description));
}

inline QString QBluetoothServiceInfo::serviceDescription() const
{
    return attribute(ServiceDescription).toString();
}

inline void QBluetoothServiceInfo::setServiceProvider(const QString &provider)
{
    setAttribute(ServiceProvider, QVariant::fromValue(provider));
}

inline QString QBluetoothServiceInfo::serviceProvider() const
{
    return attribute(ServiceProvider).toString();
}

inline void QBluetoothServiceInfo::setServiceAvailability(quint8 availability)
{
    setAttribute(ServiceAvailability, QVariant::fromValue(availability));
}

inline quint8 QBluetoothServiceInfo::serviceAvailability() const
{
    return attribute(ServiceAvailability).toUInt();
}

inline void QBluetoothServiceInfo::setServiceUuid(const QBluetoothUuid &uuid)
{
    setAttribute(ServiceId, uuid);
}

inline QBluetoothUuid QBluetoothServiceInfo::serviceUuid() const
{
    return attribute(ServiceId).value<QBluetoothUuid>();
}

inline QList<QBluetoothUuid> QBluetoothServiceInfo::serviceClassUuids() const
{
    return attribute(ServiceClassIds).value<QList<QBluetoothUuid> >();
}

QTBLUETOOTH_END_NAMESPACE

#endif

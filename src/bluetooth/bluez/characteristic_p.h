/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
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

#ifndef CHARACTERISTIC_P_H
#define CHARACTERISTIC_P_H

#endif // CHARACTERISTIC_P_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.bluez.Device
 */
class OrgBluezCharacteristicInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.bluez.Characteristic"; }

public:
    OrgBluezCharacteristicInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);
    ~OrgBluezCharacteristicInterface();

public Q_SLOTS:
    inline QDBusPendingReply<QVariantMap> GetProperties()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("GetProperties"), argumentList);
    }

    inline QDBusPendingReply<> SetProperty(const QString &in0, const QDBusVariant &in1)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0) << qVariantFromValue(in1);
        return asyncCallWithArgumentList(QLatin1String("SetProperty"), argumentList);
    }
    inline QDBusPendingReply<QList<QDBusObjectPath> > DiscoverCharacteristics()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("DiscoverCharacteristics"), argumentList);
    }

    inline QDBusPendingReply<> RegisterCharacteristicsWatcher(const QDBusObjectPath &in0)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("RegisterCharacteristicsWatcher"), argumentList);
    }

    inline QDBusPendingReply<> UnregisterCharacteristicsWatcher(const QDBusObjectPath &in0)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("UnregisterCharacteristicsWatcher"), argumentList);
    }

Q_SIGNALS:
    void PropertyChanged(const QString &in0, const QDBusVariant &in1);
};

namespace org {
  namespace bluez {
    typedef ::OrgBluezCharacteristicInterface Characteristic;
  }
}

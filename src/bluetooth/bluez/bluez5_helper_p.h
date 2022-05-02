/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef BLUEZ5_HELPER_H
#define BLUEZ5_HELPER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/private/qtbluetoothglobal_p.h>

typedef QMap<QString, QVariantMap> InterfaceList;
typedef QMap<QDBusObjectPath, InterfaceList> ManagedObjectList;
typedef QMap<quint16, QDBusVariant> ManufacturerDataList;

Q_DECLARE_METATYPE(InterfaceList)
Q_DECLARE_METATYPE(ManufacturerDataList)
Q_DECLARE_METATYPE(ManagedObjectList)

QT_BEGIN_NAMESPACE

void initializeBluez5();
bool isBluez5();

// exported for unit test purposes
Q_BLUETOOTH_PRIVATE_EXPORT QVersionNumber bluetoothdVersion();

QString sanitizeNameForDBus(const QString& text);

QString findAdapterForAddress(const QBluetoothAddress &wantedAddress, bool *ok);

class QtBluezDiscoveryManagerPrivate;
class QtBluezDiscoveryManager : public QObject
{
    Q_OBJECT
public:
    QtBluezDiscoveryManager(QObject* parent = nullptr);
    ~QtBluezDiscoveryManager();
    static QtBluezDiscoveryManager *instance();

    bool registerDiscoveryInterest(const QString &adapterPath);
    void unregisterDiscoveryInterest(const QString &adapterPath);

    //void dumpState() const;

signals:
    void discoveryInterrupted(const QString &adapterPath);

private slots:
    void InterfacesRemoved(const QDBusObjectPath &object_path,
                           const QStringList &interfaces);
    void PropertiesChanged(const QString &interface,
                           const QVariantMap &changed_properties,
                           const QStringList &invalidated_properties,
                           const QDBusMessage &msg);

private:
    void removeAdapterFromMonitoring(const QString &dbusPath);

    QtBluezDiscoveryManagerPrivate *d;
};

QT_END_NAMESPACE

#endif

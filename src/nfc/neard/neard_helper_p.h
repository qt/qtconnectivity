/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Copyright (C) 2016 BasysKom GmbH.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
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
****************************************************************************/

#ifndef NEARD_HELPER_P_H
#define NEARD_HELPER_P_H

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

#include <QMetaType>
#include <QDBusObjectPath>

typedef QMap<QString, QVariantMap> InterfaceList;
typedef QMap<QDBusObjectPath, InterfaceList> ManagedObjectList;

Q_DECLARE_METATYPE(InterfaceList)
Q_DECLARE_METATYPE(ManagedObjectList)

class OrgFreedesktopDBusObjectManagerInterface;

QT_BEGIN_NAMESPACE

class NeardHelper : public QObject
{
    Q_OBJECT
public:
    NeardHelper(QObject* parent = 0);
    static NeardHelper *instance();

    OrgFreedesktopDBusObjectManagerInterface *dbusObjectManager();

signals:
    void tagFound(const QDBusObjectPath&);
    void tagRemoved(const QDBusObjectPath&);
    void recordFound(const QDBusObjectPath&);
    void recordRemoved(const QDBusObjectPath&);

private slots:
    void interfacesAdded(const QDBusObjectPath&, InterfaceList);
    void interfacesRemoved(const QDBusObjectPath&, const QStringList&);

private:
    OrgFreedesktopDBusObjectManagerInterface *m_dbusObjectManager;
};

QT_END_NAMESPACE

#endif // NEARD_HELPER_P_H

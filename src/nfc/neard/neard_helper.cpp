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

#include <QDBusMetaType>
#include "neard_helper_p.h"
#include "dbusobjectmanager_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_NFC_NEARD)
Q_GLOBAL_STATIC(NeardHelper, neardHelper)

NeardHelper::NeardHelper(QObject *parent) :
    QObject(parent)
{
    qDBusRegisterMetaType<InterfaceList>();
    qDBusRegisterMetaType<ManagedObjectList>();

    m_dbusObjectManager = new OrgFreedesktopDBusObjectManagerInterface(QStringLiteral("org.neard"),
                                                                       QStringLiteral("/"),
                                                                       QDBusConnection::systemBus(),
                                                                       this);
    if (!m_dbusObjectManager->isValid()) {
        qCCritical(QT_NFC_NEARD) << "dbus object manager invalid";
        return;
    }

    connect(m_dbusObjectManager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
            this,                &NeardHelper::interfacesAdded);
    connect(m_dbusObjectManager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
            this,                &NeardHelper::interfacesRemoved);
}

NeardHelper *NeardHelper::instance()
{
    return neardHelper();
}

OrgFreedesktopDBusObjectManagerInterface *NeardHelper::dbusObjectManager()
{
    return m_dbusObjectManager;
}

void NeardHelper::interfacesAdded(const QDBusObjectPath &path, InterfaceList interfaceList)
{
    const QList<QString> keys = interfaceList.keys();
    for (const QString &key : keys) {
        if (key == QStringLiteral("org.neard.Tag")) {
            emit tagFound(path);
            break;
        }
        if (key == QStringLiteral("org.neard.Record")) {
            emit recordFound(path);
            break;
        }
    }
}

void NeardHelper::interfacesRemoved(const QDBusObjectPath &path, const QStringList &list)
{
    if (list.contains(QStringLiteral("org.neard.Record"))) {
        qCDebug(QT_NFC_NEARD) << "record removed" << path.path();
        emit recordRemoved(path);
    } else if (list.contains(QStringLiteral("org.neard.Tag"))) {
        qCDebug(QT_NFC_NEARD) << "tag removed" << path.path();
        emit tagRemoved(path);
    }
}

QT_END_NAMESPACE

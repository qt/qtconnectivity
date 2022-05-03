/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
****************************************************************************/

#include "profile1context_p.h"

#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

OrgBluezProfile1ContextInterface::OrgBluezProfile1ContextInterface(QObject *parent) : QObject(parent)
{
}

void OrgBluezProfile1ContextInterface::NewConnection(const QDBusObjectPath &/*remotePath*/,
                                   const QDBusUnixFileDescriptor &descriptor,
                                   const QVariantMap &/*properties*/)
{
    qCDebug(QT_BT_BLUEZ) << "Profile Context: New Connection";
    emit newConnection(descriptor);
    setDelayedReply(false);
}

void OrgBluezProfile1ContextInterface::RequestDisconnection(const QDBusObjectPath &/*remotePath*/)
{
    qCDebug(QT_BT_BLUEZ) << "Profile Context: Request Disconnection";
    setDelayedReply(false);
}

void OrgBluezProfile1ContextInterface::Release()
{
    qCDebug(QT_BT_BLUEZ) << "Profile Context: Release";
}

QT_END_NAMESPACE

#include "moc_profile1context_p.cpp"
